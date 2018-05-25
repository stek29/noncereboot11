#pragma once

#include <mach/mach.h>
#include <inttypes.h>

kern_return_t init_tfpzero(void);

size_t rkbuffer(uint64_t where, void *p, size_t size);
uint32_t rk32(uint64_t where);
uint64_t rk64(uint64_t where);

size_t wkbuffer(uint64_t where, const void *p, size_t size);
void wk32(uint64_t where, uint32_t what);
void wk64(uint64_t where, uint64_t what);

uint64_t kalloc(uint64_t size);
uint64_t kalloc_wired(uint64_t size);
void kfree(uint64_t kaddr, uint64_t size);

