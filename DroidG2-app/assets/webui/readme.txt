Sharelin styles overview

Each style is a set of template(tpl) files placed in webui/style_name_folder. 
Some file names (like downloads.tpl, searches.tpl, etc.) are basic and 
sharelin opens them directly on request, however additional template files can be 
created and included (#include) within basic tpl files. Style is set in webui settings by folder name.

Here how are templates work: Sharelin opens template file and associates several 
arrays (or variables which are single-element arrays), when all data is set, 
template code is parsed and variables are substituted. Template can ommit variables. 
Undefined variables equals "0" by default. 

The arrays (list of searches for instance) can't be indexed directly, however several arrays can 
be listed as a single set using #foreach/#next statement. In that case the block of text within 
#foreach/#next tags will be looped with index of all arrays listed in #foreach incrementing simultaniously by 1. 
#foreach can't be inclusive (no #foreach in #foreach block). The variables that are not listed in #foreach as 
iterators, are equals to their first values. See the following example
Let us have arrays A(0, 1, 2), B (3, 4), C(5, 6, 7, 8), D(9)
#foreach A C
	$A $B $C $D
#next
will give:
0 3 5 9
1 3 6 9
2 3 7 9

You can define variable by using #def derictive:
#def color #fafafa

There is also primitime if-else statement of the form:
#if $variable @operator value OR #if $variable
	block 1
#else
	block 2
#endif

#else block is optional
@operator can be either "=" or "!="
form #if $variable equals: #if $variable != 0

Example of making fancy colors output:
#foreach A
	#if $color = #fafafa
 		#def color #dfdfdf
	#else
		#def color #fafafa
	#endif
	<table><tr bgcolor = $color><td>$A</td></tr></table>
#next

To use fancy colors, see default template, there are 2 templates: fancydark.tpl and fancylight.tpl,
you can modify them and include to your templates.

To include another template file, use "#include"

PATH SETTINGS: all file names except the ones in #include, MUST contain full path from style root. For example: <img src="/img/pic.png">.

See default template for the examples.

The easiest way to make a new style is to copy default style and change tpl files.

