httpd
S" Form Submission Data" <html-doc-with-title>
<site-sidemenu>
http-method@ http-method-post =
[IF]

	<p> " test"      http-get-form-value cr throw <p> .( test = ) type </p>
	<p> " something" http-get-form-value cr throw <p> .( something = ) type </p>
[ELSE]
	<p> .( This page is only supposed to be used for form submission ) </p>
[THEN]
</html-doc>