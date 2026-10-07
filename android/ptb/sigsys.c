/* ptb for Android is a static glibc binary. Android's app seccomp filter
   (since 8.0) answers syscalls the platform's bionic never makes - clone3,
   clock_gettime64, statx, faccessat2, rseq, ... - with SIGSYS instead of
   ENOSYS. glibc falls back to the old call on ENOSYS, so turn the trap into
   that. On arm/arm64 the handler goes in at the ELF entry (ptb_start, linked
   with -e), before glibc's own start-up makes its first syscall; x86_64
   (emulators only) installs it from a constructor. Each trap is reported on
   stderr, which NoSpherA2 keeps in the job log. */
#define _GNU_SOURCE
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/syscall.h>
#include <ucontext.h>
#include <unistd.h>

/* raw syscalls: this runs before libc is initialised (no TLS, no errno) */
static long sc(long n, long a, long b, long c, long d) {
#if defined(__aarch64__)
  register long x8 __asm__("x8") = n, x0 __asm__("x0") = a,
    x1 __asm__("x1") = b, x2 __asm__("x2") = c, x3 __asm__("x3") = d;
  __asm__ volatile("svc 0" : "+r"(x0) : "r"(x8), "r"(x1), "r"(x2), "r"(x3) : "memory");
  return x0;
#elif defined(__arm__)
  register long r7 __asm__("r7") = n, r0 __asm__("r0") = a,
    r1 __asm__("r1") = b, r2 __asm__("r2") = c, r3 __asm__("r3") = d;
  __asm__ volatile("svc 0" : "+r"(r0) : "r"(r7), "r"(r1), "r"(r2), "r"(r3) : "memory");
  return r0;
#else
  return syscall(n, a, b, c, d);
#endif
}

static void enosys(int sig, siginfo_t *si, void *ctx) {
  ucontext_t *uc = ctx;
  char m[] = "ptb: seccomp trapped syscall      -> ENOSYS\n";
  int n = si->si_syscall;
  (void)sig;
  for (int i = 33; i >= 29 && n; i--, n /= 10) m[i - 1] = '0' + n % 10;
  sc(SYS_write, 2, (long)m, sizeof m - 1, 0);
#if defined(__aarch64__)
  uc->uc_mcontext.regs[0] = -ENOSYS;
#elif defined(__arm__)
  uc->uc_mcontext.arm_r0 = -ENOSYS;
#elif defined(__x86_64__)
  uc->uc_mcontext.gregs[REG_RAX] = -ENOSYS;
#endif
}

#if defined(__aarch64__) || defined(__arm__)
/* the kernel's struct sigaction (handler, flags, restorer, mask) */
struct ksa { void *h; unsigned long flags; void *restorer; unsigned long long mask; };

__attribute__((used)) static void early_sigsys(void) {
  struct ksa sa = { (void *)enosys, SA_SIGINFO, 0, 0 };
  sc(SYS_rt_sigaction, SIGSYS, (long)&sa, 0, 8);
}

/* keep _start's register (rtld_fini) across the call in a callee-saved one */
# if defined(__aarch64__)
__asm__(".globl ptb_start\n.type ptb_start,%function\nptb_start:\n"
        " mov x19, x0\n bl early_sigsys\n mov x0, x19\n b _start\n");
# else
/* the file's own instruction set (no .arm/.thumb switch behind gcc's back);
   the linker adds an interworking veneer for b _start if needed */
#  ifdef __thumb__
#   define PTB_FUNC ".thumb_func\n"
#  else
#   define PTB_FUNC ""
#  endif
__asm__(".globl ptb_start\n.type ptb_start,%function\n" PTB_FUNC "ptb_start:\n"
        " mov r4, r0\n bl early_sigsys\n mov r0, r4\n b _start\n");
# endif
#else
__attribute__((constructor)) static void trap_to_enosys(void) {
  struct sigaction sa;
  memset(&sa, 0, sizeof sa);
  sa.sa_sigaction = enosys;
  sa.sa_flags = SA_SIGINFO;
  sigaction(SIGSYS, &sa, 0);
}
#endif

/* pthread_create calls clone3 with every signal blocked, and a trap on a
   blocked SIGSYS kills the process (the kernel resets the handler). Taking
   this definition keeps libc.a's clone3.o out: glibc caches the ENOSYS and
   uses clone from then on. */
int __clone3(void *args, unsigned long size, int (*fn)(void *), void *arg) {
  (void)args; (void)size; (void)fn; (void)arg;
  errno = ENOSYS;
  return -1;
}
