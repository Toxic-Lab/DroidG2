<table cellspacing=0 cellpadding = 0 border=0><tr><td>

<h2>Downloads ($total_downloads)<br>=============</h2>
#if $total_downloads
	#if $expanded_downloads=yes
		<div align = center> <a href = "/dl?expand=no">(collapse)</a></div>
	#else
		<div align = center> <a href = "/dl?expand=yes">(expand)</a></div>
	#endif
	<br>
	<div style = "margin-left:15;">
		
	<form method=POST action="/dl">
	<table width=100% border=0 cellpadding=3 cellspacing=0 >
	<tr>
		<td></td>
		<td align = left><b><a href=/dl?sort=adl&sortby=name>Name</a></b></td>

		<td width=10></td>
		<td align=right><b><a href=/dl?sort=adl&sortby=size>Size</a></b></td>
			
		<td width=10></td>
		<td align=right><b><a href=/dl?sort=adl&sortby=done>Done %</a></b></td>
		
		<td width=10></td>
		<td align=center><b><a href=/dl?sort=adl&sortby=rate>Rate</a></b></td>
		
		<td width=10></td>
		<td align=right><b><a href=/dl?sort=adl&sortby=sources>Sources</a></b></td>

		<td width=10></td>
		<td align=center><b>Status</b></td>

		<td width=10></td>
		<td align=center><b><a href=/dl?sort=adl&sortby=priority>Priority</a></b></td>
	</tr>

	#foreach download_id download_name download_size status progress download_rate active_sources priority sources downloaders
		#def color #000000
		<tr bgcolor = "$color">
			<td>
				<input type="checkbox" name="sel$download_id">&nbsp;&nbsp;&nbsp;&nbsp;
				[<a title = "Remove download" href="/dl?del=$download_id">X</a>]&nbsp;
				#if $dl_paused
					[<a title = "Resume download" href="/dl?resume=$download_id">R</a>]
				#else
					[<a title = "Pause download" href="/dl?stop=$download_id">P</a>]
				#endif
			</td>
		
			<td align = left><a href = "/dl?info=$download_id">$download_name</a></td>
			<td width=10></td>

			<td align=right>$download_size</td>
			<td width=10></td>

			<td align=right><b>
			#if $progress
				$progress
			#endif 
			</b></td>
			<td width=10></td>

			<td align=right>
#if $status = transfer
#if $download_rate != 0 B
			$download_rate
#endif
#endif
#if $status = queued
			queued
#endif
#if $status = paused
			paused
#endif
			</td>
			<td width=10></td>

			<td align=right>$active_sources  ($sources)</td>
			<td width=10></td>

			<td align=center>$status</td>
			<td width=10></td>

			<td align=center>
			<span title = "Low">
#if $priority = 0
			<b>L</b>
#else
			<a href="/dl?id=$download_id&priority=0">l</a>
#endif
			</span>
			&nbsp;
			<span title = "Below normal">
#if $priority = 1
			<b>B</b>
#else
			<a href="/dl?id=$download_id&priority=1">b</a>
#endif
			</span>
			&nbsp;
			<span title = "Normal">
#if $priority = 2
			<b>N</b>
#else
			<a href="/dl?id=$download_id&priority=2">n</a>
#endif
			</span>
			&nbsp;
			<span title = "Above normal">
#if $priority = 3
			<b>A</b>
#else
			<a href="/dl?id=$download_id&priority=3">a</a>
#endif
			</span>
			&nbsp;
			<span title = "High">
#if $priority = 4
			<b>H</b>
#else
			<a href="/dl?id=$download_id&priority=4">h</a>
#endif
			</span>
			</td>
			<td width=10></td>
		</tr>

		#if $downloaders
			<tr>
				<td></td>
				<td>$downloaders</td>
			</tr>
		#endif
	#next
	</table>
	<br><br>
	<select name="selected" class="texta" onchange='this.form.submit()'>
		<option value="nothing">Apply to selected</option>
		<option value="pause">Pause</option>
		<option value="resume">Resume</option>
		<option value="remove">Remove</option>
		<option value="priorityl">Priority: LOW</option>
		<option value="priorityb">Priority: BELOW NORMAL</option>
		<option value="priorityn">Priority: NORMAL</option>
		<option value="prioritya">Priority: ABOVE NORMAL</option>
		<option value="priorityh">Priority: HIGH</option>
																						</select>
																																
	</form>
	<br>
	</div>
#endif

<h2>Recently downloaded (<a href = "?purge">X</a>)<br>===================</h2>
	<div style = "margin-left:15;">
<table border = 0 cellpadding = 3 cellspacing = 0>
#foreach recently_type recently_name recently_size
	<tr>
		<td align = left valign = middle>
			<a href = "/fetch/$recently_name">$recently_name</a>
			<br>
			&nbsp;&nbsp;&nbsp;
			<b>$recently_size</b>
		</td>
	</tr>
#next
</table>
</div>
</td></tr></table>

