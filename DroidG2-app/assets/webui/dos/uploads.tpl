<table cellspacing = 0 cellpadding = 0 border = 0><tr><td>
<h2>Uploads ($total_uploads)<br>==============</h2>
#if $total_uploads
	<div style = "margin-left:15;">

	<h3>Active<br>======</h3>
	<table width = 100% border=0 cellpadding=3 cellspacing=0>
	<tr>
		<td><b>Name</b></td>
		<td></td>
		<td align = right><b>Rate</b></td>
		<td></td>
		<td align = right><b>Sent</b></td>
		<td></td>
		<td align = left><b>To</b></td>
	</tr>
	
	#foreach a_type a_name a_nick a_address a_rate a_size a_client
		<tr>
			<td align = left>$a_name</td>
			<td width = 10></td>
	
			<td align = right>$a_rate KBps</td>
			<td width = 10></td>

			<td align = right>$a_size</td>
			<td width = 10></td>
			<td align = left><b>$a_nick</b> (
				#if $a_client
					$a_client,
				#endif
				$a_address 
				)</td>
		</tr>
	#next
	</table>

	<h3>Queued<br>======</h3>
	<table width=100% border=0 cellpadding=3 cellspacing=0 >

	#foreach q_type q_name q_nick q_address q_client
		<tr bgcolor = "$FANCY">
			<td></td>
			<td align = left>$q_name</td>
			<td width = 10></td>
			<td align = left><b>$q_nick</b>(
				#if $q_client
					$q_client,
				#endif
				$q_address 
				)</td>
		</tr>
	#next
	</table>
	</div>
	<br>
#endif

<h2>History ($total_history) (<a href = "?purge">X</a>)<br>================</h2>
#if $total_history
	<div style = "margin-left:15;">
	Total sent: <b>$total_size</b><br><br>
	<table border=0 cellpadding=3 cellspacing=0 >
	<tr>
		<td><b>Name</b></td>
		<td></td>
		<td align = right><b>Uploaded</b></td>
		<td></td>
		<td align = left><b>To</b></td>
	</tr>
	
	#foreach h_type h_name h_nick h_address h_size h_client
		<tr>
			<td align = left>$h_name</td>
			<td width = 10></td>

			<td align = right>$h_size</td>
			<td width = 10></td>

			<td align = left><b>$h_nick</b> (
				#if $h_client
					$h_client,
				#endif
				$h_address 
				)</td>
		</tr>
	#next
	</table>
	</div>
#endif
</td></tr></table>
<br>
