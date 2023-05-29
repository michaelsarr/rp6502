/*
 * Copyright (c) 2023 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "dev/com.h"
#include "cpu.h"
#include "main.h"
#include "mem.h"
#include "mon.h"
#include "pix.h"
#include "ria.h"
#include "str.h"
#include "sys.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/watchdog.h"

#define TIMEOUT_MS 200

static enum {
    SYS_IDLE,
    SYS_READ,
    SYS_WRITE,
    SYS_VERIFY,
} cmd_state;

static uint32_t rw_addr;
static uint32_t rw_len;
static uint32_t rw_crc;

static void cmd_ria_read()
{
    cmd_state = SYS_IDLE;
    if (ria_print_error_message())
        return;
    printf("%04lX", rw_addr);
    for (size_t i = 0; i < mbuf_len; i++)
        printf(" %02X", mbuf[i]);
    printf("\n");
}

static void cmd_ria_write()
{
    cmd_state = SYS_IDLE;
    if (ria_print_error_message())
        return;
    cmd_state = SYS_VERIFY;
    ria_verify_mbuf(rw_addr);
}

static void cmd_ria_verify()
{
    cmd_state = SYS_IDLE;
    ria_print_error_message();
}

// Commands that start with a hex address. Read or write memory.
void sys_address(const char *args, size_t len)
{
    // addr syntax is already validated by dispatch
    rw_addr = 0;
    size_t i = 0;
    for (; i < len; i++)
    {
        char ch = args[i];
        if (char_is_hex(ch))
            rw_addr = rw_addr * 16 + char_to_int(ch);
        else
            break;
    }
    for (; i < len; i++)
        if (args[i] != ' ')
            break;
    if (rw_addr > 0x1FFFF)
    {
        printf("?invalid address\n");
        return;
    }
    if (i == len)
    {
        mbuf_len = (rw_addr | 0xF) - rw_addr + 1;
        if (rw_addr > 0xFFFF)
        {
            printf("%04lX", rw_addr);
            rw_addr -= 0x10000;
            for (size_t i = 0; i < mbuf_len; i++)
                printf(" %02X", xram[rw_addr + i]);
            printf("\n");
            return;
        }
        ria_read_mbuf(rw_addr);
        cmd_state = SYS_READ;
        return;
    }
    uint32_t data = 0x80000000;
    mbuf_len = 0;
    for (; i < len; i++)
    {
        char ch = args[i];
        if (char_is_hex(ch))
            data = data * 16 + char_to_int(ch);
        else if (ch != ' ')
        {
            printf("?invalid data character\n");
            return;
        }
        if (ch == ' ' || i == len - 1)
        {
            if (data < 0x100)
            {
                mbuf[mbuf_len++] = data;
                data = 0x80000000;
            }
            else
            {
                printf("?invalid data value\n");
                return;
            }
            for (; i + 1 < len; i++)
                if (args[i + 1] != ' ')
                    break;
        }
    }
    if (rw_addr > 0xFFFF)
    {
        rw_addr -= 0x10000;
        for (size_t i = 0; i < mbuf_len; i++)
        {
            xram[rw_addr + i] = mbuf[i];
            pix_send_blocking(0, 0, mbuf[i], rw_addr + i);
        }
        return;
    }
    ria_write_mbuf(rw_addr);
    cmd_state = SYS_WRITE;
}

void sys_reboot(const char *args, size_t len)
{
    (void)(args);
    (void)(len);
    watchdog_reboot(0, 0, 0);
}

void sys_reset_6502(const char *args, size_t len)
{
    (void)(args);
    (void)(len);
    main_run();
}

static void sys_com_rx_mbuf()
{
    cmd_state = SYS_IDLE;
    if (mbuf_len < rw_len)
    {
        puts("?timeout");
        return;
    }
    if (mbuf_crc32() != rw_crc)
    {
        puts("?CRC does not match");
        return;
    }
    cmd_state = SYS_WRITE;
    ria_write_mbuf(rw_addr);
}

void sys_binary(const char *args, size_t len)
{
    if (parse_uint32(&args, &len, &rw_addr) &&
        parse_uint32(&args, &len, &rw_len) &&
        parse_uint32(&args, &len, &rw_crc) &&
        parse_end(args, len))
    {
        if (rw_addr > 0xFFFF)
        {
            printf("?invalid address\n");
            return;
        }
        if (!rw_len || rw_len > MBUF_SIZE || rw_addr + rw_len > 0x10000)
        {
            printf("?invalid length\n");
            return;
        }
        com_capture_mbuf(sys_com_rx_mbuf, rw_len, TIMEOUT_MS);
        return;
    }
    printf("?invalid argument\n");
}

void sys_task()
{
    if (ria_is_running())
        return;
    switch (cmd_state)
    {
    case SYS_IDLE:
        break;
    case SYS_READ:
        cmd_ria_read();
        break;
    case SYS_WRITE:
        cmd_ria_write();
        break;
    case SYS_VERIFY:
        cmd_ria_verify();
        break;
    }
}

bool sys_is_active()
{
    return cmd_state != SYS_IDLE;
}

void sys_reset()
{
    cmd_state = SYS_IDLE;
}
