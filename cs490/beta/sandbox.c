// This file is intended to run a process with seccomp applied.
// This should not be taken as a completely secure sandbox, but it blocks some
// of the system calls that can be used for malicious activity.

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/resource.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <sys/syscall.h>

#include <linux/filter.h>
#include <linux/seccomp.h>
#include <linux/audit.h>

// Helper macros for constructing BPF packets.
// Inspired by: https://eigenstate.org/notes/seccomp
// Instruction for returning an immediate.
#define RET_CONST (BPF_RET+BPF_K)
// Instruction for jumping to an immediate offset conditioning on acc == arg.
#define JEQ_CONST (BPF_JMP+BPF_JEQ+BPF_K)
// Instruction for loading a word with an absolute offset.
#define LDW_ABS   (BPF_LD+BPF_W+BPF_ABS)
#if __BYTE_ORDER == __LITTLE_ENDIAN
#  define ARG_LO_OFF (0)
#  define ARG_HI_OFF (sizeof(uint32_t))
#elif __BYTE_ORDER == __BIG_ENDIAN
#  define ARG_LO_OFF (sizeof(uint32_t))
#  define ARG_HI_OFF (0)
#else
#  error "Unable to determine endianness"
#endif  // __BYTE_ORDER
// Shortcut for accepting on match.
//   1. if acc == value continue (0), otherwise skip 1 line (1).
//   2. return allow.
#define ALLOW_IF_EQ(value) \
    BPF_JUMP(JEQ_CONST, (value), 0, 1), \
    BPF_STMT(RET_CONST, SECCOMP_RET_ALLOW)
// Shortcut for rejecting on mismatch.
//   1. if acc == value skip 1 line (1), otherwise continue (0).
//   2. return disallow.
#define REQUIRE_EQ(value) \
    BPF_JUMP(JEQ_CONST, (value), 1, 0), \
    BPF_STMT(RET_CONST, SECCOMP_RET_KILL)
// Shortcuts for loading fields of seccomp_data
#define LOAD_ARCH \
    BPF_STMT(LDW_ABS, offsetof(struct seccomp_data, arch))
#define LOAD_NR \
    BPF_STMT(LDW_ABS, offsetof(struct seccomp_data, nr))
// Shortcuts for getting lo and hi word from a value.
#define MASKW_LO(value) ( ((uint32_t)(value)) & 0xFFFFFFFF )
#define MASKW_HI(value) MASKW_LO( ((uint64_t)(value) >> 32) )
// Argument loading influenced by:
// github.com/openssh/openssh-portable/blob/master/sandbox-seccomp-filter.c
#define LOAD_ARG_LO(n) \
    BPF_STMT(LDW_ABS, offsetof(struct seccomp_data, args[(n)]) + ARG_LO_OFF)
#define LOAD_ARG_HI(n) \
    BPF_STMT(LDW_ABS, offsetof(struct seccomp_data, args[(n)]) + ARG_HI_OFF)
// Shortcut for allowing syscalls with a matching arg.
#define ALLOW_CALL_IF_ARG_MATCH(syscall, n, data_val) \
    LOAD_NR, \
    BPF_JUMP(JEQ_CONST, (syscall), 0, 5), \
    LOAD_ARG_LO(n), \
    BPF_JUMP(JEQ_CONST, MASKW_LO(data_val), 0, 3), \
    LOAD_ARG_HI(n), \
    BPF_JUMP(JEQ_CONST, MASKW_HI(data_val), 0, 1), \
    BPF_STMT(RET_CONST, SECCOMP_RET_ALLOW)
// Shortcut for allowing syscalls with 2 matchings args.
#define ALLOW_CALL_IF_ARGS_MATCH2(syscall, n1, dval1, n2, dval2) \
    LOAD_NR, \
    BPF_JUMP(JEQ_CONST, (syscall), 0, 9), \
    LOAD_ARG_LO(n1), \
    BPF_JUMP(JEQ_CONST, MASKW_LO(dval1), 0, 7), \
    LOAD_ARG_HI(n1), \
    BPF_JUMP(JEQ_CONST, MASKW_HI(dval1), 0, 5), \
    LOAD_ARG_LO(n2), \
    BPF_JUMP(JEQ_CONST, MASKW_LO(dval2), 0, 3), \
    LOAD_ARG_HI(n2), \
    BPF_JUMP(JEQ_CONST, MASKW_HI(dval2), 0, 1), \
    BPF_STMT(RET_CONST, SECCOMP_RET_ALLOW)

