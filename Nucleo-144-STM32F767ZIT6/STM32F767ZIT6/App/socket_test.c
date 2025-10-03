/*
* socket_test.c
*
*  Created on: Nov 23, 2020
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

#include <api_error.h>
#include <string.h>
#include <in.h>
#include <socket.h>
#include <net_utils.h>
#include <printing.h>
#include <network.h>
#include <forth_os.h>

uint8_t sock_test_buffer[256];
#if defined(NETWORK_TCP_SUPPORTED)
void send_some_stuff(int sd)
{
	int i;
	int res;
	for (i = 0; i < 100; i++)
	{
		SNPRINTF((char *)sock_test_buffer, sizeof(sock_test_buffer),"Just printing some random stuff = %10d\r\n", i);
		//PUTS(sock_test_buffer);
		res = socket_write_blocking(sd, sock_test_buffer, strlen((char *)sock_test_buffer));
		//PRINTF("socket_write_blocking() has returned = %d\r\n", res);
		// OS_SYS_DELAY(1);
	}
	(void)res;
}

int read_some_data(int sd)
{
	int res;
	int j;
	res = socket_read_nonblocking(sd, sock_test_buffer, sizeof(sock_test_buffer));

	//PRINTF("read_socket() has returned = %d\r\n", res);

	if (0 < res)
	{
		for (j = 0; j < res; j++)
		{
			if ('\n' == sock_test_buffer[j])
			{
				PUTC('\r');
			}
			PUTC(sock_test_buffer[j]);
		}
	}
	return res;
}
void send_and_receive_stuff(int sd)
{
	int i;
	int res;
	for (i = 0; i < 100; i++)
	{
		res = read_some_data(sd);
		if (0 == res)
		{
			return;
		}

		if (0 > res)
		{

			if (API_EAGAIN == res)
			{
				OS_SYS_MS_SLEEP(10);
			}
			else
			{
				return;
			}
		}

		SNPRINTF((char *)sock_test_buffer, sizeof(sock_test_buffer),"Just printing some random stuff = %10d\r\n", i);
		//PUTS(sock_test_buffer);
		res = socket_write_blocking(sd, sock_test_buffer, strlen((char *)sock_test_buffer));
		if (0 > res)
		{
			PRINTF("socket_write_blocking() has returned = %d\r\n", res);
			return;
		}

		// PRINTF("socket_write_blocking() has returned = %d\r\n", res);

		if (0 == res)
		{
			break;
		}
		//PRINTF("socket_write_blocking() has returned = %d\r\n", res);
		// OS_SYS_DELAY(1);
	}
	PUTS("\r\n-----------------------------------------------------------------------------\r\n");
	socket_flush(sd);
	res = socket_shutdown(sd, SHUT_WR); // Create an EOF on the sending side.
	PRINTF("socket_shudown(%d) has returned %d.\r\n", sd, res);
	do {
		// PUTC('*');
		res = read_some_data(sd);
		if (0 > res)
		{
			if (API_EAGAIN == res)
			{
				OS_SYS_MS_SLEEP(10);
			}
			else
			{
				return;
			}
//			PRINTF("res = %d\r\n", res);
		}
	} while( 0 != res);
	(void)res;
}

void sock_test_listen(void)
{
	struct sockaddr_in address;
	socklen_t addrlen = sizeof(address);
	int res, sc, s;

	s = tcp_server_socket(23, 1);

	if (0 > s)
	{
		PUTS("sock_test_listen(): Failed to create server socket\r\n");
		return;
	}

	while (1)
	{
		sc = socket_accept(s, (struct sockaddr *)&address, &addrlen, 0);

		if (0 > sc)
		{
			PRINTF("ERROR: socket_accept(%d,..) returned=%d\r\n", s, sc);
		}
		else
		{
			PRINTF("Accepted socket = %u\r\n", sc);
		}

		send_and_receive_stuff(sc);

		res = socket_close(sc);
		PRINTF("socket_close(%d) has returned = %d\r\n", sc, res);
	}
}

// Test with e.g. netcat.
// nc -6 -l -p 55667
void sock_test2(int sel)
{
	struct sockaddr_in address = { 0 };
	// int addrlen = sizeof(address);
	int res, s, i;
	char *str;
	ip4_address_t peer_ip = IP4_ADDRESS(192, 168, 1, 105);
#if defined(NETWORK_IPV6_SUPPORTED)
	//ip6_address_t peer_ip6 = {{ 0xFE, 0x80, 0, 0, 0, 0, 0, 0, 0x44, 0xEB, 0x5E, 0xD2, 0xF8, 0xE8, 0xE9, 0x9B }};

	ip6_address_t peer_ip6 = {{   0xfe, 0x80, 0, 0, 0, 0, 0, 0, 0xff, 0x5d, 0x31, 0xd1, 0x55, 0x22, 0xe0, 0x49 }};
	// 2001:4c4c:11ec:16f1:e072:c321:2756:1f06
	//ip6_address_t peer_ip6 = {{   0x20, 0x01, 0x4c, 0x4c, 0x11, 0xec, 0x16, 0xf1, 0xe0, 0x72, 0xc3, 0x21, 0x27, 0x56, 0x1f, 0x06 }};
	s = socket(AF_INET6, SOCK_STREAM, 0);
#else
	s = socket(AF_INET, SOCK_STREAM, 0);
#endif


	if (0 > s)
	{
		PRINTF("ERROR: s=%d\r\n", s);
	}

#if defined(NETWORK_IPV6_SUPPORTED)
	address.sin_family = AF_INET6;
	address.sin6_addr.s6_addr = in6addr_any;
#else
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
#endif
	// address.sin_port = htons(23456);
	address.sin_port = INPORT_ANY;
	res = socket_bind(s, (struct sockaddr *)&address, sizeof(address));

	if (0 > res)
	{
		PRINTF("ERROR: socket_bind() returned=%d\r\n", res);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(peer_ip);
	address.sin_port = htons(55667);
#if defined(NETWORK_IPV6_SUPPORTED)
	if (6 == sel)
	{
		address.sin_family = AF_INET6;
		address.sin6_addr.s6_addr = peer_ip6;
	}
#endif

	res = socket_connect(s, (struct sockaddr *)&address, sizeof(address), 0);

	if (0 > res)
	{
		PRINTF("ERROR: socket_connect() returned=%d\r\n", res);
	}


	// res = socket_read_nonblocking(s, sock_test_buffer, sizeof(sock_test_buffer));
	res = socket_read_blocking(s, sock_test_buffer, sizeof(sock_test_buffer));
	PRINTF("read_socket() has returned = %d\r\n", res);

	if (0 < res)
	{
		for (i = 0; i < res; i++)
		{
			PUTC(sock_test_buffer[i]);
		}
	}

	str = "Hello Everyone!\r\n";
	res = socket_write_nonblocking(s, str, strlen(str));
	PRINTF("socket_write() has returned = %d\r\n", res);
	send_some_stuff(s);
	res = socket_close(s);
	PRINTF("socket_close() has returned = %d\r\n", res);
}

void sock_test_h(ip4_address_t peer_ip)
{
	struct sockaddr_in address;
	int res, i;
	char *str;
	int s = socket(AF_INET, SOCK_STREAM, 0);
	if (0 > s)
	{
		PRINTF("%s(): ERROR: s=%d\r\n", __FUNCTION__, s);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(45678);

	res = socket_bind(s, (struct sockaddr *)&address, sizeof(address));

	if (0 > res)
	{
		PRINTF("%s(): ERROR: socket_bind() returned=%d\r\n", __FUNCTION__, res);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(peer_ip);
	address.sin_port = htons(80);


	res = socket_connect(s, (struct sockaddr *)&address, sizeof(address), 0);

	if (0 > res)
	{
		PRINTF("%s(): ERROR: socket_connect() returned=%d\r\n", __FUNCTION__, res);
	}

	str = "GET / HTTP/1.1\r\nHOST: jaeos.com\r\n\r\n";
	res = socket_write_nonblocking(s, str, strlen(str));
	PRINTF("%s(): socket_write() has returned = %d\r\n", __FUNCTION__, res);

	do {
		res = socket_read_nonblocking(s, sock_test_buffer, sizeof(sock_test_buffer));

		if (API_EAGAIN == res)
		{
			OS_SYS_MS_SLEEP(10);
		}
		else if (0 > res)
		{
			PRINTF("%s(): read_socket() has returned = %d\r\n", __FUNCTION__, res);
			break;
		}

		if (0 < res)
		{
			for (i = 0; i < res; i++)
			{
				if ('\n' == sock_test_buffer[i])
				{
					PUTC('\r');
				}
				PUTC(sock_test_buffer[i]);
			}
		}
	} while(0 != res);

	res = socket_close(s);
	PRINTF("%s(): socket_close() has returned = %d\r\n", __FUNCTION__, res);
}

void sock_test_http(const char *server)
{
	int res, i;
	char *str;
	int s;

	s =  tcp_connect_to_server_by_name(server, strlen(server), INPORT_ANY, 80);
	if (0 > s)
	{
		PRINTF("%s(%s) failed to connect to server. Result = %d\r\n", __FUNCTION__, server, s);
		return;
	}

	str = "GET / HTTP/1.1\r\nHOST: jaeos.com\r\n\r\n";
	res = socket_write_nonblocking(s, str, strlen(str));
	PRINTF("socket_write() has returned = %d\r\n", res);

	do {
		res = socket_read_nonblocking(s, sock_test_buffer, sizeof(sock_test_buffer));

		if (API_EAGAIN == res)
		{
			OS_SYS_MS_SLEEP(10);
		}
		else if (0 > res)
		{
			PRINTF("read_socket() has returned = %d\r\n", res);
			break;
		}

		if (0 < res)
		{
			for (i = 0; i < res; i++)
			{
				if ('\n' == sock_test_buffer[i])
				{
					PUTC('\r');
				}
				PUTC(sock_test_buffer[i]);
			}
		}
	} while(0 != res);

	res = socket_close(s);
	PRINTF("socket_close() has returned = %d\r\n", res);
}

#if defined(NETWORK_IPV4_SUPPORTED)
extern void print_ip4_address(ip4_address_t ipaddr, const char *name);
#else
#define print_ip4_address(ipaddr, cname) (void)0
#endif

#if defined(NETWORK_IPV6_SUPPORTED)
#if defined(NETWORK_ETHERNET_SUPPORTED)
extern network_interface_t Ethernet0;
#endif
extern void print_ip6_address(const ip6_address_t *ipaddr, const char *name);

#if defined(NETWORK_ETHERNET_SUPPORTED)
static int wait_for_slaac(void)
{
#if !defined(IPV6_LINK_LOCAL_ONLY)
	while (!Ethernet0.net_ipv6_gateway.ipv6_slaac_address_ready)
	{
		OS_SYS_MS_SLEEP(2);
	}
#endif
	return 0;
}

void sock_test_h6(void)
{
	struct sockaddr_in address = { 0 };
	int res, i;
	char *str;

	// 2a00:1288:110:c305::1:8000
	//ip6_address_t peer_ip6 = {{   0x2a, 0x00, 0x12, 0x88, 0x01, 0x10, 0xc3, 0x05, 0, 0, 0, 0, 0x00, 0x01, 0x80, 0x00 }}; // Yahoo

	// 2600:1f18:25f5:c200:fc55:9155:1fae:bbc7
	ip6_address_t peer_ip6 = {{   0x26, 0x00, 0x1f, 0x18, 0x25, 0xf5, 0xc2, 0x00, 0xfc, 0x55, 0x91, 0x55, 0x1f, 0xae, 0xbb, 0xc7 }};

	wait_for_slaac();

	PUTS("\r\nIPv6 HTTP Client Test.\r\n");

	int s = socket(AF_INET6, SOCK_STREAM, 0);

	if (0 > s)
	{
		PRINTF("ERROR: s=%d\r\n", s);
	}

	address.sin_family = AF_INET6;
	address.sin6_addr.s6_addr = in6addr_any;
	//address.sin6_addr.s6_addr = Ethernet0.net_ipv6_gateway.my_ipv6_slaac_address;
	address.sin_port = INPORT_ANY;

	res = socket_bind(s, (struct sockaddr *)&address, sizeof(address));

	if (0 > res)
	{
		PRINTF("ERROR: socket_bind() returned=%d\r\n", res);
	}

	address.sin_family = AF_INET6;
	address.sin6_addr.s6_addr = peer_ip6;
	address.sin_port = htons(80);

	print_ip6_address(&peer_ip6, "HTTP Server ");

	res = socket_connect(s, (struct sockaddr *)&address, sizeof(address), 0);

	if (0 > res)
	{
		PRINTF("ERROR: socket_connect() returned=%d\r\n", res);
	}

	str = "GET / HTTP/1.1\r\nHOST: jaeos.com\r\nConnection: close\r\n\r\n";
	res = socket_write_nonblocking(s, str, strlen(str));
	PRINTF("socket_write() has returned = %d\r\n", res);

	do {
		res = socket_read_nonblocking(s, sock_test_buffer, sizeof(sock_test_buffer));

		if (API_EAGAIN == res)
		{
			OS_SYS_MS_SLEEP(10);
		}
		else if (0 > res)
		{
			PRINTF("read_socket() has returned = %d\r\n", res);
			break;
		}

		if (0 < res)
		{
			for (i = 0; i < res; i++)
			{
				if ('\n' == sock_test_buffer[i])
				{
					PUTC('\r');
				}
				PUTC(sock_test_buffer[i]);
			}
		}
	} while(0 != res);

	res = socket_close(s);
	PRINTF("socket_close() has returned = %d\r\n", res);
}

extern int icmpv6_send_echo_request(network_interface_t *net, const ip_address_t *peer);

void ping_test(void)
{
	int i;
	//ip6_address_t peer_ip6 = {{   0xfe, 0x80, 0, 0, 0, 0, 0, 0, 0xff, 0x5d, 0x31, 0xd1, 0x55, 0x22, 0xe0, 0x49 }};
	//2001:4860:4860::8888
	ip6_address_t peer_ip6 = {{   0x20, 0x01, 0x48, 0x60, 0x48, 0x60, 0, 0, 0, 0, 0, 0, 0, 0, 0x88, 0x88 }};
	// 2001:4c4c:11ec:16f1:591d:4aa4:3971:2181
	//ip6_address_t peer_ip6 = {{   0x20, 0x01, 0x4c, 0x4c, 0x11, 0xec, 0x16, 0xf1, 0x59, 0x1d, 0x4a, 0xa4, 0x39, 0x71, 0x21, 0x81 }};
	// 2001:4C48:2::1
	//ip6_address_t peer_ip6 = {{   0x20, 0x01, 0x4c, 0x48, 0, 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x01 }};

	wait_for_slaac();

	//ip6_address_t peer_ip6 = Ethernet0.net_ipv6_gateway.router_ip_address;

	for (i = 0; i < 32; i++)
	{
		print_ip6_address(&peer_ip6, "Pinging");
		icmpv6_send_echo_request(&Ethernet0, &peer_ip6);
		OS_SYS_MS_SLEEP(1000);
	}
}
#endif // NETWORK_ETHERNET_SUPPORTED
#endif

#endif

#if defined(NETWORK_UDP_SOCKET_INTERFACE_SUPPORTED)
void socket_test_udp(void)
{
	char *hello = "Hello UDP!\r\n";
	struct sockaddr_in address;
	socklen_t addr_len;
	int res;
	//ip4_address_t peer_ip = IP4_ADDRESS(192, 168, 2, 28);
	//char *str;
	PRINTF("Socket Test UDP\r\n");
	int s = socket(AF_INET, SOCK_DGRAM, 0);
	PRINTF("socket() returned %d\r\n", s);
	if (0 > s)
	{
		PRINTF("ERROR: s=%d\r\n", s);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(23456);

	res = socket_bind(s, (struct sockaddr *)&address, sizeof(address));

	PRINTF("socket_bind() returned=%d\r\n", res);

	if (0 > res)
	{
		return;
	}

	while(1)
	{
		res = socket_recvfrom(s, sock_test_buffer, sizeof(sock_test_buffer), 0, (struct sockaddr *)&address, &addr_len);
		PRINTF("socket_recvfrom() returned=%u\r\n", res);
		if (0 < res)
		{
			print_ip4_address(ntohl(address.sin_addr.s_addr), "Incoming");
			PRINTF("Peer port = %u\r\n", ntohs(address.sin_port));
			HEXDUMP(sock_test_buffer, res);
		}

		res = socket_sendto(s, hello, strlen(hello), 0, (struct sockaddr *)&address, addr_len);
		PRINTF("socket_sendto() returned=%u\r\n", res);
	}
#if 0
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(peer_ip);
	address.sin_port = htons(55667);


	res = socket_connect(s, (struct sockaddr *)&address, sizeof(address));
#endif
}
#endif

void sock_test(void *p)
{
	int choice = 0;
	uint32_t tmp = 0;

	switch(choice)
	{
#if defined(NETWORK_TCP_SUPPORTED)
	case 0:
#if defined(CONFIG_INCLUDE_FORTH_TELNET)
		forth_telnet_server(p);
#endif
		break;
	case 1:
		sock_test_listen();
		break;
	case 2:
		sock_test2(4);
		break;
	case 4:
		//sock_test_h(IP4_ADDRESS(99, 79, 17, 226));
		sock_test_h(IP4_ADDRESS(44, 205, 211, 212));
		break;
#endif
#if defined(NETWORK_UDP_SOCKET_INTERFACE_SUPPORTED)
	case 5:
		socket_test_udp();
		break;
#endif
#if defined(NETWORK_IPV6_SUPPORTED) && defined(NETWORK_TCP_SUPPORTED) && defined(NETWORK_ETHERNET_SUPPORTED)
	case 6:
		wait_for_slaac();
		sock_test2(6);
		break;
#endif
	case 8:
		wait_for_slaac();
		//tmp = NETWORK_VERBOSE_TCP_INPUT | NETWORK_VERBOSE_TCP_TRAFFIC | NETWORK_VERBOSE_TCP_INTERACTION | NETWORK_VERBOSE_IP4|NETWORK_VERBOSE_ROUTING|NETWORK_VERBOSE_ETHERNET;
		//network_verbose |= tmp;
		//sock_test_h(IP4_ADDRESS(44, 205, 211, 212));
		//sock_test_h(IP4_ADDRESS(192, 168, 0, 13));
		sock_test_h(IP4_ADDRESS(127, 0, 0, 1));
		//network_verbose &= ~tmp;
		break;

#if defined(NETWORK_IPV6_SUPPORTED) && defined(NETWORK_TCP_SUPPORTED)  && defined(NETWORK_ETHERNET_SUPPORTED)
	case 9:
		sock_test_h6();
		break;

	case 10:
		ping_test();
		break;

	case 11:
		//wait_for_slaac();
		//sock_test_http("44.205.211.212");
		//sock_test_http("2600:1f18:25f5:c200:fc55:9155:1fae:bbc7");
		//sock_test_http("[2600:1f18:25f5:c200:fc55:9155:1fae:bbc7]");
		sock_test_http("jaeos.com");
		break;
#endif
	default:
		PUTS("\r\nsocket_test.c:sock_test() No test was selected.\r\n");
		break;
	}
	(void)tmp;
}
