/*
* network_config.h
*
*  Created on: Nov 27, 2020
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

#ifndef NETWORK_CONFIG_H_
#define NETWORK_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <mcu2pc_config.h>
#include <stm32/random.h>

#define NETWORK_RANDOM() random32()

#define NETWORK_IPV4_SUPPORTED
#define NETWORK_IPV6_SUPPORTED

// These pretty much MUST be set to 0-s in any sane scheme.
#define IPV4_INSIDE_IPV6_0	0
#define IPV4_INSIDE_IPV6_1	0

// Setting IPV4_INSIDE_IPV6_2 to 0 results in an IPv4-compatible IPv6 address.
// Since it is only used internally it works, but that format has been deprecated.
// We should use an IPv4-mapped IPv6 address instead, where the last 32-bits are the IPv4 address and the preceeding 16-bits are set to 0xFFFF.
// So lets do that.
// For further details see: https://datatracker.ietf.org/doc/html/rfc4291

#if defined(LITTLE_ENDIAN)
#define IPV4_INSIDE_IPV6_2	0XFFFF0000
#else
#define IPV4_INSIDE_IPV6_2	0X0000FFFF
#endif

#define NETWORK_ETHERNET_SUPPORTED
#define NETWORK_PPP_SUPPORTED

#define NETWORK_NUMBER_OF_LOOPBACK_BUFFERS 2

#if defined(NETWORK_ETHERNET_SUPPORTED)
extern void Network_NotifyIncomingLoopbackPacket(void);
#define NETWORK_NOTIFY_INCOMING_LOOPBACK_PACKET() Network_NotifyIncomingLoopbackPacket()
#endif

// Note: Currently this only enables/disables calls to the TCP from the rest of the network code.
// TCP sources are still compiled even if the calls are disabled if present in the source directory and the resulting object code
// MAY get linked to you executable, depending on how smart your linker is.
#define NETWORK_TCP_SUPPORTED

// Enable support for UDP sockets.
#define NETWORK_UDP_SOCKET_INTERFACE_SUPPORTED

#define NETWORK_SOCKET_SELECT_SUPPORTED

#if defined(NETWORK_ETHERNET_SUPPORTED) && defined(NETWORK_IPV4_SUPPORTED)
#define NETWORK_IPV4_DHCP_SUPPORTED
#endif

#if defined(NETWORK_IPV4_DHCP_SUPPORTED)
// DHCP timeout out.
#define DHCP_NEGOTIATION_TIMEOUT_MS 240000 /* 4 minutes, really how long should we wait. */
#endif

#if defined(NETWORK_IPV4_SUPPORTED) && !defined(NETWORK_IPV4_DHCP_SUPPORTED)
#define USE_STATIC_IP
#endif

#if defined(USE_STATIC_IP)
#define STATIC_IP_IP_ADDRESS IP4_ADDRESS(192, 168, 0, 200)
#define STATIC_IP_NETMASK    IP4_ADDRESS(255, 255, 255, 0)
#define STATIC_IP_GATEWAY    IP4_ADDRESS(192, 168, 0, 1)
#endif

#define NETWORK_SOFTWARE_CHECKSUMS 1

#if defined(NETWORK_SOFTWARE_CHECKSUMS)
#define NETWORK_SOFTWARE_ICMP_CHECKSUM 1
#endif

#define NETWORK_SPACE_NEEDED_FOR_HW_HEADERS sizeof(ethernet_header_t)
#define NETWORK_SPACE_NEEDED_FOR_IP4_HEADERS sizeof(ip4_header_t)
#define NETWORK_SPACE_NEEDED_FOR_IP6_HEADERS sizeof(ip6_header_t)


#define ETHERNET_BUFFER_FULL_TIMEOUT_MS 16
#define NETWORK_USE_DEFAULT_TIMEOUT (~(uint32_t)0)

// This is what the hardware on the Nucleo-144 uses for its Ethernet frames.
#define ETHERNET_BUFFER_SIZE 1524

