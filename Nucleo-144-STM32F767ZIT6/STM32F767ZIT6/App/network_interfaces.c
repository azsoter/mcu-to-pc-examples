/*
* network_interfaces.c
*
*  Created on: Oct 26, 2023
*      Author: Andras Zsoter
*
*      Copyright (c) Andras Zsoter 2023-2025.
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

#include <network_config.h>
#include <net_interface.h>
#include <network.h>
#include <network_interfaces.h>
#include <ethernet.h>

#if defined(NETWORK_PPP_SUPPORTED)
ppp_state_t PPP_State;
network_interface_t PPP0Interface;
#endif

network_interface_t *NetworkInterfaces[] =
{
#if defined(NETWORK_ETHERNET_SUPPORTED)
		&Ethernet0,			// Our primary (preferred) interface.
#endif

#if defined(NETWORK_PPP_SUPPORTED)
		&PPP0Interface,		// PPP (over serial) connection.
#endif
		&LoopBackInterface,	// Loop back is last.
		0
};
