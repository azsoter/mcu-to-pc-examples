/*
* dg_buffer.c
*
*  Created on: Nov 17, 2023
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
#include <datagram_proto.h>

#if defined(CONFIG_USE_DATAGRAM_BASED_TRANSPORT)

#include <managed_buffers.h>
#include <packetized_datagram.h>
#include <datagram_proto_handler.h>
#include <datagram_proto_internal.h>
#include <printing.h>

static buffer_header_t big_buffer_header = { 0 };

buffer_header_t *GetDatagramPayloadBuffer(DatagramHandler_t *dgh, uint16_t tag, int32_t length)
{
	// PRINTF("%s(0x%04x,0x%08x)\r\n", __FUNCTION__, tag, length);

	buffer_header_t *mb;

	if (MANAGED_BUFFER_SIZE >= length)
	{
		mb = GetPooledBuffer(OS_SYS_TIMEOUT_NO_WAIT);
#if defined(NETWORK_VERBOSE_ERROR)
			if ((0 == mb) && (0 != (network_verbose & NETWORK_VERBOSE_ERROR)))
			{
				PRINTF("%s() Failed to obtain pooled buffer.\r\n", __FUNCTION__);
			}
#endif
		return mb;
	}

	if ((length <= sizeof(big_test_buffer)) && !big_buffer_header.busy)
	{
		InializeManagedBuffer(&big_buffer_header, big_test_buffer, sizeof(big_test_buffer), MANAGED_BUFFER_TAG_SPECIAL);
		//big_buffer_header.busy = 1;
		mb = &big_buffer_header;
		return mb;
	}

	return 0;
}

int Datagram_DispatchReceivedBuffer(DatagramHandler_t *dgh, uint16_t tag, buffer_header_t *mb)
{
	int res = -1;

	if ((0 != mb) && (0 != dgh->PhDev))
	{
		if (big_test_buffer == mb->buffer)
		{
			mb->busy = 1;
		}

		res = EnqueueBuffer(&(dgh->PhDev->incoming_queue), mb, OS_SYS_TIMEOUT_NO_WAIT);
	}

	// The caller needs to handle failure.
	//If we could not enqueue the buffer we will need to discard it, or try again.
	return res;
}

void Datagram_ReleaseBuffer(buffer_header_t **mb)
{
	int res = 0;

	// PRINTF("%s(%p) *mb=%p, tag = 0x%02x\r\n", __FUNCTION__, mb, *mb, (*mb)->tag);

	if (0 != *mb)
	{
		if (MANAGED_BUFFER_TAG_POOLED == (*mb)->tag)
		{
			res = ReleasePooledBuffer(*mb, OS_SYS_TIMEOUT_FOREVER);
		}
		else
		{
			ResetBuffer(*mb);
		}
	}

	*mb = 0;
	(void)res;
}
#endif // CONFIG_USE_DATAGRAM_BASED_TRANSPORT
