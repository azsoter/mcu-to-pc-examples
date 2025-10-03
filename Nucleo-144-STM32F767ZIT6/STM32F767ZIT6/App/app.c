/*
* app.c
*
*  Created on: Aug 27, 2020
*      Author: Andras Zsoter
*
*      Copyright (c) Andras Zsoter 2020-2025.
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

#include <stdint.h>
#include <string.h>
#include <rtos.h>
#include <board.h>
#include <app.h>

#include <main.h>
#include <gpio.h>
#include <usb_otg.h>
#include <crc.h>
#include <usart.h>
#include <rng.h>
#include <network.h>
#include <net_interface.h>
#include <network_interfaces.h>
#include <net_utils.h>
#include <ethernet.h>
#include <debug_reg.h>
#if defined(NETWORK_TCP_SUPPORTED)
#include <tcp.h>
#endif
#include <formatted_printing.h>
#include <serial.h>
#include <stm32/stm32_serial.h>
#include <packetized.h>
#include <packetized_serial.h>
#include <packetized_datagram.h>
#include <packetized_tcp.h>
#include <calculate_crc.h>
#include <remote_printing.h>
#include <cli.h>
#include <managed_buffers.h>
#include <dhcp.h>
#include <hires_time.h>
#include <stm32/random.h>
#include <printing.h>
#include <sp_httpd.h>
#include <ppp.h>
#include <forth_os.h>
#include <forth_scripting.h>
#include <utility.h>
#include <system_time.h>
#include <syslog.h>
#include <syslog_internal.h>
#if defined(NETWORK_DNS_SUPPORT) || defined(NETWORK_MDNS_SUPPORT)
#include <dns_internal.h>
#include <dns.h>
#endif
#include <version.h>
// -----------------------

#define STACK_SIZE 1024
RTOS_StackItem_t stack[RTOS_Priority_Highest][STACK_SIZE];
RTOS_StackItem_t stack_idle[RTOS_MIN_STACK_SIZE];

RTOS_Task task[RTOS_Priority_Highest];
RTOS_Task task_idle;

#define SCRATCHBUFFER_SIZE 256
uint8_t scratch_buffer[2][SCRATCHBUFFER_SIZE];

// ----------------------------------- Serial communication ----------------------------------------
PacketizedProtoHandler_t pphSerial;
#define SERIAL2_BUFFER_SIZE 0x100
uint8_t serial2_buffer[SERIAL2_BUFFER_SIZE];
Serial_t serial2;

#define SERIAL6_BUFFER_SIZE 3040
uint8_t serial6_buffer[SERIAL6_BUFFER_SIZE];
Serial_t serial6;

void Serial2_ReceiveByte(UART_HandleTypeDef *h)
{
	Serial_ReceiveByte(h, &serial2);
}

void Serial2_SignalTransmit(UART_HandleTypeDef *h)
{
	Serial_SignalTransmit(h, &serial2);
}

void Serial6_ReceiveByte(UART_HandleTypeDef *h)
{
	Serial_ReceiveByte(h, &serial6);
}

void Serial6_SignalTransmit(UART_HandleTypeDef *h)
{
	Serial_SignalTransmit(h, &serial6);
}

void io_task_serial(void *p)
{
	packetized_serial_ReceiverLoop((PacketizedProtoHandler_t *)p);
}
//--------------------------------------------------------------------------------------------------
#if defined(CONFIG_USE_DATAGRAM_BASED_TRANSPORT)
PacketizedProtoHandler_t pphDg;
packetized_datagram_handler_dev_t dgHandlerDev;

void io_task_datagram(void *p)
{
	packetized_datagram_ReceiverLoop((PacketizedProtoHandler_t *)p);
}
#endif // CONFIG_USE_DATAGRAM_BASED_TRANSPORT
//--------------------------------------------------------------------------------------------------
PacketizedProtoHandler_t pphTcp;
void io_task_packetized_tcp(void *p)
{
	packetized_tcp_ReceiverLoop((PacketizedProtoHandler_t *)p);
}
//--------------------------------------------------------------------------------------------------
DATA_ALIGN(unsigned char big_test_buffer[BIG_TEST_BUFFER_SIZE], 64);
//--------------------------------------------------------------------------------------------------
#if 1
void PrintHiresTime(uint64_t ht)
{
	PRINTF("ht = %u.%03u ms\r\n", HIRES_TIME_INT(ht), (uint32_t)((1000 * HIRES_TIME_FRAC(ht)) >> 32));
}

void tst_loop(int cycles)
{
	int i;
	for (i = 0; i < cycles; i++)
	{
		__asm__ __volatile__ ("nop");
	}
}

void ht_test(void)
{
	uint64_t t1, t2;
	//t1 = HiresTime();
	t1 = HiresTime(); tst_loop(10);  t2 = HiresTime();  PUTS("   10: "); PrintHiresTime(t2 - t1); //t1 = t2;
	t1 = HiresTime(); tst_loop(20);  t2 = HiresTime();  PUTS("   20: "); PrintHiresTime(t2 - t1); //t1 = t2;
	t1 = HiresTime(); tst_loop(40);  t2 = HiresTime();  PUTS("   40: "); PrintHiresTime(t2 - t1); //t1 = t2;
	t1 = HiresTime(); tst_loop(50);  t2 = HiresTime();  PUTS("   50: "); PrintHiresTime(t2 - t1); //t1 = t2;
	t1 = HiresTime(); tst_loop(100); t2 = HiresTime();  PUTS("  100: "); PrintHiresTime(t2 - t1); //t1 = t2;
	t1 = HiresTime(); tst_loop(200); t2 = HiresTime();  PUTS("  200: "); PrintHiresTime(t2 - t1); //t1 = t2;
	t1 = HiresTime(); tst_loop(500); t2 = HiresTime();  PUTS("  500: "); PrintHiresTime(t2 - t1); //t1 = t2;
	t1 = HiresTime(); tst_loop(1000); t2 = HiresTime(); PUTS(" 1000: "); PrintHiresTime(t2 - t1); //t1 = t2;
	t1 = HiresTime(); tst_loop(2000); t2 = HiresTime(); PUTS(" 2000: "); PrintHiresTime(t2 - t1); //t1 = t2;
	t1 = HiresTime(); tst_loop(5000); t2 = HiresTime(); PUTS(" 5000: "); PrintHiresTime(t2 - t1); //t1 = t2;
	t1 = HiresTime(); tst_loop(10000); t2 = HiresTime(); PUTS("10000: "); PrintHiresTime(t2 - t1); //t1 = t2;
	t1 = HiresTime(); tst_loop(20000); t2 = HiresTime(); PUTS("20000: "); PrintHiresTime(t2 - t1); //t1 = t2;
	t1 = HiresTime(); tst_loop(40000); t2 = HiresTime(); PUTS("40000: "); PrintHiresTime(t2 - t1); //t1 = t2;
	t1 = HiresTime(); tst_loop(80000); t2 = HiresTime(); PUTS("80000: "); PrintHiresTime(t2 - t1); //t1 = t2;
	t1 = HiresTime(); RTOS_Delay(1000); t2 = HiresTime(); PUTS("RTOS_Delay(1000): "); PrintHiresTime(t2 - t1); //t1 = t2;

}

void print_system_time(void)
{
	char tmp_buff[64];
	PRINTF("System Time = %s\r\n", format_time(get_system_time(), tmp_buff));
}

/*
void dbg3(void)
{
	rtos_StackFrame *sp = (rtos_StackFrame *)(task[RTOS_Priority_TaskMain - 1].SP);
	set_debug_reg(3, sp->hwsaved.pc);
}
*/

