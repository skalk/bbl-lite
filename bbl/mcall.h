#ifndef _RISCV_MCALL_H
#define _RISCV_MCALL_H

#define MCALL_HART_ID 300
#define MCALL_CONSOLE_PUTCHAR 301
#define MCALL_CONSOLE_GETCHAR 302
#define MCALL_HTIF_SYSCALL 303
#define MCALL_SEND_IPI 304
#define MCALL_CLEAR_IPI 305
#define MCALL_SHUTDOWN 306
#define MCALL_SET_TIMER 307
#define MCALL_GET_TIMER 308
#define MCALL_REMOTE_SFENCE_VM 309
#define MCALL_REMOTE_FENCE_I 310

#ifndef __ASSEMBLER__

extern uintptr_t do_mcall(uintptr_t which, ...);

#endif

#endif
