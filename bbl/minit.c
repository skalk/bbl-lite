#include "mtrap.h"
#include "atomic.h"
#include "vm.h"
#include "bits.h"
#include "uart.h"
#include <string.h>
#include <limits.h>

pte_t* root_page_table;
uintptr_t first_free_paddr;
uintptr_t mem_size;
uintptr_t num_harts;
uintptr_t rtc_hz;
volatile uint64_t* mtime;
volatile uint32_t* plic_priorities;
size_t plic_ndevs;
volatile uint64_t* ptr_tohost;
volatile uint64_t* ptr_fromhost;
volatile uint8_t* uart_base;

static void mstatus_init()
{
  // Enable FPU and set VM mode
  uintptr_t ms = 0;
  ms = INSERT_FIELD(ms, MSTATUS_VM, VM_CHOICE);
  ms = INSERT_FIELD(ms, MSTATUS_FS, 1);
  write_csr(mstatus, ms);

  // Make sure the hart actually supports the VM mode we want
  ms = read_csr(mstatus);
  assert(EXTRACT_FIELD(ms, MSTATUS_VM) == VM_CHOICE);

  // Enable user/supervisor use of perf counters
  //write_csr(mucounteren, -1);
  //write_csr(mscounteren, -1);
  write_csr(mie, ~MIP_MTIP); // disable timer; enable other interrupts
}

// send S-mode interrupts and most exceptions straight to S-mode
static void delegate_traps()
{
  uintptr_t interrupts = MIP_SSIP | MIP_STIP | MIP_SEIP;
  uintptr_t exceptions =
    (1U << CAUSE_MISALIGNED_FETCH) |
    (1U << CAUSE_FAULT_FETCH) |
    (1U << CAUSE_BREAKPOINT) |
    (1U << CAUSE_FAULT_LOAD) |
    (1U << CAUSE_FAULT_STORE) |
    (1U << CAUSE_BREAKPOINT) |
    (1U << CAUSE_USER_ECALL);

  write_csr(mideleg, interrupts);
  write_csr(medeleg, exceptions);
  assert(read_csr(mideleg) == interrupts);
  assert(read_csr(medeleg) == exceptions);
#if ENABLE_H_MODE
  write_csr(hideleg, interrupts);
  write_csr(hedeleg, exceptions);
  assert(read_csr(hideleg) == interrupts);
  assert(read_csr(hedeleg) == exceptions);
#endif
}

hls_t* hls_init(uintptr_t id)
{
  hls_t* hls = OTHER_HLS(id);
  memset(hls, 0, sizeof(*hls));
  return hls;
}

static uintptr_t sbi_top_paddr()
{
  extern char _end;
  return ROUNDUP((uintptr_t)&_end, RISCV_PGSIZE);
}

static void memory_init()
{
  mem_size = mem_size / MEGAPAGE_SIZE * MEGAPAGE_SIZE;
  first_free_paddr = sbi_top_paddr() + num_harts * RISCV_PGSIZE;
}

static void hart_init()
{
  mstatus_init();
  delegate_traps();
}

static void plic_init()
{
  for (size_t i = 1; i <= plic_ndevs; i++)
    plic_priorities[i] = 1;
}

static void uart_init()
{
  /* enable UART frequency programming */
  uart_base[REG_LCR] = LCR_DLAB;

  /* set highest frequency */
  uart_base[REG_DLL] = 1;
  uart_base[REG_DLM] = 0;

  /* 8-bit data, 1-bit odd parity */
  uart_base[REG_LCR] = LCR_8BIT | LCR_PODD;

  /* interrupt */
  uart_base[REG_IER] = IER_ERBDA;
}

static void hart_plic_init()
{
  // clear pending interrupts
  *HLS()->ipi = 0;
  *HLS()->timecmp = -1ULL;
  write_csr(mip, 0);

  if (!plic_ndevs)
    return;

  size_t ie_words = plic_ndevs / sizeof(uintptr_t) + 1;
  for (size_t i = 0; i < ie_words; i++)
    HLS()->plic_s_ie[i] = ULONG_MAX;
  *HLS()->plic_m_thresh = 1;
  *HLS()->plic_s_thresh = 0;
}

void init_first_hart()
{
  hart_init();
  hls_init(0); // this might get called again from parse_config_string
  parse_config_string();
  plic_init();
  uart_init();
  hart_plic_init();
  memory_init();
  boot_loader();
}

void init_other_hart()
{
  hart_init();
  hart_plic_init();
  boot_other_hart();
}

void enter_supervisor_mode(void (*fn)(uintptr_t), uintptr_t stack)
{
  uintptr_t mstatus = read_csr(mstatus);
  mstatus = INSERT_FIELD(mstatus, MSTATUS_MPP, PRV_S);
  mstatus = INSERT_FIELD(mstatus, MSTATUS_MPIE, 0);
  write_csr(mstatus, mstatus);
  write_csr(mscratch, MACHINE_STACK_TOP() - MENTRY_FRAME_SIZE);
  write_csr(mepc, fn);
  write_csr(sptbr, (uintptr_t)root_page_table >> RISCV_PGSHIFT);
  asm volatile ("mv a0, %0; mv sp, %0; mret" : : "r" (stack));
  __builtin_unreachable();
}