#if defined(SYSLOG_SUPPORT)
void syslog_setup(void)
{
#if defined(SYSLOG_DEFAULT_DESTINATION)
	ip_address_t syslog_dest;
	if (IP_EQ(syslog_collector_address, IP_ADDRESS_ZERO))
	{
		memset(&syslog_dest, 0, sizeof(syslog_dest));
		string0_to_ip(&syslog_dest, SYSLOG_DEFAULT_DESTINATION);
		syslog_collector_address = syslog_dest;
	}
#endif
	print_any_ip_address(&syslog_collector_address, "Syslog collector");
	syslog(LOG_INFO|LOG_SYSLOG,"%s", "System has booted up.");
}
#endif

void main_loop(void *p)
{
	int i;
#if defined(NETWORK_TCP_SUPPORTED)
	int f;
#endif

	while (1)
	{
		for (i = 0; i < 100; i++)
		{
			OS_SYS_MS_SLEEP(200);

#if defined(NETWORK_ETHERNET_SUPPORTED)
			ethernet_periodic();
#endif

#if defined(NETWORK_PPP_SUPPORTED)
			ppp_periodic(&PPP_State);
#endif

#if defined(NETWORK_TCP_SUPPORTED)

#if defined(NETWORK_ETHERNET_SUPPORTED)
			f = NetInterface_IsUp(&Ethernet0);
#else
			f = 0;
#endif // NETWORK_ETHERNET_SUPPORTED

#if defined(NETWORK_MDNS_SUPPORT)
			mdns_periodic(&Ethernet0);
#endif

#if defined(NETWORK_PPP_SUPPORTED)
			f |= NetInterface_IsUp(&PPP0Interface);
#endif // NETWORK_PPP_SUPPORTED

			if (f)
			{
				tcp_periodic();
			}

#endif  // NETWORK_TCP_SUPPORTED
		}

		PRINTF("%06x: RX=%5u TX=%5u TCPRX=%5u TCPTX=%5u DgRX=%5u DgTX=%5u\r\n",
				(unsigned int)(OS_SYS_GET_TIME_TICKS()),
				NetStats.ethernet_rx + NetStats.ppp_rx, NetStats.ethernet_tx + NetStats.ppp_tx,
				NetStats.tcp_rx, NetStats.tcp_tx,
				NetStats.datagram_rx, NetStats.datagram_tx);

		print_system_time();
	}
	(void)p;
}
#endif

