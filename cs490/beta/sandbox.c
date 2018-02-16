// This file is intended to run a process with seccomp applied.
// This should not be taken as a completely secure sandbox, but it blocks some
// of the system calls that can be used for malicious activity.

#include <argp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

#include <sys/resource.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/wait.h>

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
    // Default rules allowed by strict mode (except write).
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
    ALLOW_IF_EQ(SYS_mremap),

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

#ifdef SANDBOX_TRACE_MODE
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

// ARGP Definitions.
const char *argp_program_version = "0.1.0";

static char doc[] = "Runs an executable within a seccomp-filter style sandbox."
                    "The path to the executable must be an absolute path. "
                    "This sandbox is in no way mean to be impenetrable, but is "
                    "meant to provide a moderate amount of protection when "
                    "executing software from an unknown source. The use of a "
                    "sandbox like this is often most secure when paired with "
                    "additional sandboxing at the application level.";
static char args_doc[] = "-- sandboxee [SANDBOXEE_ARG...]";
static const struct argp_option cmd_options[] = {
  // Resource Group
  { NULL, 0, NULL, 0,
    "specify process resource limits:" },
  { "cpu_timout", 'c', "SECONDS", 0,
    "Maximum time the sandboxed program will run on the CPU. This include both "
    "user and sys time, but not idle time. The default is 60 seconds." },
  { "stack_size", 's', "SIZE", 0,
    "Maximum stack size. Units may be specified in human readable form with a "
    "suffixed unit [GMK]B?. Bytes will be assumed if not specified. The "
    "default is 16M." },
  { "heap_size", 'd', "SIZE", 0,
    "Maximum heap size. Units may be specified in human readable form with a "
    "suffixed unit [GMK]B?. Bytes will be assumed if not specified. The "
    "default is 16M." },
  { "virt_size", 'a', "SIZE", 0,
    "Maximum size for the sandboxee's virtual memory pool (address space). "
    "Units may be specified in human readable form with a suffixed unit "
    "[GMK]B?. Bytes will be assumed if not specified. The default is 100M." },
  // Monitoring Group
  { NULL, 0, NULL, 0,
    "specifiy monitoring parameters:" },
  { "real_timeout", 'r', "SECONDS", 0,
    "Maximum time the sandboxed program can run for in seconds. This "
    "is analagous to wall clock time. The default is 15 seconds." },

  { 0 }
};

// The storage for the above args (defaults specified).
static uint32_t real_timeout = 15;
static rlim_t cpu_timeout = 60;
static rlim_t stack_size = 16u << 20;  // 16M
static rlim_t heap_size = 16u << 20;   // 16M
static rlim_t virt_size = 100u << 20;  // 100M
static char **sandboxee_args = NULL;   // Points to the start of sandboxee args.

// Quick method for handling byte values.
long str_to_bytes(char *str) {
  char *end;
  long res;
  int shift = 0;

  // Try to read a numerical value.
  res = strtol(str, &end, 0);
  if (!errno) {
    end += strspn(end, " \t\n\r\v\f");
    switch (*end) {
     case 'G': shift += 10;  // FALLTHROUGH
     case 'M': shift += 10;  // FALLTRHOUGH
     case 'K': shift += 10;
      ++end;
      break;
    }
    if (*end == 'B') ++end;  // Allow for suffix of B (or just B by itself).

    if (*end) errno = EINVAL;
    else if (res <= 0) errno = ERANGE;
  }

  return errno ? -1 : (res << shift);
}

