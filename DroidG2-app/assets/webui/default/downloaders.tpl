<table border=0 cellpadding=0 cellspacing=0 ><tr><td width = 20></td><td>
<table border=0 cellpadding=0 cellspacing=0 >
#foreach address status pushed rate qnum qsize left right length total
	#include fancydark.tpl
	<tr bgcolor=$color>
		<td align = left> $address
			#if $pushed
				(<b>p</b>)
			#endif
		</td>
		
		<td width = 20></td>
		<td align = left>
			#if $status = init
				initializing
			#endif
	
			#if $status = idle
				thinking
			#endif

			#if $status = connect
				connecting
			#endif
			
			#if $status = reply
				talking to host
			#endif
			
			#if $status = tiger
				fetching tiger tree
			#endif

			#if $status = file
				<b>R $left  -- $right  ($length)</b> 
			#endif

			#if $status = queued
				Q $qnum  of $qsize
			#endif
		</td>
		
		<td width = 20></td>
		
		<td>
			#if $rate != 0 B
				$rate ps
			#endif
		</td>
		
		<td width = 20></td>
		
		<td>
			#if $total != 0 B
				Total in: $total
			#endif
		</td>
	</tr>
#next
</table>

</td></tr></table>
