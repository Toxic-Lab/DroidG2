<h2 align=center>Search results</h2>
<div align = center>Name: <b>$dn</b>
#if $min
	&nbsp;&nbsp;&nbsp;Min: <b>$min</b>
#endif
#if $max
	&nbsp;&nbsp;&nbsp;Max: <b>$max</b>
#endif
	<br>
#if $extensions
	File types: <b>$extensions</b>
#endif
<br>	<br>
	<div align = center><a href = /srch?show=$id>refresh</a></div>
	<br>
#foreach pagen
	#if $pagen = $page_cur
		<b>$pagen</b>&ensp;
	#else 
		<a href = /srch?show=$id&page=$pagen>$pagen</a>&ensp;
	#endif 

#next
	<br>
<table align = center border=0 cellpadding=3 cellspacing=0 >
	<tr>
		<td></td>
		<td><b><a href=/srch?show=$id&sort=sres&sortby=name>Name</a></b></td>
		<td width=10></td>
		
		<td align=right><b><a href=/srch?show=$id&sort=sres&sortby=size>Size</a></b></td>
		<td width=10></td>
		
		<td align=right><b><a href=/srch?show=$id&sort=sres&sortby=sources>Sources</a></b></td>
	</tr>
#foreach type name size sources
	#include fancydark.tpl
	<tr bgcolor = "$color">
		<td><img src="/img/ext-$type.png"></td>
		<td>$name</td>
		<td width=10></td>
		
		<td align=right>$size</td>
		<td width=10></td>
		
		<td align=right>$sources</td>
	</tr>
#next
</table>
<br><br>
#foreach pagen
	#if $pagen = $page_cur
		<b>$pagen</b>&ensp;
	#else 
		<a href = /srch?show=$id&page=$pagen>$pagen</a>&ensp;
	#endif 

#next
<br><br>
