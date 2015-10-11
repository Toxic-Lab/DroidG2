<table border=0 cellpadding=0 cellspacing=0 ><tr><td width = 20></td><td>
<table border=0 cellpadding=0 cellspacing=0 >
#fancy light
#foreach
	<tr bgcolor=$FANCY>
		<td align = left> $address
			#if $pushed
				(<b>p</b>)
			#endif
		</td>
		
		<td width = 20></td>
		<td align = left>
			#if $task = connecting
				connecting
			#endif
	
			#if $task = closing
				closing connection
			#endif

			#if $task = tiger
				fetching tiger tree
			#endif

			#if $task = file
				<b>R $left  -- $right</b>
			#endif

			#if $task = queued
				Q $qnum  of $qsize
			#endif
		</td>
		
		<td width = 20></td>
		
		<td>
			#if $rate != 0 B
				$rate ps
			#endif
		</td>
	</tr>
#endfor
</table>

</td></tr></table>
