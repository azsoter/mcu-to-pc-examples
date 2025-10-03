/*
* mcu2pc_config.h
*
*  Created on: Oct 17, 2020
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
#ifndef MCU2PC_CONFIG_H_
#define MCU2PC_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define USE_JAEOS

// If this is defined use a separate Ethernet transmitter thread.
//#define CONFIG_ETHERNET_TRANSMITTER_THREAD 1
#define ETHERNET_XMIT_QUEUE_LENGTH 4	/* For now make it as long as the number of actual transmit buffers. */

#define CONFIG_USE_DATAGRAM_BASED_TRANSPORT 1

#if defined(CONFIG_USE_DATAGRAM_BASED_TRANSPORT)

#define DATAGRAM_PROTO_DEFAULT_DATAGRAM_PAYLOAD_SIZE 1400

// We are set up with the assumption that we send payloads at least this big when sending a multi-segment chunk of data.
// This is needed for the compiled in acknowledgement map size since we don't do dynamic allocation.
// So the peer MUST be able to accept data chunks that are at least this big.
#define DATAGRAM_PROTO_MIN_TX_PAYLOAD_SIZE 256

// The largest incoming datagram payload we can possibly handle.
#define DATAGRAM_PROTO_MAX_RX_PAYLOAD_SIZE (1500 - 68) /* 1500 bytes of Ethernet frame minus headers (IPv6, UDP, our DG header. */


#define DEFAULT_DATAGRAM_DELIVERY_TIMEOUT_MS 5
#define DEFAULT_DATAGRAM_DELIVERY_RETRIES 255

#define DATAGRAM_MIN_RTO 1		// 1 ms
#define DATAGRAM_MAX_RTO 30000	// 30000ms (30 sec)
#endif

//#define MANAGED_BUFFER_SIZE 1460
#define MANAGED_BUFFER_SIZE 1536	/* 0x0600 */
#define MANAGED_BUFFERS_COUNT 16
#define MANAGED_BUFFER_ALIGNMENT 32

#if defined ( __GNUC__ )
#define PACKED_STRUCTURE(X) X __attribute__((packed))
#define DATA_ALIGN4(X) X __attribute__ ((aligned (4)))

#define DATA_ALIGN(X, ALIGN) X __attribute__ ((aligned (ALIGN)))

//#define CLZ_FUNCTION(X)   __builtin_clz((X))
//#define CLZL_FUNCTION(X)  __builtin_clzl((X))
//#define CLZLL_FUNCTION(X) __builtin_clzll((X))

#define CTZ_FUNCTION(X)   __builtin_ctz((X))
#define CTZL_FUNCTION(X)  __builtin_ctzl((X))
#define CTZLL_FUNCTION(X) __builtin_ctzll((X))

#endif

# if 0
#define MEMCPY(D, S, L) memcpy((D), (S), (L))
#else
extern void faster_memcpy(unsigned char *d, const unsigned char *s, unsigned long len);
#define MEMCPY(D, S, L) faster_memcpy((D), (S), (L))
#endif


/*
#ifndef INLINE
# if __GNUC__ && !__GNUC_STDC_INLINE__
#  define INLINE extern inline
# else
#  define INLINE inline
# endif
#endif
*/
#if defined ( __GNUC__ )
#define INLINE extern __inline__
#endif

#if !defined(LITTLE_ENDIAN)
#define LITTLE_ENDIAN 1

#define host_to_little_endian16(X) (X)
#define host_to_little_endian32(X) (X)
#define host_to_little_endian64(X) (X)

#define little_endian_to_host16(X) (X)
#define little_endian_to_host32(X) (X)
#define little_endian_to_host64(X) (X)
#endif

#define CONFIG_INCLUDE_HTTPD 1

#define CONFIG_INCLUDE_OUTGOING_PING 1

#define BIG_TEST_BUFFER_SIZE (300 * 1024)
extern unsigned char big_test_buffer[BIG_TEST_BUFFER_SIZE];

#define INCLUDE_LED_DEMO

#define INCLUDE_RTC 1

// Scripting.
#define CONFIG_HAS_CLI 1
#define CONFIG_CLI_FORTH 1
#define FORTH_SCRIPTING_RP_SIZE 32
#define FORTH_SCRIPTING_SP_SIZE 64
#define CONFIG_INCLUDE_FORTH_TELNET 1

#define CONFIG_SUPPORT_STREAMING 1

#define CONFIG_USE_HW_CRC 1

#define CONFIG_DEVICE_CONFIG_IN_FORTH_BLOCKS 1

#ifdef __cplusplus
}
#endif

#endif /* MCU2PC_CONFIG_H_ */
