/*
* ppp_auth_secrets.c
*
*  Created on: Oct 29, 2023
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

#include <ppp.h>
#if defined(NETWORK_PPP_SUPPORTED)
#include <stdint.h>
#include <string.h>

// These are obviously skeleton functions for demo purposes.
// In real life you can use similarly hard coded names and secrets, or have some mechanism to retrieve them
// from some persistent storage.

static const char *my_name = "ppptest";
static const char *server_name = "pppserver";

// Return our login name.
// The protocol idenfies PAP or CHAP (or whatever may be implemented later).
// The peer is some sort of identification about what system we are trying to log into,
// can be passed as NULL if names/secrets do not depend on the server.
// Return the name (or NULL if the authentication scheme is not supported).
const char *ppp_auth_my_name(uint16_t protocol, void *peer)
{
	switch(protocol)
	{
		case PPP_PROTOCOL_PAP:
			return my_name;

		case PPP_PROTOCOL_CHAP:
			return my_name;

		default:
			break;
	}
	return 0;
}

// Get the secret matching the name.
// Name is either our login name, or the name of the remote end.
const char *ppp_auth_secret(uint16_t protocol, void *peer, const char *name)
{
	if (0 == name)
	{
		return 0;
	}

	if ((PPP_PROTOCOL_PAP == protocol) || (PPP_PROTOCOL_CHAP == protocol)) // Just use the same secrets for PAP and CHAP for now.
	{
		if (0 == strcmp(name, my_name))
		{
			return "xyzzy"; // Our password.
		} else if (0 == strcmp(name, server_name))
		{
			return "XYZZY"; // Server's password.
		}
	}
	return 0;
}
#endif // defined(NETWORK_PPP_SUPPORTED)
