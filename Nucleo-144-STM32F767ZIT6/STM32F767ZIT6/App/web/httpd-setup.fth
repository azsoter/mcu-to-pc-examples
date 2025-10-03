( HTTDP Setup )

: // ( -- ) 0x0d parse drop drop ; immediate

vocabulary httpd-voc
: httpd ( -- ) only forth also httpd-voc ;
httpd definitions

: S' ( compile: <string> -- ) ( -- c-addr count )  [char] ' parse state @ if postpone sliteral then ; immediate

1 constant http-method-get
2 constant http-method-head
3 constant http-method-post

: <html>  ( -- ) ." <html>" cr ;
: </html> ( -- ) ." </html>" cr ;
: <head>  ( -- )  ." <head>" cr ;
: </head> ( -- )  ." </head>" cr ;
: <title> ( -- ) ." <title>" ;
: </title> ( -- ) ." </title>" cr ;
: <body>  ( -- )  ." <body>" cr ;
: </body> ( -- )  ." </body>" cr ;
: <p>     ( -- ) ." <p>" ;
: </p>    ( -- ) ." </p>" cr ;
: <div>   ( -- ) ." <div>" ;
: </div>  ( -- ) ." </div>" cr ;

: <pre>   ( -- ) ." <pre>" cr ;
: </pre>  ( -- ) ." </pre>" cr ;

: <br>    ( -- ) ." <br>" cr ;

: <h1>    ( -- ) ." <h1>" ;
: </h1>   ( -- ) ." </h1>" cr ;
: <h2>    ( -- ) ." <h2>" ;
: </h2>   ( -- ) ." </h2>" cr ;
: <h3>    ( -- ) ." <h3>" ;
: </h3>   ( -- ) ." </h3>" cr ;
: <h4>    ( -- ) ." <h4>" ;
: </h4>   ( -- ) ." </h4>" cr ;

: <i>     ( -- ) ." <i>" ;
: </i>    ( -- ) ." </i>" ;
: <b>     ( -- ) ." <i>" ;
: </b>    ( -- ) ." </b>" ;
: <em>    ( -- ) ." <em>" ;
: </em>   ( -- ) ." </em>" ;

: <default-form>  ( -- ) S' <form method="POST">' type cr ;
: <form   ( -- ) ." <form " [char] > parse type [char] > emit cr ;
: </form> ( -- ) ." </form>" cr ;

: <site-sidemenu> ( -- )
 S' <aside>
	 <h1  style="text-align:center;">Navigation</h1>
	 <ul>
	  	 <li><a href="/">Home</a></li>
	  	 <li><a href="/other_page.html">Other</a></li>
		 <li><a href="/script/led-demo.fth">LED Demo</a></li>
		 <li><a href="/script/sys-info.fth">System Info</a></li>
		 <li><a href="/script/net-info.fth">Network Info</a></li>
		 <li><a href="/script/demo.fth">Script Demo</a></li>
		 <li><a href="https://github.com/azsoter/jaeos-devel" target="_blank" title="azsoter/jaeos-devel � GitHub">JaeOS<sup>&#174;</sup> Sources on GitHub</a></li>
	 </ul>
 </aside>'
 cr type cr
 ;
 
 : <div.clearfix/> ( -- ) S' <div class="clearfix"></div>' type cr ;
 
: <home> ( -- ) S' <a href="/">Home</a>' type cr ;

: .HTML5-Doctype ( -- ) ." <!DOCTYPE html>" cr ;
: .css-link ( -- ) ." <link rel='stylesheet' href='/style.css'>" cr ;
: .favicon ( -- )  ." <link rel='icon' type='image/png' href='/favicon.png'>" cr ;

: <html-doc> ( -- )
  set-http-output-html
  .HTML5-Doctype
  <html>
  <head>
  	.css-link
  	.favicon
  </head>
  <body>
;

: <html-doc-with-title> ( caddr count -- )
  set-http-output-html
  .HTML5-Doctype
  <html>
  	<head>
  		<title> 2dup type </title> 
  		.css-link
  		.favicon
  	</head>
  	<body>
  		<h1> type </h1>
;
: </html-doc> ( -- ) </body> </html> ;
 