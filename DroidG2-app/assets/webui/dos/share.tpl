
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
		<td align = left><b><a href = "/share?sort=sfl&sortby=name">Name</a></b></td>
		<td width=10></td>
		<td align = right><b><a href = "/share?sort=sfl&sortby=requests">Requests</a></b></td>
		<td width=10></td>
		<td align = right><b><a href = "/share?sort=sfl&sortby=hits">Hits</a></b></td>
	</tr>
#foreach type name path size requests hits
	<tr>
		<td align = left valign = middle>$name&nbsp;&nbsp;&nbsp;(
#if $size = 0 B
	not scanned yet
#else
	<b>$size</b>
#endif
	)
</td>
		<td width=10></td>
		<td align = right>$requests</td>
		<td width=10></td>
		<td align = right>$hits</td>
	</tr>
#next
</table>
<br>
<div align = center>
#foreach pagen
	#if $pagen = $page_cur
		<b>$pagen</b>&ensp;
	#else 
		<a href = /share?page=$pagen>$pagen</a>&ensp;
	#endif 

#next
</div>

