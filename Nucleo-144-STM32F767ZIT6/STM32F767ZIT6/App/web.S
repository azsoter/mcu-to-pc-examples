/*
*      Copyright (c) Andras Zsoter 2020-2022.
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
.section .rodata
.align 4
.global WebContent
WebContent:
.index_html:
.word .icon
.word .index_html_end - .index_html_start
.asciz "/index.html"
.index_html_start:
.incbin "web/index.html"
.index_html_end:

.align 4
.icon:
.word .favicon_png
.word .icon_end - .icon_start
.asciz "/icon.jpg"
.icon_start:
.incbin "web/icon.jpg"
.icon_end:

.align 4
.favicon_png:
.word .other_page
.word .favicon_end - .favicon_start
.asciz "/favicon.png"
.favicon_start:
.incbin "web/favicon.png"
.favicon_end:

.align 4
.other_page:
.word .style_css
.word .other_page_end - .other_page_start
.asciz "/other_page.html"
.other_page_start:
.incbin "web/other_page.html"
.other_page_end:

.align 4
.style_css:
.word .demo_fth
.word .style_css_end - .style_css_start
.asciz "/style.css"
.style_css_start:
.incbin "web/style.css"
.style_css_end:

.align 4
.demo_fth:
.word .led_demo_fth
.word .demo_fth_end - .demo_fth_start
.asciz "/script/demo.fth"
.demo_fth_start:
.incbin "web/demo.fth"
.demo_fth_end:

.align 4
.led_demo_fth:
.word .sys_info_fth
.word .led_demo_fth_end - .led_demo_fth_start
.asciz "/script/led-demo.fth"
.led_demo_fth_start:
.incbin "web/led-demo.fth"
.led_demo_fth_end:

.align 4
.sys_info_fth:
.word .process_form_fth
.word .sys_info_fth_end - .sys_info_fth_start
.asciz "/script/sys-info.fth"
.sys_info_fth_start:
.incbin "web/sys-info.fth"
.sys_info_fth_end:

.align 4
.process_form_fth:
.word .net_info_fth
.word .process_form_fth_end - .process_form_fth_start
.asciz "/script/process_form.fth"
.process_form_fth_start:
.incbin "web/process_form.fth"
.process_form_fth_end:

.align 4
.net_info_fth:
.word 0
.word .net_info_fth_end - .net_info_fth_start
.asciz "/script/net-info.fth"
.net_info_fth_start:
.incbin "web/net-info.fth"
.net_info_fth_end:

// --------------------------------------
// This file cannot be found by the normal mechanism.
.align 4
.global HttpdInitScript
HttpdInitScript:
.word 0
.word .httpd_init_script_end - .httpd_init_script_start
.asciz "httpd-setup.fth"
.httpd_init_script_start:
.incbin "web/httpd-setup.fth"
.httpd_init_script_end:
