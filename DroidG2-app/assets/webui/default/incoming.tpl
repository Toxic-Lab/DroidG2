
<h2 align = center>Incoming files ($total_files)</h2>
<table align = center border = 0 cellpadding = 3 cellspacing = 0>
#foreach type name size
	#if $type != tmp
		#include fancydark.tpl
		<tr bgcolor = $color>
			<td valign = middle><a href = "/incoming?del=$name" onclick="return confirm('The file will be deleted from the disk. Continue?')">
				<img border = 0 title="Delete file" src="/img/remove-file.png">
			</a></td>
			<td width = 2></td>
		
			<td valign = middle><img src="/img/ext-$type.png"></td>
			<td align = left valign = middle><a href = "/fetch/$name">$name</a><br>&nbsp;&nbsp;&nbsp;<b>$size</b></td>
		</tr>
	#endif
#next
</table>