// Option parsing.
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  errno = 0;

  switch (key) {
   case 'c': {
    char *end;
    long tmp = strtol(arg, &end, 0);
    if (errno || *end || tmp <= 0)  {
      argp_error(state, "invalid cpu timeout '%s'", arg);
      return EINVAL;
    }
    cpu_timeout = tmp;
    break;
   }
   case 'r': {
    char *end;
    long tmp = strtol(arg, &end, 0);
    if (errno || * end || tmp <= 0) {
      argp_error(state, "invalid real timeout '%s'", arg);
      return EINVAL;
    }
    real_timeout = tmp;
    break;
   }

   case 's': {
    stack_size = str_to_bytes(arg);
    if (errno) {
      argp_error(state, "invalid stack size '%s'", arg);
      return EINVAL;
    }
    break;
   }
   case 'd': {
    heap_size = str_to_bytes(arg);
    if (errno) {
      argp_error(state, "invalid heap size '%s'", arg);
      return EINVAL;
    }
    break;
   }
   case 'a': {
    virt_size = str_to_bytes(arg);
    if (errno) {
      argp_error(state, "invalid virt mem size '%s'", arg);
      return EINVAL;
    }
    break;
   }

   // No more options, just regular args left (sandboxee args).
   case ARGP_KEY_ARGS: {
    sandboxee_args = state->argv + state->next;
    if (state->next >= state->argc) {
      argp_error(state, "no sandboxee specified");
      return EINVAL;
    }
    break;
   }

   // Cleanup
   case ARGP_KEY_END: {
    if (!sandboxee_args) {
      argp_error(state, "no sandboxee specified");
      return EINVAL;
    }
    break;
   }

   default:
    return ARGP_ERR_UNKNOWN;
  }

  return 0;
}

// For argument parsing.
static struct argp argp = { cmd_options, parse_opt, args_doc, doc };

// Return codes.
enum return_code {
  RET_OK = 0,
  RET_DO_NOT_USE_RESERVED = 1,
  RET_UNABLE_TO_SECCOMP,  // Unable to enter seccomp filter mode.
  RET_UNABLE_TO_EXECVE,   // Unable to execve the sandboxee.
  RET_UNABLE_TO_TIME,     // Couldn't start real timer.
  RET_REAL_TIMEOUT,       // Child took to much real (wall clock) time.
  RET_CHILD_MEM_FAULT,    // SEGV from bad memory access or out of mem.
  RET_CHILD_CPU_TIMEOUT,  // XCPU from system indicates exceeded cpu timeout.
  RET_CHILD_SYS_KILL,     // KILL from system (probably seccomp violation).
  RET_CHILD_SIG_OTHER,    // Some other signal killed the child.
  RET_CHILD_BAD_STATE,    // Child responded to wait with unknown wait status.
  RET_UNABLE_TO_FORK,     // Unable to fork a child process.
  RET_UNABLE_TO_RLIMIT,   // Unable to limit child process's resources.
};

// Forks the process, then the child process enter seccomp filter mode and
// execve's the sandboxee. This method should only return on the parent.
pid_t fork_sandboxee() {
  pid_t child_id = fork();

  // We are the parent process (child may or may not have been created).
  if (child_id)
    return child_id;

  // Otherwise we are the child.
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

  errno = 0;

  // Set resource limits.
  struct rlimit resource_lim;

  // CPU
  if (getrlimit(RLIMIT_CPU, &resource_lim)) {
    perror("Couldn't get cpu rlimit");
    return -RET_UNABLE_TO_RLIMIT;
  }
  resource_lim.rlim_cur = cpu_timeout;
  if (setrlimit(RLIMIT_CPU, &resource_lim)) {
    perror("Couldn't set cpu rlimit");
    return -RET_UNABLE_TO_RLIMIT;
  }

  // STACK
  if (getrlimit(RLIMIT_STACK, &resource_lim)) {
    perror("Couldn't get stack rlimit");
    return -RET_UNABLE_TO_RLIMIT;
  }
  resource_lim.rlim_max = stack_size;
  if (resource_lim.rlim_cur > stack_size)
    resource_lim.rlim_cur = stack_size;
  if (setrlimit(RLIMIT_STACK, &resource_lim)) {
    perror("Couldn't set stack rlimit");
    return -RET_UNABLE_TO_RLIMIT;
  }

  // DATA (includes heap).
  if (getrlimit(RLIMIT_DATA, &resource_lim)) {
    perror("Couldn't get data rlimit");
    return -RET_UNABLE_TO_RLIMIT;
  }
  resource_lim.rlim_max = heap_size;
  if (resource_lim.rlim_cur > heap_size)
    resource_lim.rlim_cur = heap_size;
  if (setrlimit(RLIMIT_DATA, &resource_lim)) {
    perror("Couldn't set data rlimit");
    return -RET_UNABLE_TO_RLIMIT;
  }

  // VIRT
  if (getrlimit(RLIMIT_AS, &resource_lim)) {
    perror("Couldn't get virtual memory rlimit");
    return -RET_UNABLE_TO_RLIMIT;
  }
  resource_lim.rlim_max = virt_size;
  if (resource_lim.rlim_cur > virt_size)
    resource_lim.rlim_cur = virt_size;
  if (setrlimit(RLIMIT_AS, &resource_lim)) {
    perror("Couldn't set virtual memory rlimit");
    return -RET_UNABLE_TO_RLIMIT;
  }

  // Enter seccomp filter mode.
  errno = 0;
  if (seccomp2()) {
    perror("Coulnd't enter seccomp filter mode");
    return -RET_UNABLE_TO_SECCOMP;
  }

  // Run the program specified by argv[after_opts] with all other args.
  // our minimal environment.
  execve(sandboxee_args[0], sandboxee_args, env);

  return -RET_UNABLE_TO_EXECVE;  // execve failed.
}