// Enables filtered seccomp mode for this process.
// This is the primary sandboxing done by this executable.
int seccomp2() {
  static struct sock_filter seccomp_filter[] = {
  // Load arch.
  LOAD_ARCH,
  // Filter on arch.
  REQUIRE_EQ(AUDIT_ARCH_X86_64),
  
  // Load syscall number.
  LOAD_NR,
  // Filter on syscall number.
  // Default rules allowed by strict mode (except write which is handled later).
  ALLOW_IF_EQ(SYS_rt_sigreturn),
  ALLOW_IF_EQ(SYS_exit),
  ALLOW_IF_EQ(SYS_read),

  // Syscalls that probably don't need to be blocked, and are often used.
  // Influenced by: https://github.com/stedolan/Sandbox/blob/master/sandbox.c
  ALLOW_IF_EQ(SYS_uname),
  ALLOW_IF_EQ(SYS_set_thread_area),
  ALLOW_IF_EQ(SYS_time),
  ALLOW_IF_EQ(SYS_gettimeofday),
  ALLOW_IF_EQ(SYS_sched_getaffinity),
  ALLOW_IF_EQ(SYS_getpid),
  ALLOW_IF_EQ(SYS_clock_gettime),

  // Syscalls used by python during startup (we have to allow these).
  ALLOW_IF_EQ(SYS_brk),
  ALLOW_IF_EQ(SYS_access),
  ALLOW_IF_EQ(SYS_mmap),
  ALLOW_IF_EQ(SYS_mprotect),
  ALLOW_IF_EQ(SYS_fstat),
  ALLOW_IF_EQ(SYS_close),
  ALLOW_IF_EQ(SYS_arch_prctl),
  ALLOW_IF_EQ(SYS_munmap),
  ALLOW_IF_EQ(SYS_set_tid_address),
  ALLOW_IF_EQ(SYS_set_robust_list),
  ALLOW_IF_EQ(SYS_rt_sigaction),
  ALLOW_IF_EQ(SYS_rt_sigprocmask),
  ALLOW_IF_EQ(SYS_stat),
  ALLOW_IF_EQ(SYS_getrlimit),
  ALLOW_IF_EQ(SYS_readlink),
  ALLOW_IF_EQ(SYS_sysinfo),
  ALLOW_IF_EQ(SYS_geteuid),
  ALLOW_IF_EQ(SYS_getegid),
  ALLOW_IF_EQ(SYS_getuid),
  ALLOW_IF_EQ(SYS_getgid),
  ALLOW_IF_EQ(SYS_lstat),
  ALLOW_IF_EQ(SYS_getdents),
  ALLOW_IF_EQ(SYS_lseek),
  ALLOW_IF_EQ(SYS_dup),
  ALLOW_IF_EQ(SYS_getcwd),
  ALLOW_IF_EQ(SYS_exit_group),
  ALLOW_IF_EQ(SYS_connect),
  ALLOW_IF_EQ(SYS_futex),
  ALLOW_IF_EQ(SYS_getrandom),
  ALLOW_IF_EQ(SYS_sigaltstack),
  ALLOW_IF_EQ(SYS_select),

  // Allow execve so this process can run the python environment.
  // I have been unsuccesful in filtering this further, so I have to just
  // whitelist the syscall, but since PR_NO_NEW_PRIVS is set to 1 in order to
  // enter seccomp filter mode, this is probably okay.
  ALLOW_IF_EQ(SYS_execve),

  // These syscalls also occur during python startup, but we are filtering
  // beyond just the syscall number for these.
  // Filtering below influenced by: http://khamidou.com/sandboxing/
  ALLOW_CALL_IF_ARG_MATCH(SYS_ioctl, 1, TCGETS),   // get term struct.
  ALLOW_CALL_IF_ARG_MATCH(SYS_ioctl, 1, FIOCLEX),  // set close on exec.
  // Only allow creation of RDONLY file descriptors.
  ALLOW_CALL_IF_ARG_MATCH(SYS_open, 1, O_RDONLY),
  ALLOW_CALL_IF_ARG_MATCH(SYS_open, 1, O_RDONLY | O_CLOEXEC),
  ALLOW_CALL_IF_ARG_MATCH(SYS_open, 1, O_RDONLY | O_CLOEXEC |
                                       O_NONBLOCK | O_DIRECTORY),
  ALLOW_CALL_IF_ARG_MATCH(SYS_openat, 2, O_RDONLY),
  ALLOW_CALL_IF_ARG_MATCH(SYS_openat, 2, O_RDONLY | O_CLOEXEC),
  ALLOW_CALL_IF_ARG_MATCH(SYS_openat, 2, O_RDONLY | O_CLOEXEC |
                                         O_NONBLOCK | O_DIRECTORY),
  // Only allow creation of local sockets.
  ALLOW_CALL_IF_ARG_MATCH(SYS_socket, 0, AF_LOCAL),
  // Only allow writes to known file descriptors.
  ALLOW_CALL_IF_ARG_MATCH(SYS_write, 0, 0),
  ALLOW_CALL_IF_ARG_MATCH(SYS_write, 0, 1),
  ALLOW_CALL_IF_ARG_MATCH(SYS_write, 0, 2),

  // Only allow a couple fcntl operations.
  ALLOW_CALL_IF_ARG_MATCH(SYS_fcntl, 1, F_DUPFD),
  ALLOW_CALL_IF_ARG_MATCH(SYS_fcntl, 1, F_DUPFD_CLOEXEC),
  ALLOW_CALL_IF_ARG_MATCH(SYS_fcntl, 1, F_GETFD),
  ALLOW_CALL_IF_ARG_MATCH(SYS_fcntl, 1, F_GETFL),
  ALLOW_CALL_IF_ARGS_MATCH2(SYS_fcntl, 1, F_SETFD, 2, FD_CLOEXEC),

#ifdef SANDBOX_TRACE_UNMATCHED
  // If no filters above matched, send signal to tracer.
  BPF_STMT(RET_CONST, SECCOMP_RET_TRACE)
#else
  // If no filters above matched, kill the process.
  BPF_STMT(RET_CONST, SECCOMP_RET_KILL)
#endif  // SANDBOX_TRACE_UNMATCHED
  };
  static struct sock_fprog seccomp_filterprog = {
    .len = sizeof(seccomp_filter)/sizeof(seccomp_filter[0]),
    .filter = seccomp_filter
  };

  if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0))
    return -1;

  if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &seccomp_filterprog, 0, 0))
    return -2;

  return 0;
}

