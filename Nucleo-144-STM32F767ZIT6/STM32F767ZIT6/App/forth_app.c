/*
* Copyright (c) 2024-2025 Andras Zsoter
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*/

#include <string.h>
#include <forth.h>
#include <forth_internal.h>
#include <forth_os.h>
#include <network_config.h>
#include <version.h>
#include <app.h>

// #define FORTH_APP_MINIMAL 1

#define FORTH_INCLUDE_PPH_TESTS

// .SYS ( -- )
void forth_dot_sys_local(forth_runtime_context_t *rctx)
{
#if 0
	forth_TYPE0(rctx, "Seamless Forth Scripting Engine");
	forth_cr(rctx);
	forth_TYPE0(rctx, "Version: ");
	forth_print_version(rctx);
	forth_cr(rctx);
#else
	extern void forth_dot_sys(forth_runtime_context_t *rctx);
	forth_dot_sys(rctx);
#endif
	forth_TYPE0(rctx, "Compiled on ");
	forth_TYPE0(rctx, firmware_compilation_date);
	forth_space(rctx);
	forth_TYPE0(rctx, firmware_compilation_time);
	forth_cr(rctx);
}

#if !defined(FORTH_APP_MINIMAL)
#include <os_sys.h>
#include <debug_reg.h>
#include <board.h>

#if defined(NETWORK_IPV4_SUPPORTED) || defined(NETWORK_IPV6_SUPPORTED)
#include <network.h>
#include <net_interface.h>
#include <network_interfaces.h>
#endif

#if defined(NETWORK_PPP_SUPPORTED)
#include <ppp.h>
#endif

#if defined(INCLUDE_LED_DEMO)
#include <led.h>

// led@ ( led-index -- led-value ) if value is -1 there is no such LED.
void forth_led_fetch(forth_runtime_context_t *rctx)
{
	rctx->sp[0] = led_status(rctx->sp[0]);
}

// led! ( led-value led-index -- )
void forth_led_store(forth_runtime_context_t *rctx)
{
	forth_cell_t index = forth_POP(rctx);
	forth_cell_t value = forth_POP(rctx);
	led_control(index, value);
}
#endif

#if 1
#include <usart.h>
#include <serial.h>
extern Serial_t serial6;
extern RTOS_Task task[];

// .DEBUG ( -- )
void forth_print_debug(forth_runtime_context_t *ctx)
{
	uint32_t reg;

	// dbg3();
	forth_print_debug_regs(ctx);
	reg = READ_REG (USART6->ISR);
	forth_TYPE0(ctx, "USART6 ISR = "); forth_PUSH(ctx, reg); forth_hdot(ctx); forth_cr(ctx);
	reg = READ_REG (USART6->CR3);
	forth_TYPE0(ctx, "USART6 CR3 = "); forth_PUSH(ctx, reg); forth_hdot(ctx); forth_cr(ctx);

	forth_PUSH(ctx, serial6.tail);
	forth_PUSH(ctx, serial6.head);
	forth_TYPE0(ctx, "serial6.head = "); forth_dot(ctx);
	forth_TYPE0(ctx, "serial6.tail = "); forth_dot(ctx); forth_cr(ctx);

	forth_PUSH(ctx, serial6.tx_count);
	forth_TYPE0(ctx, "serial6.tx_count = "); forth_dot(ctx); forth_cr(ctx);
	forth_PUSH(ctx, serial6.rx_count);
	forth_TYPE0(ctx, "serial6.rx_count = "); forth_dot(ctx); forth_cr(ctx);
	forth_PUSH(ctx, serial6.rx_overruns);
	forth_TYPE0(ctx, "serial6.rx_overruns = "); forth_dot(ctx); forth_cr(ctx);

#if defined(NETWORK_PPP_SUPPORTED)
	forth_PUSH(ctx, PPP_State.TransmitterLock.Event.TasksWaiting);
	forth_TYPE0(ctx, "Tasks waiting to lock the PPP Transmitter = "); forth_hdot(ctx); forth_cr(ctx);

	forth_PUSH(ctx, ((rtos_StackFrame *)(task[1].SP))->hwsaved.pc);
	forth_TYPE0(ctx, "PPP Task PC = "); forth_hdot(ctx); forth_cr(ctx);

	forth_PUSH(ctx, PPP_State.lcp_state);
	forth_PUSH(ctx, PPP_State.phase);
	forth_TYPE0(ctx, "PPP Phase = "); forth_hdot(ctx); forth_cr(ctx);
	forth_TYPE0(ctx, "PPP LCP State = "); forth_hdot(ctx); forth_cr(ctx);
#endif
}
#else
// .DEBUG ( -- )
void forth_print_debug(forth_runtime_context_t *ctx)
{
	forth_print_debug_regs(ctx);
}
#endif

