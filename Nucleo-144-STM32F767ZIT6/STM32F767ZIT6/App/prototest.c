/*
* prototest.c
*
*  Created on: Feb 16, 2021
*      Author: Andras Zsoter
*
*      Copyright (c) Andras Zsoter 2021-2025.
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
#include <prototest.h>
#include <os_sys.h>
#include <packetized.h>
#include <packetized_datagram.h>
#include <datagram_proto.h>
#if defined(CONFIG_USE_DATAGRAM_BASED_TRANSPORT)
#include <datagram_proto_interface.h>
#include <datagram_proto_handler.h>
#endif // CONFIG_USE_DATAGRAM_BASED_TRANSPORT

#include <hires_time.h>
#include <printing.h>
#include <remote_printing.h>

extern PacketizedProtoHandler_t pphDg;


void sendTestPacket(PacketizedProtoHandler_t *pph, int use_crc)
{
	int res;
	OS_SYS_Time start_time = OS_SYS_GET_TIME_TICKS();
	OS_SYS_Time delta;
	PacketHeader_t header;
	uint32_t length = sizeof(big_test_buffer) - 32;
#if defined(CONFIG_USE_DATAGRAM_BASED_TRANSPORT)
	DatagramHandler_t *dgh;
	uint64_t dt;
#endif // CONFIG_USE_DATAGRAM_BASED_TRANSPORT

	if (use_crc)
	{
		packetized_MakeHeader(&header, PACKET_INFO_FLAGS_REQUEST|PACKET_INFO_USE_CRC|PACKET_ROUTING_SYSTEM, PACKET_TYPE_SYSTEM_PROTO_TEST, length);
	}
	else
	{
		packetized_MakeHeader(&header, PACKET_INFO_FLAGS_REQUEST|PACKET_ROUTING_SYSTEM, PACKET_TYPE_SYSTEM_PROTO_TEST, length);
	}

	res = packetized_SendPacket(pph, &header, big_test_buffer + sizeof(PacketHeader_t));
	delta = (OS_SYS_GET_TIME_TICKS()) - start_time;
	PRINTF("\r\nsizeof(big_test_buffer) = %u\r\n", sizeof(big_test_buffer));
	PRINTF("DeltaT = %d ticks (%dms), res = %d.\r\n\r\n", delta, OS_SYS_TICKS_TO_MS(delta), res);

#if defined(CONFIG_USE_DATAGRAM_BASED_TRANSPORT)
	if (&pphDg == pph)
	{
		dgh = ((packetized_datagram_handler_dev_t *)(pph->dev))->dgHandler;
		if (0 != dgh)
		{
			dt = dgh->dt_tx;

			PRINTF("dt_tx = %u.%03u ms\r\n", HIRES_TIME_INT(dt), (uint32_t)((1000 * HIRES_TIME_FRAC(dt)) >> 32));
			debug_printf(pph, "Segments = %u, Retries = %u, Gap = %u us, ACKs = %u, NACKs = %u ACKS(peer) = %u ACKsTX = %u\r\n",
					dgh->segment_count, dgh->retry_count, dgh->usGap,
					dgh->segment_ack_count, dgh->segment_nak_count,
					dgh->segment_acks_reported, dgh->segment_acks_sent);
			debug_printf(pph, "dt_tx = %u.%03u ms\r\n", HIRES_TIME_INT(dt), (uint32_t)((1000 * HIRES_TIME_FRAC(dt)) >> 32));
		}
	}
#endif // CONFIG_USE_DATAGRAM_BASED_TRANSPORT

	debug_printf(pph, "DeltaT = %d ticks (%dms), size = %u, res = %d.\r\n\r\n", delta, OS_SYS_TICKS_TO_MS(delta), sizeof(big_test_buffer), res);
}

#if defined(CONFIG_USE_DATAGRAM_BASED_TRANSPORT)
void sendLowLevelEchoRequest(PacketizedProtoHandler_t *pph)
{
	uint64_t dly = 0;
	struct DgRTTMState rttm;
	int res;

	res = packetized_ioctl(pph, PACKETIZED_IOCTL_DATAGRAM_SEND_TIMED_ECHO_REQUEST, 0);

	if (0 > res)
	{
		PRINTF("ioctl() to send echo request has failed res = %d.\r\n", res);
		return;
	}

	OS_SYS_MS_SLEEP(100);

	res = packetized_ioctl(pph, PACKETIZED_IOCTL_DATAGRAM_GET_ECHO_REPLY_DELAY, &dly);

	if (0 > res)
	{
		PRINTF("ioctl() to retrieve delay has failed res = %d.\r\n", res);
		return;
	}

	PRINTF("EchoReplyDelay = %u.%03u ms\r\n", HIRES_TIME_INT(dly), (uint32_t)((1000 * HIRES_TIME_FRAC(dly)) >> 32));
	debug_printf(pph, "EchoReplyDelay = %u.%03u ms\r\n", HIRES_TIME_INT(dly), (uint32_t)((1000 * HIRES_TIME_FRAC(dly)) >> 32));

	res = packetized_ioctl(pph, PACKETIZED_IOCTL_DATAGRAM_GET_RTTM, &rttm);

	if (0 > res)
	{
		PRINTF("ioctl() to retrieve RTTM has failed res = %d.\r\n", res);
		return;
	}

	PRINTF("RTTM: SRTT=%6u, RTTVAR=%6u RTO=%6u\r\n", rttm.srtt, rttm.rtt_var, rttm.rto);
	debug_printf(pph, "RTTM: SRTT=%6u, RTTVAR=%6u RTO=%6u\r\n", rttm.srtt, rttm.rtt_var, rttm.rto);
}
#endif // CONFIG_USE_DATAGRAM_BASED_TRANSPORT

void mem_cpy(uint8_t *to, const uint8_t *from, int bytes)
{
	while(bytes--)
	{
		*to++ = *from++;
	}
}

void copyTest(PacketizedProtoHandler_t *pph)
{
	uint64_t dly, t;
	t = HiresTime();
	memmove(big_test_buffer + 64, big_test_buffer, sizeof(big_test_buffer) - 64);
	dly = HiresTime() - t;
	PRINTF("CopyDelay (memcpy) = %u.%03u\r\n", HIRES_TIME_INT(dly), (uint32_t)((1000 * HIRES_TIME_FRAC(dly)) >> 32));
	debug_printf(pph, "CopyDelay (memcpy) = %u.%03u\r\n", HIRES_TIME_INT(dly), (uint32_t)((1000 * HIRES_TIME_FRAC(dly)) >> 32));
	t = HiresTime();
	mem_cpy(big_test_buffer + 64, big_test_buffer, sizeof(big_test_buffer) - 64);
	dly = HiresTime() - t;
	PRINTF("CopyDelay (mem_cpy) = %u.%03u\r\n", HIRES_TIME_INT(dly), (uint32_t)((1000 * HIRES_TIME_FRAC(dly)) >> 32));
	debug_printf(pph, "CopyDelay (mem_cpy) = %u.%03u\r\n", HIRES_TIME_INT(dly), (uint32_t)((1000 * HIRES_TIME_FRAC(dly)) >> 32));
#if 0
	t = HiresTime();
	memmove(big_test_buffer + 64, big_test_buffer, sizeof(big_test_buffer) - 64);
	dly = HiresTime() - t;
	PRINTF("CopyDelay (memmove) = %u.%03u\r\n", HIRES_TIME_INT(dly), (uint32_t)((1000 * HIRES_TIME_FRAC(dly)) >> 32));
#endif
}

#if defined(CONFIG_USE_DATAGRAM_BASED_TRANSPORT)
void PacketizedDatagramProtoTest(void)
{
	int i;
	int is_ipv6;
	ip_address_t peer_ip;
	network_packet_info_t details;

	//network_verbose |= NETWORK_VERBOSE_DATAGRAM;

#if defined(NETWORK_IPV6_SUPPORTED)
	is_ipv6 = 1;
	peer_ip = LoopBackInterface.net_my_link_local_ip6_address;
#else
#if defined(NETWORK_IPV4_SUPPORTED)
	SET_IPv4_Addr(peer_ip, LoopBackInterface.net_my_ip_addr);
	is_ipv6 = 0;
#else
#error Neither IPv4 nor IPv6 is supported.
#endif
#endif

	if (0 != FillConnectionDetailsForDatagramProtocol(&details, &LoopBackInterface, peer_ip, is_ipv6))
	{
		PRINTF("Failed to fill in connection details.\r\n");
		return;
	}

	SendConnectRequestDatagram(&dgHandler, &details);
	OS_SYS_MS_SLEEP(10);
	ResetDatagramHandlerState(&dgHandler);
#if 1
	for (i = 0; i < 4; i++)
	{
		sendLowLevelEchoRequest(&pphDg);
		OS_SYS_MS_SLEEP(10);
	}
	sendTestPacket(&pphDg, 0);
#endif
	OS_SYS_MS_SLEEP(100);
	SendDisconnectRequestDatagram(&dgHandler);
}
#endif // CONFIG_USE_DATAGRAM_BASED_TRANSPORT
