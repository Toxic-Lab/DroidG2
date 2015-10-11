<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
#if $reload
		<meta http-equiv="refresh"  content="$reload">
#endif 
 		<link rel="icon" type="image/png" href="/img/fav.png" />
		<title>Sharelin</title>
		<style type="text/css">
#body {overflow-y:scroll;} 
a
{
	text-decoration: none;
	color: #FF6600;
}
a:active
{
	text-decoration: none;
	color: #FF6600;
}
a:visited
{
	text-decoration: none;
	color: #FF6600;
}

a:hover
{
	text-decoration: underline;
	color: #FF6600;
}

table {font-size:12px;}

.texta {
	font-size: 12px;
	background-color: #DDDDDD;
	border: 1px solid #666666;
}

.button {
	border: 1px solid;
}

.tinyform {
	margin: 0px; padding: 0px;
}

.hl {
	background-color: #e8e8e8;
	padding-top: 5px;
	padding-right: 5px;
	padding-bottom: 5px;
	padding-left: 5px;
}

.nohl {
	background-color: #ffffff;
	padding-top: 5px;
	padding-right: 5px;
	padding-bottom: 5px;
	padding-left: 5px;
}
		</style>
	</head>
	<body
#if $pgname = search
	onload = "document.search.dn.focus();"
#endif
	>
	<table cellspacing = 0 cellpadding = 0 width = 100%>
		<tr><td>
			#include menu.tpl
		</td>
		<td valign=top>
#if $pgname != none
			#include pageprefs.tpl
#endif
		</td>
		</tr>
	</table>
	<table cellspacing = 0 cellpadding = 0 width = 100%>
		<tr><td><br>
		<iframe name = "status" frameborder = 0 width=100% height=50 align="center"> </iframe>		
		<br></td></tr>
		<tr><td>
		<table cellspacing=0 cellpadding=0 width=80% align=center border = 0>
			<tr width = 100%>
				<td valign = top>
#include selectpage.tpl
				</td>
			</tr>
		</table>		
		</td></tr></table>
	</body>
</html> 
