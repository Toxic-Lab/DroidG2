<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
#if $reload
		<meta http-equiv="refresh"  content="$reload">
#endif 
 		<link rel="icon" type="image/png" href="/img/fav.png" />
		<title>Sharelin</title>
		<style type="text/css">
body {
	background: #000000;
	color: yellow;
	font-family: "fixed";
} 
a
{
	text-decoration: none;
	color: yellow;
}
a:active
{
	text-decoration: none;
	color: yellow;
}
a:visited
{
	text-decoration: none;
	color: yellow;
}

a:hover
{
	text-decoration: none;
	color: yellow;
}

table {font-size:12px;}

.texta {
	font-size: 12px;
	color: yellow;
	background-color: black;
	border: 1px solid yellow;
}

.button {
	font: bold;
	border: 1px solid yellow;
	background-color: black;
	color: yellow;
}

.tinyform {
	margin: 0px; padding: 0px;
}

		</style>
	</head>
	<body 
#if $pgname = search
	onload = "document.search.dn.focus();"
#endif
	>

	<table cellspacing = 0 cellpadding = 0>
		<tr><td valign = top>
			#include menu.tpl
		<br>
#if $pgname != none
			#include pageprefs.tpl
#endif
		</td>
		<td width = 30></td>
		<td>
		<iframe name = "status" frameborder = 0 width=100% height=30 align="center"> </iframe>		
		<br>
		<table cellspacing=0 cellpadding=0 border = 0>
			<tr>
				<td valign = top>
#include selectpage.tpl
				</td>
			</tr>
		</table>	
		<br>
		<div align = right><img src="/img/cursor.gif"></div>
		</td></tr></table>
	</body>
</html> 
