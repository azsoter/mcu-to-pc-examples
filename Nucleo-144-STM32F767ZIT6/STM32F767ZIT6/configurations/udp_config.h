/*
* udp_config.h
*
*  Created on: Dec 7, 2020
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

#ifndef UDP_CONFIG_H_
#define UDP_CONFIG_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <network.h>

// The number of UDP (sockets) that exist at any one time.
// Since everything is static (the implementation does not use malloc() this has to be determined at compile time.
#define MAX_NUMBER_OF_UDP_SOCKETS 2

// UDP sockets are numbered after TCP sockets.
// This is an offset to map UDP socket numbers to indexes inside the UDP socket array.
// If TCP support is enabled this should be larger than MAX_NUMBER_OF_TCP_CONNECTIONS.
// But it is good to just pick something larger so it looks obvious which socket is for UDP.
#define UDP_SOCKET_NUMBER_OFFSET 16


// We only support packets that fit into one ethernet frame.
#define UDP_MAX_PACKET_SIZE (1500 - (sizeof(ip4_header_t) + sizeof(udp_header_t)))


#ifdef __cplusplus
}
#endif

#endif /* UDP_CONFIG_H_ */