// delayed-config! ( c-addr -- len -- )
void forth_delayed_config_store(forth_runtime_context_t *rctx)
{
	forth_cell_t length = forth_POP(rctx);
	char *addr = (char *)forth_POP(rctx);

	if (0 == length)
	{
		delayed_config[0] = 0;
		return;
	}

	if (0 == addr)
	{
		forth_THROW(rctx, -9); // Invalid memory address.
	}

	if ((sizeof(delayed_config) - 1) < length)
	{
		forth_THROW(rctx, -11); // Out of range(?)
	}

	memcpy(delayed_config, addr, length);
	delayed_config[length] = 0;
}

#if defined(FORTH_INCLUDE_PPH_TESTS)
#include <packetized.h>
#include <prototest.h>
#if defined(CONFIG_USE_DATAGRAM_BASED_TRANSPORT)

// timed-echo ( -- )
void forth_send_timed_echo(forth_runtime_context_t *ctx)
{
	if ((FORTH_FEATURE_SELECTOR_PACKETIZED_CLI == ctx->extra.selector) && (0 != ctx->extra.caller_context))
	{
		sendLowLevelEchoRequest((PacketizedProtoHandler_t *)ctx->extra.caller_context);
	}
	else
	{
		forth_THROW(ctx, -21); // unsupported operation
	}
}

// test-dg ( -- )
void forth_test_dg(forth_runtime_context_t *rctx)
{
	PacketizedDatagramProtoTest();
}

#endif // CONFIG_USE_DATAGRAM_BASED_TRANSPORT

// proto-test ( use_crc_flag -- )
void forth_send_test_packet(forth_runtime_context_t *rctx)
{
	if ((FORTH_FEATURE_SELECTOR_PACKETIZED_CLI == rctx->extra.selector) && (0 != rctx->extra.caller_context))
	{
		sendTestPacket((PacketizedProtoHandler_t *)rctx->extra.caller_context, (int)forth_POP(rctx));
	}
	else
	{
		forth_THROW(rctx, -21); // unsupported operation
	}
}

// copy-test ( -- )
void forth_send_copy_test(forth_runtime_context_t *rctx)
{
	if ((FORTH_FEATURE_SELECTOR_PACKETIZED_CLI == rctx->extra.selector) && (0 != rctx->extra.caller_context))
	{
		copyTest((PacketizedProtoHandler_t *)rctx->extra.caller_context);
	}
	else
	{
		forth_THROW(rctx, -21); // unsupported operation
	}
}
#endif


// REBOOT ( -- )
void forth_reboot(forth_runtime_context_t *rctx)
{
	Board_Reboot();
}

void forth_test_xx(forth_runtime_context_t *ctx)
{
extern void  bkpsram_init(void);
	 bkpsram_init();
}
#endif // FORTH_APP_MINIMAL

const forth_vocabulary_entry_t forth_wl_application[] =
{
DEF_FORTH_WORD(".SYS",        0, forth_dot_sys_local, "( -- )"), // This will override the other implementation in forth_os.c.

#if !defined(FORTH_APP_MINIMAL)

#if defined(INCLUDE_LED_DEMO)
DEF_FORTH_WORD("led@",  0, forth_led_fetch,  "( led-index -- led-value ) if value is -1 there is no such LED."),
DEF_FORTH_WORD("led!",  0, forth_led_store,  "( led-value led-index -- )"),
DEF_FORTH_WORD(".debug",0, forth_print_debug,    "( -- )"),
#endif

DEF_FORTH_WORD("delayed-config!", 0, forth_delayed_config_store, "( c-addr -- len -- )" ),

#if defined(FORTH_INCLUDE_PPH_TESTS)
#if defined(CONFIG_USE_DATAGRAM_BASED_TRANSPORT)
DEF_FORTH_WORD("timed-echo",  0, forth_send_timed_echo,  "( -- )"),
DEF_FORTH_WORD("test-dg",     0, forth_test_dg,          "( -- )"),
#endif // CONFIG_USE_DATAGRAM_BASED_TRANSPORT
DEF_FORTH_WORD("proto-test",  0, forth_send_test_packet, "( use_crc_flag -- )"),
DEF_FORTH_WORD("copy-test",   0, forth_send_copy_test,   "( -- )"),
#endif

DEF_FORTH_WORD("test-xx", 0, forth_test_xx, " ( ??? )"),
DEF_FORTH_WORD("reboot",0, forth_reboot,      	 "( -- )"),

#endif // FORTH_APP_MINIMAL

DEF_FORTH_WORD(0, 0, 0, 0)
};
