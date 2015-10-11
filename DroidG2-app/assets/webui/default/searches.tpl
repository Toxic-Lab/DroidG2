#include searchform.tpl
#if $total_searches
<div align = center>
<h2 align=center>Searches ($total_searches)</h2>
<table border=0 cellpadding=3 cellspacing=0 >
	<tr>
		<td></td>
		<td><b>Criteria</b></td>
		<td width=10></td>
		
		<td align=right><b>Results</b></td>
	</tr>

#foreach id active extensions criteria hits
	#include fancydark.tpl
	<tr bgcolor = "$color">
		<td>
			<a href="/srch?del=$id"><img border = 0 title="Remove search" src=img/delete16.png></a>
			&nbsp;&nbsp;
			#if $active
				<a href="/srch?stop=$id"><img border = 0 title="Stop search" src=img/stop16.png></a>
			#else
				<a href="/srch?resume=$id"><img border = 0 title="Resume search" src=img/resume16.png></a>
			#endif
		</td>
			<td><a href="/srch?show=$id">$criteria</a>
			#if $extensions 
				<br><b>$extensions</b>
			#endif
			</td>
		<td width=10></td>
		
		<td align=right>$hits</td>

	</tr>
#next
</table>
</div>
#endif
