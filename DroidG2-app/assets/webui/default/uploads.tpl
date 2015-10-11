<table align = center cellspacing = 0 cellpadding = 0 border = 0><tr><td>
<h2 align = center>Uploads ($total_uploads)</h2>
#if $total_uploads
	<h3 align = center>Active</h3>
	<table width = 100% align = center border=0 cellpadding=3 cellspacing=0 >
	<tr>
		<td></td>
		<td><b>Name</b></td>
		<td></td>
		<td align = right><b>Rate</b></td>
		<td></td>
		<td align = right><b>Sent</b></td>
	</tr>
	
	#foreach a_type a_name a_nick a_address a_rate a_size a_client
		#include fancydark.tpl
		<tr bgcolor = "$color">
			<td><img src="img/ext-$a_type.png"></td>
			<td align = left>
				$a_name<br>
				&nbsp;&nbsp;&nbsp;
				<b>$a_nick</b> (
#if $a_client
						$a_client,
#endif
						$a_address
						)
			</td>
			<td width = 10></td>
	
			<td align = right>$a_rate KBps</td>
			<td width = 10></td>

			<td align = right>$a_size</td>
		</tr>
	#next
	</table>

	<h3 align = center>Queued</h3>
	<table width=100% border=0 cellpadding=3 cellspacing=0 >

	#foreach q_type q_name q_nick q_address q_client
		#include fancydark.tpl
		<tr bgcolor = "$color">
			<td><img src="img/ext-$q_type.png"></td>
			<td align = left>
				$q_name<br>
				&nbsp;&nbsp;&nbsp;
				<b>$q_nick</b>(
#if $q_client
						$q_client,
#endif
						$q_address
						)
			</td>
		</tr>
	#next
	</table>

	<br>
#endif

<h2 align = center>History ($total_history)</h2>
<div align = center><a href = "?purge">clear</a></div>
#if $total_history
	<div align = center>Total sent: <b>$total_size</b></div>
	<table align = center border=0 cellpadding=3 cellspacing=0 >
	<tr>
		<td></td>
		<td><b>Name</b></td>
		<td></td>
		<td align = right><b>Uploaded</b></td>
	</tr>

	#foreach h_type h_name h_nick h_address h_size h_client
		#include fancydark.tpl
		<tr bgcolor = "$color">
			<td><img src="img/ext-$h_type.png"></td>
			<td align = left>
				$h_name<br>
				&nbsp;&nbsp;&nbsp;
				<b>$h_nick</b> (
#if $h_client
						$h_client,
#endif
						$h_address )
			</td>
			<td width = 10></td>

			<td align = right>$h_size</td>
		</tr>
	#next
	</table>
#endif
</td></tr></table>
<br>