// --------------------------------------------------------------------------
#if 0
// For some reason the compiler for Nucleo links in a version of memcpy() that was optimized for size,
// and even this gives better performance optimized with -O3.
void *memcpy(void *dst, const  void *src, size_t len)
{
	uint8_t *to = (uint8_t *)dst;
	uint8_t *from = (uint8_t *)src;
	while(len--)
	{
		*to++ = *from++;
	}
	return dst;
}
#else
void faster_memcpy(unsigned char *d, const unsigned char *s, unsigned long len)
{
	//PRINTF("%s(%p, %p, %d)\r\n", __FUNCTION__, d, s, len);

	if ((0 == (1 & (uintptr_t)d)) && (0 == (1 & (uintptr_t)s))) // It seems even addresses are OK even for 4byte operations.
	{
		while(len >= 4)
		{
			*(uint32_t *)d = *(uint32_t *)s;
			d += 4;
			s += 4;
			len -= 4;
		}
	}
	memcpy(d, s, len);
}

#endif

#if 0
void serial_cpy(void *x)
{
	unsigned char c;
		while(1)
		{
			c = serial_getchar(&serial6);
			//serial_putchar(&serial6, c);
			Board_Putc(c);
			//serial_putchar(&serial3, c);
			if (13 == c)
			{
				//serial_putchar(&serial6, 10);
				//serial_putchar(&serial3, 10);
				Board_Putc(10);
			}
		}

}
#endif


