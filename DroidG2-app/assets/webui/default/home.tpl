<h2 align=center>$full_name</h2>
<div align = center>
	<a href = "http://sharelin.sf.net">Home</a>&nbsp;&nbsp;&nbsp;
	<a href = "http://sourceforge.net/forum/?group_id=235758">Forums</a>&nbsp;&nbsp;&nbsp;
	<a href = "irc://irc.p2pchat.net:6667/sharelin">IRC</a>
</div>
<br>
<br>
#include linkform.tpl
	<br>
#if $version
	<table cellspacing = 5 cellpadding = 0 border = 0 align = center>
	<tr>
	<td valign = top><img src = /img/hint.png></td>
	<td><div style="text-align: center; padding-right: 64px;"><b>New version available: <a href=$vlink>$version</a></b></div><br>$vcomments</td>
	</tr>
	</table>
#endif

<h3 align = center>Status</h3>
<div align = center>
	<table cellspacing = 0 cellpadding = 0 border = 0 align = center><tr><td>
	<table cellspacing = 0 cellpadding = 0 border = 0>
		<tr><td>Downloads</td> <td>&nbsp;&nbsp;</td> <td><b>$total_downloads</b></td></tr>
		<tr><td>Uploads</td> <td>&nbsp;&nbsp;</td> <td><b>$total_uploads</b></td></tr>
		<tr><td>Searches</td> <td>&nbsp;&nbsp;</td> <td><b>$total_searches</b></td></tr>
		<tr><td>Average bw in</td> <td>&nbsp;&nbsp;</td> <td><b>$hr_avr_bw_in ps</b></td></tr>
		<tr><td>Average bw out</td> <td>&nbsp;&nbsp;</td> <td><b>$hr_avr_bw_out ps</b></td></tr>
		<tr><td>Firewalled</td> <td>&nbsp;&nbsp;</td> <td><b>$firewalled</b></td></tr>
	</table>
	<br>
	<b>Neighbors</b>
	<div style = "margin-left:15;">
	<table cellspacing = 0 cellpadding = 3 border = 0>

#foreach num address status

#include fancydark.tpl
	<tr bgcolor=$color>
	<td>$num</td>
	<td width=10></td>

	<td>$address</td>
	<td width=10></td>

	<td>$status</td>
	</tr>
#next	
	</table>
</div>

<br><br><div align = right> <a href = /kill>kill core</a></div>
	
</td></tr></table>
