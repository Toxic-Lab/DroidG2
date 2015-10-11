<h2 align = center>Settings</h2>
<FORM name="settings" action="settings" method="POST" target = "status" class="tinyform">
<table align = center cellspacing=0 cellpadding=3 border = 0>


<tr><td><h3 align = left>General</h3></td><td></td></tr>
	
	<tr>
		<td>
			User name
		</td>
		<td>
<INPUT type="text" value = "$username" class="texta" name="username" size="15" maxlen="50">
		</td>
	</tr>

	<tr>
		<td>
			Web UI port*
		</td>
		<td>
<INPUT type="text" value = "$webuiport" class="texta" name="webuiport" size="15" maxlen="50">
<i>blank to disable</i>
		</td>
	</tr>

	<tr>
		<td>
			Remote UI allowed ip
		</td>
		<td>
<INPUT type="text" value = "$remoteranges" class="texta" name="remoteranges" size="50" maxlen="250">
		</td>
	</tr>

	<tr>
		<td>
			Telnet port*
		</td>
		<td>
<INPUT type="text" value = "$telnetport" class="texta" name="telnetport" size="15" maxlen="50">
<i>blank to disable</i>
		</td>
	</tr>

	<tr>
		<td>
			Lines per page in webui
		</td>
		<td>
<INPUT type="text" value = "$wuipagelines" class="texta" name="wuipagelines" size="15" maxlen="50">
<i>blank to disable</i>
		</td>
	</tr>
	<tr>
		<td>
			Log file size
		</td>
		<td>
<INPUT type="text" value = "$logsize" class="texta" name="logsize" size="15" maxlen="50">
<i>in Mb, 0 to disable</i>
		</td>
	</tr>
	<tr>
		<td>
			WebUI style
		</td>
		<td>
<INPUT type="text" value = "$webuistyle" class="texta" name="webuistyle" size="15" maxlen="50">
<i>blank for default. Built-in themes: <b>default, dos</b></i>
		</td>
	</tr>

<tr><td><br><h3 align = left>Network</h3></td><td></td></tr>

	<tr>
		<td>
			Listen port*
		</td>
		<td>
<INPUT type="text" value = "$listenport" class="texta" name="listenport" size="15" maxlen="50">
			<a target="_blank" href=http://gwc.dyndns.info:28960/connectiontest/?test=1&port=$listenport>Connection test</a>
		</td>
	</tr>
	
	<tr>
		<td>
			Direct connection*
		</td>
		<td>
<INPUT type="text" value = "$directable" class="texta" name="directable" size="15" maxlen="50">
<i><b>yes</b>, <b>no</b>, <b>auto</b></i>
		</td>
	</tr>
	
	<tr>
		<td>
			Max bandwidth in
		</td>
		<td>
<INPUT type="text" value = "$max_bw_in_kb" class="texta" name="maxbwin" size="15" maxlen="50">
<i>kbps</i>
		</td>
	</tr>
	
	<tr>
		<td>
			Max bandwidth out
		</td>
		<td>
<INPUT type="text" value = "$max_bw_out_kb" class="texta" name="maxbwout" size="15" maxlen="50"> 
<i>kbps</i>
		</td>
	</tr>
	
	
<tr><td><br><h3 align = left>Sharing</h3></td><td></td></tr>

	<tr>
		<td>
			Shared folders*
		</td>
		<td>
<INPUT type="text" value = "$share" class="texta" name="share" size="50" maxlen="250">
<i>use '<b>;</b>' to separate folders</i>
		</td>
	</tr>
	
	<tr>
		<td>
			Folder for complete files*
		</td>
		<td>
<INPUT type="text" value = "$complete" class="texta" name="complete" size="50" maxlen="250">
		</td>
	</tr>
	
	<tr>
		<td>
			Folder for incomplete files*
		</td>
		<td>
<INPUT type="text" value = "$incomplete" class="texta" name="incomplete" size="50" maxlen="250">
		</td>
	</tr>

<tr><td><br><h3 align = left>Advanced</h3></td><td></td></tr>
	
	<tr>
		<td>
			Max hubs
		</td>
		<td>
<INPUT type="text" value = "$maxhubs" class="texta" name="maxhubs" size="15" maxlen="3">
		</td>
	</tr>
	

	<tr>
		<td>
			Uploads per host
		</td>
		<td>
<INPUT type="text" value = "$uploadsperhost" class="texta" name="uploadsperhost" size="15" maxlen="50">
		</td>
	</tr>
	
	<tr>
		<td>
			Max concurrent uploads
		</td>
		<td>
<INPUT type="text" value = "$maxuploads" class="texta" name="maxuploads" size="15" maxlen="50">
		</td>
	</tr>

	<tr>
		<td>
			Upload queue size
		</td>
		<td>
<INPUT type="text" value = "$queuesize" class="texta" name="queuesize" size="15" maxlen="50">
		</td>
	</tr>
	
	<tr>
		<td>
			Max concurrent downloaded files
		</td>
		<td>
<INPUT type="text" value = "$maxfiles" class="texta" name="maxconcurrentfiles" size="15" maxlen="50">
		</td>
	</tr>
	<tr>
		<td>
			Max concurrent downloads
		</td>
		<td>
<INPUT type="text" value = "$maxdownloads" class="texta" name="maxdownloads" size="15" maxlen="50">
		</td>
	</tr>
	
	<tr>
		<td>
			Hashing rate
		</td>
		<td>
<INPUT type="text" value = "$hashrate" class="texta" name="hashrate" size="15" maxlen="50">
<i>Mbps, 1 is lowest rate, 0 to disable limiting</i>
		</td>
	</tr>
	

</table>
<div align = right><b>*</b> restart Sharelin to take effect</div>
<br>
<DIV align = center> <INPUT type="submit" class = "button" value="Save changes"></DIV>
</FORM>
