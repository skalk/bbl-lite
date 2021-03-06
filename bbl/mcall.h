#ifndef _RISCV_MCALL_H
#define _RISCV_MCALL_H

#define MCALL_HART_ID 0
#define MCALL_CONSOLE_PUTCHAR 1
#define MCALL_CONSOLE_GETCHAR 2
#define MCALL_HTIF_SYSCALL 3
#define MCALL_SEND_IPI 4
#define MCALL_CLEAR_IPI 5
#define MCALL_SHUTDOWN 6
#define MCALL_SET_TIMER 7
#define MCALL_REMOTE_SFENCE_VM 8
#define MCALL_REMOTE_FENCE_I 9
#define MCALL_TIMEBASE 10

#ifndef __ASSEMBLER__

extern uintptr_t do_mcall(uintptr_t which, ...);

#endif

#endif
