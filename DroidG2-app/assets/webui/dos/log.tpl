<h2 align = center>Log</h2>
<div align = center>Verbose levels: 
<font color = yellow>basic</font>, <font color = blue>advanced</font>, <font color = white>developer</font>
</div>
<br>
<table align = center border=0 cellpadding=1 cellspacing=0 >
#foreach level line
	<tr> <td>

		#if $level = 1	
			<font color = yellow>
		#endif

		#if $level = 2	
			<font color = blue>
		#endif

		#if $level = 3	
			<font color = white>
		#endif
		
		$line
		</font>
	</td></tr>
#next
</table>