// Returns a char * to a string of the form NAME=VALUE, where the value was
// grabbed from the current environment or NULL if no such value existed.
// If not NULL, the return value may be freed when done.
char *make_env_str(char *name) {
  char *value = getenv(name);
  char *result = NULL;

  if (value) {
    size_t nlen = strlen(name);
    size_t vlen = strlen(value);

    // Two extra characters are needed for the = and the \0.
    result = (char *) malloc(nlen + vlen + 2);
    if (result) {
      strcpy(result, name);
      result[nlen] = '=';
      strcpy(result + nlen + 1, value);
      result[nlen + vlen + 2] = '\0';
    }
  }

  return result;
}

int main(int argc, char **argv) {
  // For now just try to activate seccomp and report if it succeeds.
  
  // TODO: Actual arg parsing, consider argp.
  if (argc < 2)
    return -1;

  // Capture some of the environment to forward.
  char *env_with_nulls[] = {
    make_env_str("USER"),
    make_env_str("LOGNAME"),
    make_env_str("HOME"),
    make_env_str("LANG"),
    make_env_str("PATH"),
    make_env_str("PWD"),
    make_env_str("SHELL"),
    make_env_str("TERM"),
    make_env_str("PAGER"),
    make_env_str("EDITOR"),
    NULL,
  };
  size_t env_len = sizeof(env_with_nulls)/sizeof(env_with_nulls[0]);

  char *env[env_len], **src, **dst;
  // Only copy over non-null pointers.
  for (src = env_with_nulls, dst = env; src < env_with_nulls + env_len; ++src)
    if (*src) *dst++ = *src;
  *dst = NULL;  // Terminate env with NULL.

  // Enter seccomp filter mode.
  errno = 0;
  if (seccomp2()) {
    perror("Coulnd't enter seccomp filter mode: ");
    return -2;
  }

  // Run the program specified by argv[1] with args argv[1]..argv[argc-1]
  // our minimal environment.
  execve(argv[1], argv + 1, env);

  return -3;  // execve failed.
}
