httpd
" System Information Page" <html-doc-with-title>

<site-sidemenu>

<h2> .( General ) </h2>
<pre>
.SYS cr
.( Current Time: ) get-system-time print-time cr
</pre>
<p>

</p>

<h2> .( All wordlists) </h2>
<pre>
.wordlists cr
</pre>

<h2> .( IP Addresses) </h2>
<p>
[defined] MY-IPV4-ADDRESS
[IF]
S' .( <a href="http://) MY-IPV4-ADDRESS dup .IPV4-ADDRESS .( /">http://) .IPV4-ADDRESS .( /</a><br>) cr'
' evaluate catch drop
[THEN]

[defined] .IPV6-ADDRESS
[IF]
S' .( <a href="http://[) MY-IPV6-LL-ADDRESS .IPV6-ADDRESS .( ]/">http://[) MY-IPV6-LL-ADDRESS .IPV6-ADDRESS .( ]/</a><br>) cr'
' evaluate catch drop

S' .( <a href="http://[) MY-IPV6-SLAAC-ADDRESS .IPV6-ADDRESS .( ]/">http://[) MY-IPV6-SLAAC-ADDRESS .IPV6-ADDRESS .( ]/</a><br>) cr'
' evaluate catch drop
[THEN]
</p>
<h2> .( Debug Information) </h2>
<pre>
.DEBUG
</pre>
</html-doc>
