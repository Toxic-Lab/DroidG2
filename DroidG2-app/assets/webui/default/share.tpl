
<h2 align = center>Local share ($total_files  files)</h2>
<div align = center>
#foreach pagen
	#if $pagen = $page_cur
		<b>$pagen</b>&ensp;
	#else 
		<a href = /share?page=$pagen>$pagen</a>&ensp;
	#endif 

#next
</div>
<br>
<table align = center border = 0 cellpadding = 3 cellspacing = 0>
	<tr>
		<td></td>
		<td align = left><b><a href = "/share?sort=sfl&sortby=name">Name</a></b></td>
		<td width=10></td>
		<td align = right><b><a href = "/share?sort=sfl&sortby=size">Size</a></b></td>
		<td width=10></td>
		<td align = right><b><a href = "/share?sort=sfl&sortby=requests">Requests</a></b></td>
		<td width=10></td>
		<td align = right><b><a href = "/share?sort=sfl&sortby=hits">Hits</a></b></td>
	</tr>
#foreach type name path size requests hits
	#include fancydark.tpl
	<tr bgcolor = $color>
		<td valign = middle><img src="/img/ext-$type.png"></td>
		<td align = left valign = middle>$name
		<br>
	<div style="margin-left:15; font-size:11px;">
		$path
		</div>
</td>
		<td width=10></td>
		<td align = right>
#if $size = 0 B
	not scanned yet
#else
<nobr>	$size</nobr>
#endif
		</td>
		<td width=10></td>
		<td align = right>$requests</td>
		<td width=10></td>
		<td align = right>$hits</td>
	</tr>
#next
</table>
<br> <br>
<div align = center>
#foreach pagen
	#if $pagen = $page_cur
		<b>$pagen</b>&ensp;
	#else 
		<a href = /share?page=$pagen>$pagen</a>&ensp;
	#endif 

#next
</div>

