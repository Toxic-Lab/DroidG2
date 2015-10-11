<h2 align = center>Log</h2>
<div align = center>Verbose levels: 
<font color = #000000>basic</font>, <font color = #FF6600">advanced</font>, <font color = #0000FF>developer</font>
</div>
<br>
<table align = center border=0 cellpadding=1 cellspacing=0 >
#foreach level line
	#include fancydark.tpl
	<tr bgcolor = "$color"> <td>

		#if $level = 0	
			<font color = #000000">
		#endif

		#if $level = 1	
			<font color = #FF6600">
		#endif

		#if $level = 2	
			<font color = #0000FF">
		#endif
		
		$line
		</font>
	</td></tr>
#next
</table>
