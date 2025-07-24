#ifndef MACROS_H
#define MACROS_H

#define HOT                __attribute__((hot))
#define PURE               __attribute__((pure))
#define NOINLINE           __attribute__((noinline))
#define NORETURN           __attribute__((noreturn))
#define INLINE             __attribute__((always_inline))
#define COLD               __attribute__((cold))
#define CONST              __attribute__((const))
#define CONSTRUCTOR        __attribute__((constructor))
#define DESTRUCTOR         __attribute__((destructor))
#define ALIGNED(n)         __attribute__((aligned(n)))
#define WARN_UNUSED        __attribute__((warn_unused_result))
#define WEAK               __attribute__((weak))
#define UNUSED             __attribute__((unused))
#define DEAD               __attribute__((unused))
#define UNREACHABLE        __attribute__((unreachable))

#define MAX_PRINT_LEN      (32 * 1024 * 1024)
#define INIT_ARENA_SIZE    (1024 * 1024)
#define DEBUG              false
#define PROFILE            false

/* ANSI escape codes */
#define ESC                ("\x1b[")
#define RESET              (ESC "0m")
#define HIGHLIGHT          (ESC "38;2;255;255;0m")

#endif /* MACROS_H */
