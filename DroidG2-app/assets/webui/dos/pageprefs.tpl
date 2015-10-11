
<script language="javascript"> 
function ShowPrefs(link) 
{
	if(typeof ShowPrefs.shown == 'undefined')
	{
		ShowPrefs.shown = false;
	}

	var elPrefs = document.getElementById("prefs");

	ShowPrefs.shown = !ShowPrefs.shown;
	if(ShowPrefs.shown) 
	{
		elPrefs.style.display = "block";
		link.innerHTML = "&lt;"
	}
	else 
	{
		elPrefs.style.display = "none";
		link.innerHTML = "&gt;"
	}
} 
</script>

<div align = left>
<a id = "aPagePrefs" style="font-size: 12px; text-decoration: none;" href="#" 
onclick="javascript: ShowPrefs(this); return false">&gt;</a> 
</div>

#def prefix
#if $pgname = dlinfo
	#def prefix info=$id&
#endif
#if $pgname = srchinfo
	#def prefix show=$id&
#endif

<div id="prefs" style ="width=auto; height=230px;  margin-top:0px; display: none">
Reload every <br>

#if $reload = 0
<b>
#endif
<a href = "?$prefix pgreload=0">off</a>
#if $reload = 0
</b>
#endif

#if $reload = 5
<b>
#endif
<a href = "?$prefix pgreload=5">5s</a>
#if $reload = 5
</b>
#endif

#if $reload = 30
<b>
#endif
<a href = "?$prefix pgreload=30">30s</a>
#if $reload = 30
</b>
#endif

#if $reload = 60
<b>
#endif
<a href = "?$prefix pgreload=60">1m</a>
#if $reload = 60
</b>
#endif

#if $reload = 300
<b>
#endif
<a href = "?$prefix pgreload=300">5m</a>
#if $reload = 300
</b>
#endif

#if $reload = 1800
<b>
#endif
<a href = "?$prefix pgreload=1800">30m</a>
#if $reload = 1800
</b>
#endif

#if $reload = 3600
<b>
#endif
<a href = "?$prefix pgreload=3600">1h</a>
#if $reload = 3600
</b>
#endif

</div>

