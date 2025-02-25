/*
 * Copyright (c) 2023 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _API_H_
#define _API_H_

#include "sys/ria.h"

// RIA XRAM portals
#define API_RW0 REGS(0xFFE4)
#define API_STEP0 *(int8_t *)&REGS(0xFFE5)
#define API_ADDR0 REGSW(0xFFE6)
#define API_RW1 REGS(0xFFE8)
#define API_STEP1 *(int8_t *)&REGS(0xFFE9)
#define API_ADDR1 REGSW(0xFFEA)

// RIA fastcall registers
#define API_OP REGS(0xFFEF)
#define API_ERRNO REGSW(0xFFED)
#define API_STACK REGS(0xFFEC)
#define API_BUSY (REGS(0xFFF2) & 0x80)
#define API_A REGS(0xFFF6)
#define API_X REGS(0xFFF4)
#define API_SREG REGSW(0xFFF8)
#define API_AX (API_A | (API_X << 8))
#define API_AXSREG (API_AX | (API_SREG << 16))

// 64KB Extended RAM
#ifdef NDEBUG
extern uint8_t xram[0x10000];
#else
extern uint8_t *const xram;
#endif

// The xstack is:
// 256 bytes, enough to hold a CC65 stack frame.
// 1 byte at end+1 always zero for cstrings.
// Many OS calls can use xstack instead of xram for cstrings.
// Using xstack doesn't require sending the zero termination.
// Cstrings and data are pushed in reverse so data is ordered correctly on the top down stack.
#define XSTACK_SIZE 0x100
extern uint8_t xstack[];
extern volatile size_t xstack_ptr;

// Kernel events
void api_task(void);
void api_run(void);

// How to build an API handler:
// 1. The last paramater is in API_AX or API_AXSREG.
// 2. Stack was pushed "in order". Like any top-down stack.
// 3. First parameter may support a "short stack".
//    e.g. a uint16 is sent for fseek instead of a uint64.
// 4. Be careful with the stack. Especially returning xstack_ptr.
// 5. Registers must be refreshed if XSTACK data changes.
// 6. Use the return functions always!

// Helpers for a "short" stack.
// success = (xstack_ptr == XSTACK_SIZE)
uint16_t api_sstack_uint16(void);
uint32_t api_sstack_uint32(void);
uint64_t api_sstack_uint64(void);
int16_t api_sstack_int16(void);
int32_t api_sstack_int32(void);
int64_t api_sstack_int64(void);

// Returning data on XSTACK requires
// ensuring the REGS have fresh data.
static inline void api_sync_xstack()
{
    API_STACK = xstack[xstack_ptr];
}

// Return works by manipulating 10 bytes of registers.
// FFF0 EA      NOP
// FFF1 80 FE   BRA -2
// FFF3 A2 FF   LDX #$FF
// FFF5 A9 FF   LDA #$FF
// FFF7 60      RTS
// FFF8 FF FF   .SREG $FF $FF
static inline void api_return_blocked() { *(uint32_t *)&ria_regs[0x10] = 0xA2FE80EA; }
static inline void api_return_released() { *(uint32_t *)&ria_regs[0x10] = 0xA20080EA; }

static inline void api_set_ax(uint16_t val)
{
    *(uint32_t *)&ria_regs[0x14] = 0x6000A900 | ((val >> 8) & 0xFF) | ((val << 16) & 0xFF0000);
}

static inline void api_set_axsreg(uint32_t val)
{
    api_set_ax(val);
    API_SREG = val >> 16;
}

// Call one of these at the very end. These signal
// the 6502 that the operation is complete.
static inline void api_return_ax(uint16_t val)
{
    api_set_ax(val);
    api_return_released();
}
static inline void api_return_axsreg(uint32_t val)
{
    api_set_axsreg(val);
    api_return_released();
}
static inline void api_return_errno_ax(uint16_t errno, uint16_t val)
{
    API_ERRNO = errno;
    api_return_ax(val);
}
static inline void api_return_errno_ax_zxstack(uint16_t errno, uint16_t val)
{
    xstack_ptr = XSTACK_SIZE;
    api_return_errno_ax(errno, val);
}
static inline void api_return_errno_axsreg(uint16_t errno, uint32_t val)
{
    API_ERRNO = errno;
    api_return_axsreg(val);
}
static inline void api_return_errno_axsreg_zxstack(uint16_t errno, uint32_t val)
{
    xstack_ptr = XSTACK_SIZE;
    api_return_errno_axsreg(errno, val);
}
#endif /* _API_H_ */
