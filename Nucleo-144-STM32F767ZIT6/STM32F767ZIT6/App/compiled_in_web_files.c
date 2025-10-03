/*
* compiled_in_web_files.c
*
*  Created on: Jul 27, 2024
*      Author: Andras Zsoter
*
*      Copyright (c) Andras Zsoter 2025.
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
#include <sp_httpd.h>

#if defined(CONFIG_INCLUDE_HTTPD)
extern const web_file_header_t WebContent;
extern const web_file_header_t HttpdInitScript;

// Actual content location of a compiled in file
const char *httpd_web_file_content(const web_file_header_t *f)
{
	return f->name + strlen(f->name) + 1;
}

//
// Find a file in the compiled in Web content.
//
const web_file_header_t *http_find_file(const char *filename)
{
	const web_file_header_t *p;

	if (0 == strcmp("/", filename)) // The first file.
	{
		return &WebContent;
	}

	for (p = &WebContent; 0 != p; p = p->next)
	{
		if (0 == strcmp(p->name, filename))
		{
			return p;
		}
	}

	return 0;
}

const web_file_header_t *http_get_initscript(void)
{
	return &HttpdInitScript;
}
#endif
