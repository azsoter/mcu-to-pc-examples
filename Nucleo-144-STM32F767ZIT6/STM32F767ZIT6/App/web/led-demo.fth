httpd
" LED Control Over HTTP Demo" <html-doc-with-title>

<site-sidemenu>

http-method@ http-method-post =
[IF]
	" led1" http-get-form-value 0= [IF] 1 1 led! [ELSE] 0 1 led! [THEN] 2DROP
	" led2" http-get-form-value 0= [IF] 1 2 led! [ELSE] 0 2 led! [THEN] 2DROP
	" led3" http-get-form-value 0= [IF] 1 3 led! [ELSE] 0 3 led! [THEN] 2DROP
[THEN]
<default-form>
	.( <label for="led1">LED 1</label><input type="checkbox" id="led1" name="led1" ) 1 led@ [IF] .( checked ) [THEN] .( > ) cr
	.( <label for="led2">LED 2</label><input type="checkbox" id="led2" name="led2" ) 2 led@ [IF] .( checked ) [THEN] .( > ) cr
	.( <label for="led3">LED 3</label><input type="checkbox" id="led3" name="led3" ) 3 led@ [IF] .( checked ) [THEN] .( > ) <br>
 	.( <input type="submit" value="Submit"> ) <br>
 </form>
<p>
1 dup . char : emit led@ . <br>
2 dup . char : emit led@ . <br>
3 dup . char : emit led@ .
</p>
</html-doc>