extern void sock_test(void *p);
char delayed_config[256];
void taskLevelInit(void *p)
{
#if defined(NETWORK_ETHERNET_SUPPORTED)
	network_interface_t *net;
#endif

	ManagedBuffersInit();
	Serial_Init();
	PrintingInit();
	RemotePrintingInit();
#if defined(INCLUDE_RTC)
	if (0 == rtc_init())
	{
		set_system_time(get_rtc_time());
	}
	print_system_time();
#endif
	ht_test();
	CalculateCrcInit();
	forth_os_init();
#if defined(CONFIG_HAS_CLI)
	CliInit();
#endif
	random_init();

#if defined(CONFIG_USE_DATAGRAM_BASED_TRANSPORT)
	DatagramInit();
#endif // CONFIG_USE_DATAGRAM_BASED_TRANSPORT

	NetworkInit();
#if defined(FORTH_INCLUDE_BLOCKS) && defined(CONFIG_DEVICE_CONFIG_IN_FORTH_BLOCKS)
	forth_run_script_background("1 load", 6, 32, 32);
#endif

	MX_USB_OTG_FS_PCD_Init();
#if defined(NETWORK_ETHERNET_SUPPORTED)
	net = ethernet_init();
	ethernet_hardware_init(net);
#endif
	SerialDevInit(&serial2, &huart2, serial2_buffer, SERIAL2_BUFFER_SIZE, Serial2_ReceiveByte, Serial2_SignalTransmit);
	SerialDevInit(&serial6, &huart6, serial6_buffer, SERIAL6_BUFFER_SIZE, Serial6_ReceiveByte, Serial6_SignalTransmit);

#if 0
	// serial_send(&serial6, "CONNECT\r\n", 9);
	//packetized_serial_InitConnection(&pphSerial, &serial6, scratch_buffer[0], SCRATCHBUFFER_SIZE);
	//RTOS_CreateTask(&(task[RTOS_Priority_Serial_Receiver - 1]),          "I/O_Serial",  RTOS_Priority_Serial_Receiver,         stack[RTOS_Priority_Serial_Receiver - 1],         STACK_SIZE, &serial_cpy, 0);
#else


#if defined(IP_PARSER_TEST)
ip_parser_test();
#endif

#if defined(NETWORK_PPP_SUPPORTED)
	if (0 == ppp_init(&PPP_State, &serial6, &PPP0Interface))
	{
		RTOS_CreateTask(&(task[RTOS_Priority_Serial_Receiver - 1]),          "I/O_PPP",  RTOS_Priority_Serial_Receiver,         stack[RTOS_Priority_Serial_Receiver - 1],         STACK_SIZE, &ppp_loop, &PPP_State);
	}
#else
//	packetized_serial_InitConnection(&pphSerial, &serial2, scratch_buffer[0], SCRATCHBUFFER_SIZE);
	packetized_serial_InitConnection(&pphSerial, &serial6, scratch_buffer[0], SCRATCHBUFFER_SIZE);
	RTOS_CreateTask(&(task[RTOS_Priority_Serial_Receiver - 1]),          "I/O_Serial",  RTOS_Priority_Serial_Receiver,         stack[RTOS_Priority_Serial_Receiver - 1],         STACK_SIZE, &io_task_serial, &pphSerial);
#endif

#endif

#if defined(CONFIG_USE_DATAGRAM_BASED_TRANSPORT)
	packetized_datagram_InitConnection(&pphDg, &dgHandlerDev, &dgHandler);
#endif

#if defined(NETWORK_TCP_SUPPORTED)
	packetized_tcp_InitConnection(&pphTcp, scratch_buffer[1], SCRATCHBUFFER_SIZE);
#endif

#if defined(NETWORK_ETHERNET_SUPPORTED)
#if defined(CONFIG_ETHERNET_TRANSMITTER_THREAD)
	RTOS_CreateTask(&(task[RTOS_Priority_Ethernet_Transmitter - 1]),     "Eth_TX",         RTOS_Priority_Ethernet_Transmitter,    stack[RTOS_Priority_Ethernet_Transmitter - 1],    STACK_SIZE, &ethernet_transmitter_thread, net);

	#endif
	RTOS_CreateTask(&(task[RTOS_Priority_Ethernet_Receiver - 1]),        "Eth_RX",         RTOS_Priority_Ethernet_Receiver,       stack[RTOS_Priority_Ethernet_Receiver - 1],       STACK_SIZE, &ethernet_receiver_thread, net);
#else
	RTOS_CreateTask(&(task[RTOS_Priority_Ethernet_Receiver - 1]),        "LoopBack",       RTOS_Priority_Ethernet_Receiver,       stack[RTOS_Priority_Ethernet_Receiver - 1],       STACK_SIZE, &LoopbackReceiverLoop, 0);
#endif

#if defined(CONFIG_HAS_CLI)
	RTOS_CreateTask(&(task[RTOS_Priority_Cli - 1]),                       "CLI",        RTOS_Priority_Cli,                     stack[RTOS_Priority_Cli - 1],                     STACK_SIZE, &cli_loop, 0);
#endif

#if defined(CONFIG_USE_DATAGRAM_BASED_TRANSPORT)
	RTOS_CreateTask(&(task[RTOS_Priority_Packetized_Eth_Receiver - 1]), "I/O_Datagram", RTOS_Priority_Packetized_Eth_Receiver, stack[RTOS_Priority_Packetized_Eth_Receiver - 1], STACK_SIZE, &io_task_datagram, &pphDg);
#endif // CONFIG_USE_DATAGRAM_BASED_TRANSPORT

	OS_SYS_DELAY(200);
#if defined(NETWORK_ETHERNET_SUPPORTED)
	ethernet_periodic();
#endif
#if defined(NETWORK_TCP_SUPPORTED)
#if defined(CONFIG_INCLUDE_HTTPD)
	RTOS_CreateTask(&(task[RTOS_Priority_Httpd - 1]),                   "HTTPD",               RTOS_Priority_Httpd,                 stack[RTOS_Priority_Httpd - 1],                   STACK_SIZE, &httpd_loop, 0);
#endif
	RTOS_CreateTask(&(task[RTOS_Priority_Task_Test - 1]),               "Socket-Test",         RTOS_Priority_Task_Test,             stack[RTOS_Priority_Task_Test - 1],               STACK_SIZE, &sock_test, 0);
	RTOS_CreateTask(&(task[RTOS_Priority_Comproto_TCP_Receiver - 1]),   "I/O_TCP_Packetized",  RTOS_Priority_Comproto_TCP_Receiver, stack[RTOS_Priority_Comproto_TCP_Receiver - 1],   STACK_SIZE, &io_task_packetized_tcp, &pphTcp);
#endif

	dns_test();

	syslog_init();

#if defined(FORTH_INCLUDE_BLOCKS) && defined(CONFIG_DEVICE_CONFIG_IN_FORTH_BLOCKS)
	if (0 != delayed_config[0])
	{
		forth_run_script_background(delayed_config, strlen(delayed_config), 32, 32);
	}
#endif

#if defined(SYSLOG_SUPPORT)
	syslog_setup();
#endif

	main_loop(p);
}

