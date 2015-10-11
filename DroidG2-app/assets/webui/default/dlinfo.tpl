<h2 align = center>Download ID $id  details</h2>
<table align = center cellspacing = 0 cellpadding = 0 border = 0>
	<tr><td>File name</td> <td>&nbsp;&nbsp;</td> <td><b>$name</b></td></tr>
	<tr><td>Size</td> <td>&nbsp;&nbsp;</td> <td><b>$size ($hrsize)</b></td></tr>
	<tr><td>Progress</td> <td>&nbsp;&nbsp;</td> <td><b>$progress %</b></td></tr>
	<tr><td>Downloaders</td> <td>&nbsp;&nbsp;</td> <td><b>$downloaders</b></td></tr>
	<tr><td>Rate</td> <td>&nbsp;&nbsp;</td> <td><b>$rate ps</b></td></tr>
	<tr><td>SHA1</td> <td>&nbsp;&nbsp;</td> <td><b>$sha1</b></td></tr>
	<tr><td>TTR</td> <td>&nbsp;&nbsp;</td> <td><b>$ttr</b></td></tr>
	<tr><td>BP</td> <td>&nbsp;&nbsp;</td> <td><b>$bp</b></td></tr>
	<tr><td>ED2K</td> <td>&nbsp;&nbsp;</td> <td><b>$ed2k</b></td></tr>
	<tr><td>MD5</td> <td>&nbsp;&nbsp;</td> <td><b>$md5</b></td></tr>
#if $tiger_min
	<tr><td>TigerTree</td> <td>&nbsp;&nbsp;</td> <td><b>available (min chunk: $tiger_min)</b></td></tr>
#else
	<tr><td>TigerTree</td> <td>&nbsp;&nbsp;</td> <td><b>n/a</b></td></tr>
#endif

</table>
<br>
<div align = center><b>File map</b></div><br>
<table cellspacing = 0 cellpadding = 0 border = 0 align = center width = 50% height = 25><tr>

#foreach chunk_status chunk_size
<td bgcolor = 

#if $chunk_status = empty 
	"#DDDDDD"
#endif

#if $chunk_status = full
	"#9999FF"
#endif

#if $chunk_status = valid
	"#0000FF"
#endif

#if $chunk_status = invalid
	"#000000"
#endif

#if $chunk_status = active
	"#33DD55"
#endif

#if $chunk_status = mixed
	"#333366"
#endif

#if $chunk_status = 0
	"#FFFFFF"
#endif

width = $chunk_size%></td>
#next
</tr>
</table>
<br><br>
<div align = center><b>Fragmentation</b><br>(number of fragments followed by percent of file space)</div>
<table align = center width = 50% cellspacing = 0 cellpadding=0><tr><td>
Valid: $parts_valid  ($parts_valid_p%)<br>
<table width = $parts_valid_p% cellspacing = 0 cellpadding = 0><tr><td width = 100% height = 3 bgcolor="#0000FF"></td> </tr></table>
Not verified: $parts_full  ($parts_full_p%)<br>
<table width = $parts_full_p% cellspacing = 0 cellpadding = 0><tr><td width = 100% height = 3 bgcolor="#9999FF"></td> </tr></table>
Empty: $parts_empty  ($parts_empty_p%)<br>
<table width = $parts_empty_p% cellspacing = 0 cellpadding = 0><tr><td width = 100% height = 3 bgcolor="#DDDDDD"></td> </tr></table>
Active: $parts_active  ($parts_active_p%)<br>
<table width = $parts_active_p% cellspacing = 0 cellpadding = 0><tr><td width = 100% height = 3 bgcolor="#33DD55"></td> </tr></table>
Invalid: $parts_invalid  ($parts_invalid_p%)<br>
<table width = $parts_invalid_p% cellspacing = 0 cellpadding = 0><tr><td width = 100% height = 3 bgcolor="#000000"></td> </tr></table>
</td></tr></table>

<br>
<div align = center><b>Sources</b></div>
<br>
<table align = center border=0 cellpadding=3 cellspacing=0 >
<tr>
	<td> </td>
	<td></td>
	<td align = left><b>Host</b></td>
	<td></td>
	<td align = left><b>Connections</b></td>
	<td></td>
	<td align = left><b>User</b></td>
	<td></td>
	<td align = right><b>Connect after</b></td>
</tr>
	
	#foreach source_number source_endpoint source_connections source_max source_nick
		#include fancydark.tpl
		<tr bgcolor = "$color">
			<td align = right>$source_number</td>
			<td width = 10></td>
	
			<td align = left>$source_endpoint</td>
			<td width = 10></td>

			<td align = center>$source_connections  of $source_max</td>
			<td width = 10></td>

			<td align = left>$source_nick</td>
			<td width = 10></td>

			<td align = right>$source_after</td>
			<td width = 10></td>
		</tr>
	#next
	</table>

