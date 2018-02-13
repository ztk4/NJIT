// This file is intended to run a process with seccomp applied.
// This should not be taken as a completely secure sandbox, but it blocks some
// of the system calls that can be used for malicious activity.

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
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
// Shortcut for accepting on match.
//   1. if acc == value continue (0), otherwise skip 1 line (1).
//   2. return allow.
#define ALLOW(value) \
    BPF_JUMP(JEQ_CONST, (value), 0, 1), \
    BPF_STMT(RET_CONST, SECCOMP_RET_ALLOW)
// Shortcut for rejecting on mismatch.
//   1. if acc == value skip 1 line (1), otherwise continue (0).
//   2. return disallow.
#define REQUIRE(value) \
    BPF_JUMP(JEQ_CONST, (value), 1, 0), \
    BPF_STMT(RET_CONST, SECCOMP_RET_KILL)
// Shortcuts for loading fields of seccomp_data
#define LOAD_ARCH \
    BPF_STMT(LDW_ABS, offsetof(struct seccomp_data, arch))
#define LOAD_NR \
    BPF_STMT(LDW_ABS, offsetof(struct seccomp_data, nr))
#define LOAD_ARG(n) \
    BPF_STMT(LDW_ABS, offsetof(struct seccomp_data, args) + (n)*8)
// Shortcut for allowing syscalls with a matching arg.
#define ALLOW_CALL_IF_ARG_MATCH(syscall, n, data_val) \
    LOAD_NR, \
    BPF_JUMP(JEQ_CONST, (syscall), 0, 3), \
    LOAD_ARG(n), \
    BPF_JUMP(JEQ_CONST, (data_val), 0, 1), \
    BPF_STMT(RET_CONST, SECCOMP_RET_ALLOW)

// Enables filtered seccomp mode for this process.
// This is the primary sandboxing done by this executable.
int seccomp2() {
  static struct sock_filter seccomp_filter[] = {
  // Load arch.
  LOAD_ARCH,
  // Filter on arch.
  REQUIRE(AUDIT_ARCH_X86_64),
  
  // Load syscall number.
  LOAD_NR,
  // Filter on syscall number.
  // Default rules allowed by strict mode (except write which is handled later).
  ALLOW(SYS_rt_sigreturn),
  ALLOW(SYS_exit),
  ALLOW(SYS_read),

  // Syscalls that probably don't need to be blocked, and are often used.
  // Influenced by: https://github.com/stedolan/Sandbox/blob/master/sandbox.c
  ALLOW(SYS_uname),
  ALLOW(SYS_set_thread_area),
  ALLOW(SYS_time),
  ALLOW(SYS_gettimeofday),
  ALLOW(SYS_sched_getaffinity),
  ALLOW(SYS_getpid),
  ALLOW(SYS_clock_gettime),

  // Syscalls used by python during startup (we have to allow these).
  ALLOW(SYS_brk),
  ALLOW(SYS_access),
  ALLOW(SYS_mmap),
  ALLOW(SYS_mprotect),
  ALLOW(SYS_fstat),
  ALLOW(SYS_close),
  ALLOW(SYS_arch_prctl),
  ALLOW(SYS_munmap),
  ALLOW(SYS_set_tid_address),
  ALLOW(SYS_set_robust_list),
  ALLOW(SYS_rt_sigaction),
  ALLOW(SYS_rt_sigprocmask),
  ALLOW(SYS_stat),
  ALLOW(SYS_getrlimit),
  ALLOW(SYS_readlink),
  ALLOW(SYS_sysinfo),
  ALLOW(SYS_geteuid),
  ALLOW(SYS_getegid),
  ALLOW(SYS_getuid),
  ALLOW(SYS_getgid),
  ALLOW(SYS_lstat),
  ALLOW(SYS_getdents),
  ALLOW(SYS_lseek),
  ALLOW(SYS_dup),
  ALLOW(SYS_getcwd),
  ALLOW(SYS_exit_group),
  ALLOW(SYS_connect),
  ALLOW(SYS_futex),
  ALLOW(SYS_getrandom),
  ALLOW(SYS_sigaltstack),

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
  
  // Allow execve so this process can run the python environment.
  // I have been unsuccesful in filtering this further, so I have to just
  // whitelist the syscall, but since PR_NO_NEW_PRIVS is set to 1 in order to
  // enter seccomp filter mode, this is probably okay.
  ALLOW(SYS_execve),
  
  // If no filters above matched, kill the process.
  BPF_STMT(RET_CONST, SECCOMP_RET_KILL)
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

int main(int argc, char **argv) {
  // For now just try to activate seccomp and report if it succeeds.
  
  if (argc < 2)
    return -1;

  // Enter seccomp filter mode.
  errno = 0;
  if (seccomp2()) {
    perror("Coulnd't enter seccomp filter mode: ");
    return -2;
  }

  // Run the program specified by argv[1] with args argv[1]..argv[argc-1]
  // and an empty environment.
  char *env[] = { NULL };
  execve(argv[1], argv + 1, env);

  return -3;  // execve failed.
}
