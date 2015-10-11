#include searchform.tpl
#if $total_searches
<h2>Searches ($total_searches)<br>=============</h2>
	<div style = "margin-left:15;">
<table border=0 cellpadding=3 cellspacing=0 >
	<tr>
		<td></td>
		<td><b>Criteria</b></td>
		<td width=10></td>
		
		<td align=right><b>Results</b></td>
	</tr>
#foreach id active extensions criteria hits
	<tr>
		<td>
			[<a href="/srch?del=$id">X</a>]&nbsp;
			#if $active
				[<a href="/srch?stop=$id">P</a>]
			#else
				[<a href="/srch?resume=$id">R</a>]
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