int main(int argc, char **argv) {
  // Handle args.
  argp_parse(&argp, argc, argv, 0, 0, NULL);

#ifdef SANDBOX_TRACE_MODE
  // Warn the user if running in TRACE mode.
  fprintf(stderr, "WARNING: Running in TRACE mode. DO NOT USE IN PRODUCTION!\n"
                  "         Recompile without -DSANDBOX_TRACE_MODE to\n"
                  "         generate a production binary.\n");
#endif  // SANDBOX_TRACE_MODE
  
  pid_t child_pid = fork_sandboxee();

  if (child_pid > 0) {
    // We are the parent, and the child forked succesfully.
    errno = 0;
    int wstatus;
    uint32_t s_elapsed = 0;
    struct timespec child_start, curr;
    if (!clock_gettime(CLOCK_REALTIME, &child_start)) {
      // Spinlock until child finished or until real time has expired.
      do {
        // Check if child has exited by some means.
        if (waitpid(child_pid, &wstatus, WNOHANG)) break;

        // Check if timed out.
        if (clock_gettime(CLOCK_REALTIME, &curr)) break;
        s_elapsed =
            (uint32_t) (curr.tv_sec - child_start.tv_sec +
                        (double)(curr.tv_nsec - child_start.tv_nsec)*1e-9);
      } while(s_elapsed < real_timeout);
    }

    // Couldn't measure time or process timed out.
    if (errno || s_elapsed >= real_timeout) {
      // Kill the (now rougue) child process.
      kill(child_pid, SIGKILL);  // Would only fail if child is already dead.
      // Print an error message and quit.
      if (errno) {
        perror("Unable to time sandboxee, killed");
        return RET_UNABLE_TO_TIME;
      } else {
        fprintf(stderr, "Sandboxee exceeded real time limit, killed\n");
        return RET_REAL_TIMEOUT;
      }
    }

    // The child process has exited.
    if (WIFEXITED(wstatus)) {
      // Child exited by it's own volition (exit, _exit, etc).
      int status = WEXITSTATUS(wstatus);
      if (status)
        fprintf(stderr, "Sandboxee execution failed\n");

      return status;
    } else if (WIFSIGNALED(wstatus)) {
      // Some signal terminated the child (SIGKILL, SIGSYS, etc).
      fprintf(stderr, "Sandboxee was terminated by signal %s\n",
                      strsignal(WTERMSIG(wstatus)));
      switch (WTERMSIG(wstatus)) {
       case SIGKILL: return RET_CHILD_SYS_KILL;
       case SIGSEGV: return RET_CHILD_MEM_FAULT;
       case SIGXCPU: return RET_CHILD_CPU_TIMEOUT;
       default     : return RET_CHILD_SIG_OTHER;
      }
    } else {
      // Not sure what's going on, kill the child to be safe and then exit.
      kill(child_pid, SIGKILL);
      fprintf(stderr, "Sandboxee entered unspecified state, killed\n");
      return RET_CHILD_BAD_STATE;
    }

  } else if (child_pid == -1) {
    fprintf(stderr, "Unable to fork child process\n");
    return RET_UNABLE_TO_FORK;
  } else {
    // We are the child, and we at some point failed to execve the sandboxee.
    // Significant error messages have already been printed at this point.
    return -child_pid;  // Error codes from fork_sandboxee.
  }
}
