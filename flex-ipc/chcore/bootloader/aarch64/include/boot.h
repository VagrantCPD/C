#pragma once

#include "image.h"

extern void el1_mmu_activate(void);
extern void start_kernel(void *boot_flag);
extern int get_current_el(void);
extern void init_boot_pt(void);
extern void secondary_cpu_boot(int cpuid);

/* SMC call to support PSCI */
extern unsigned long boot_smc_call(unsigned long a0, unsigned long a1,
				   unsigned long a2, unsigned long a3,
				   unsigned long a4, unsigned long a5,
				   unsigned long a6, unsigned long a7);
/* DCache flush */
extern void __flush_dcache_area(void *start_addr, unsigned long size);

/* symbol in linker */
extern char _edata;
extern char img_start;
extern char img_end;

extern char _bss_start;
extern char _bss_end;

/* assembly macros */
#define __nops(n)		".rept  " #n "\nnop\n.endr\n"
#define nops(n)			asm volatile(__nops(n))
#define HLT			asm volatile("hlt #0")

#define ALIGN(n)		__attribute__((__aligned__(n)))
#define IS_ALIGNED(x, a)	(((x) & ((typeof(x))(a) - 1)) == 0)
#define BIT(x)			(1UL << (x))
#define PAGE_BITS		12
#define VISIBLE			__attribute__((externally_visible))
#define CONFIG_MAX_NUM_CPUS	64

#define NULL			0

typedef unsigned int size_t;