#define NETWORK_MAX_MTU 1500
#define NETWORK_DEFAULT_MTU 1500

#if defined(NETWORK_IPV6_SUPPORTED)
#define NETWORK_MIN_MTU 1280
#undef IPV6_LINK_LOCAL_ONLY
//#define IPV6_LINK_LOCAL_ONLY 1
#else
//#define NETWORK_MIN_MTU 68
#define NETWORK_MIN_MTU 576
#endif



#if defined(NETWORK_PPP_SUPPORTED)
#define PPP_DEFAULT_IDLE_TIMEOUT_MS 40000 // 20 seconds
#define PPP_MAX_UNANSWERED_ECHO_REQUESTS 6
#define PPP_DEFAULT_CLIENT_AUTHENTICATION 0 /* Possible values are 0 (no), PPP_PROTOCOL_PAP, PPP_PROTOCOL_CHAP */
#define PPP_DEFAULT_CHAP_REAUTHENTICATIN_PERIOD 0 /* 0 means don't re-authenticate after the initial login. */
#define PPP_MAX_CONFIGURE 10 /* Number of configure requests before giving up. */
#define PPP_MAX_TERMINATE  2 /* Number of terminate requests before giving up. */
#define PPP_MAX_FAILURE 10
#define PPP_RESTART_TIMEOUT_MS 3000 /* 3 seconds */

#define PPP_ALLOW_SERIAL_TIMEOUT

#if defined(PPP_ALLOW_SERIAL_TIMEOUT)
#define PPP_SERIAL_TIMEOUT_MS 2
#endif

// For testing, or statically configuring a compiled in interface ID.
#define PPP_IPV6_INTERFACE_ID { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 }

#if !defined(PPP_IPV6_INTERFACE_ID)
#define PPP_IPV6_INTERFACE_ID { 0 }
#endif

#endif

#if (!defined(NETWORK_IPV6_SUPPORTED)) && (!defined(NETWORK_IPV4_SUPPORTED))
#error At least one of IPv4 or IPv6 must be supported.
#endif

#if defined(NETWORK_IPV6_SUPPORTED)

#if !defined(IPV6_LINK_LOCAL_ONLY)
#define ANNOUNCE_SLAAC_ADDRESS 1 /* Announce the SLAAC address by sending a neghbour advertisement it to FF02::1. */
#endif
#endif

#define ARP_RESPONSE_TIMEOUT_MS 10000

#define SYSLOG_SUPPORT 1
#define SYSLOG_UDP_PORT 514
#if defined(NETWORK_IPV4_SUPPORTED)
// #define SYSLOG_DEFAULT_DESTINATION "192.168.0.29"
// #define SYSLOG_DEFAULT_DESTINATION "zotax.local"
#endif

#define HTTP_SERVER_PORT 80
#define TELNET_SERVER_PORT 23
#define PACKETIZED_PROTOCOL_TCP_PORT 12345
#define DATAGRAM_TRANSPORT_UDP_PORT  12345

#define NETWORK_DNS_SUPPORT

// The well known port number for the DNS server.
#define NETWORK_DNS_PORT 53

// This could be anything higher than 1024 in case lower ports are filtered by routers, etc. in between.
// We assign ephemeral ports numbers starting from 0xC000 so something lower is a good choice.
#define NETWORK_DNS_CLIENT_PORT 1053

// How many name servers can we have?
// Should be at least 2 or 3, but we can have more.
#define NETWORK_MAX_DNS_SERVERS 6

#define NETWORK_DNS_MAX_OUTSTANDING_REQUEST 2

#define DEFAULT_DNS_QUERY_TIMEOUT 1000

#define NETWORK_MDNS_SUPPORT
//#define NETWORK_OUR_HOSTNAME { 7, 's', 't', '_', 't', 'e', 's', 't' }
#define NETWORK_OUR_HOSTNAME { 6, 'N', 'u', 'c', 'l', 'e', 'o' } // Default host name, can be overridden in Forth config scripts.

#ifdef __cplusplus
}
#endif


#endif /* NETWORK_CONFIG_H_ */