void dbg1(void)
{

}

int app_start(void)
{
	RTOS_CreateTask(&task_idle, "Idle",   RTOS_Priority_Idle,  stack_idle, RTOS_MIN_STACK_SIZE, &RTOS_DefaultIdleFunction, 0);
	RTOS_CreateTask(&(task[RTOS_Priority_TaskMain - 1]), "Main",  RTOS_Priority_TaskMain, stack[RTOS_Priority_TaskMain - 1], STACK_SIZE, &taskLevelInit, 0);

	// RTOS_CreateSemaphore(&s, 0);

	Board_HardwareInit();					// Initialize hardware as appropriate for the system/board.

	Board_Puts("\r\nHello World!\r\n");
//	Board_Puts("\r\nUSART6->ISR: "); PrintHex(USART6->ISR);
//	Board_Puts("\r\nUSART6->CR3: "); PrintHex(USART6->CR3);
//	Board_Puts("USART2->CR3: "); PrintHex(USART2->CR3);
//	Board_Puts("USART3->CR3: "); PrintHex(USART3->CR3);

	Board_Puts(firmware_compilation_date); Board_Putc(' '); Board_Puts(firmware_compilation_time); Board_Puts("\r\n");
	RTOS_StartMultitasking();
	Board_Puts("Something has gone seriously wrong!\r\n");	// We should never get here!
	while(1);
	return 0;						// Unreachable.
}
