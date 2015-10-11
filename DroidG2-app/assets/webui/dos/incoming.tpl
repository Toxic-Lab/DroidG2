
<h2>Incoming files ($total_files)<br>===============</h2>
#fancy dark
<div style = "margin-left:15;">
<table align = center border = 0 cellpadding = 3 cellspacing = 0>
#foreach type name size
	#if $type != tmp
		<tr>
			<td valign = middle>[<a href = "/incoming?del=$name" onclick="return confirm('The file will be deleted from the disk. Continue?')">X</a>]</td>
			<td width = 2></td>
		
			<td align = left valign = middle><a href = "/fetch/$name">$name  ($size)</a></td>
		</tr>
	#endif
#next
</div>
</table>

