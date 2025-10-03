/*
* network_interfaces.h
*
*  Created on: Nov 6, 2023
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

#ifndef NETWORK_INTERFACES_H_
#define NETWORK_INTERFACES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <network.h>
#include <ppp.h>

#if defined(NETWORK_PPP_SUPPORTED)
extern ppp_state_t PPP_State;
extern network_interface_t PPP0Interface;
#endif

#if defined(NETWORK_ETHERNET_SUPPORTED)
extern network_interface_t Ethernet0;
#endif

#ifdef __cplusplus
}
#endif

#endif /* NETWORK_INTERFACES_H_ */
