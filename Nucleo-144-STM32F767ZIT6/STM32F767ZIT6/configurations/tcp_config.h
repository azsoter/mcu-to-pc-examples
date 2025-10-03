/*
* tcp_config.h
*
*  Created on: Nov 24, 2020
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

#ifndef TCP_CONFIG_H_
#define TCP_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <network_config.h>

#if defined(NETWORK_IPV6_SUPPORTED)
#define TCP_MIN_MSS ((NETWORK_MIN_MTU) - 60)
#else
#define TCP_MIN_MSS ((NETWORK_MIN_MTU) - 40)
#endif


// The socket number of the first TCP socket.
#define TCP_SOCKET_NUMBER_OFFSET 4

// The number of TCP connections (sockets) that exist at any one time.
// Since everything is static (the implementation does not use malloc() this has to be determined at compile time.
#define MAX_NUMBER_OF_TCP_CONNECTIONS 8


#if 0
#define TCP_USE_MANAGED_BUFFERS
#else
#define TCP_USE_CIRCULAR_BUFFERS
#endif

#if defined(TCP_USE_MANAGED_BUFFERS)
// The number of input and output buffers per connection.
// These are requisitioned from the managed buffer pool when the socket is connected (incoming or outgoing connection)
// thus they are somewhat more dynamically allocated.
// This avoids having buffers allocated to e.g. a socket that just listens, since it is not sending and receiving data.
#define NUMBER_OF_TCP_INPUT_BUFFERS  2
#define NUMBER_OF_TCP_OUTPUT_BUFFERS 2
#endif

#if defined(TCP_USE_CIRCULAR_BUFFERS)
#define NUMBER_OF_TCP_CIRCULAR_BUFFERS 10 /* Needs 2 per MAX_NUMBER_OF_TCP_CONNECTIONS except for those that only listen. */
//#define TCP_CIRCULAR_BUFFER_SIZE 2048
//#define TCP_CIRCULAR_BUFFER_SIZE 0x0C00 // 3K

#define TCP_CIRCULAR_BUFFER_SIZE (2 * (NETWORK_MAX_MTU))

#endif

// Timers and timeouts.

// Min, max and default values for the TCP retransmission time out.
#define TCP_MIN_RTO_MS      200
//#define TCP_MIN_RTO_MS      100
#define TCP_MAX_RTO_MS   120000
#define TCP_DEFAULT_RTO_MS 3000

// If we cannot connect within this time we simply reset the connection.
#define TCP_CONNECTION_ESTABLISH_MS 75000

// Maximum Segment Lifetime, used for time wait timeout. This is somewhat arbitrary.
#define TCP_MSL 30000 // 30 seconds, for now.

// Allow the use of the TCP time stamp option to measure RTT.
#define TCP_ALLOW_TIMESTAMPS

// Keepalive
#define TCP_KEEPALIVE
//#define TCP_KEEPALIVE_DEFAULT_IDLE 600
//#define TCP_KEEPALIVE_DEFAULT_IDLE 7200
#define TCP_KEEPALIVE_DEFAULT_IDLE 120 // Seconds
#define TCP_KEEPALIVE_DEFAULT_INTERVAL 75 // Seconds
#define TCP_KEEPALIVE_DEFAULT_RETRIES 9


#ifdef __cplusplus
}
#endif

#endif /* TCP_CONFIG_H_ */
