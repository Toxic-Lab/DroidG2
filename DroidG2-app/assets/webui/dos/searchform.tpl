<FORM name="search" action="srch" method="POST">
<table align = center cellspacing=0 cellpadding=3 border = 0>
	<tr>
		<td>
			Name
		</td>
		<td>
			<INPUT type="text" class="texta" name="dn" size="50" maxlength="100">
		</td>
	</tr>
	<tr>
		<td>
			Min size 
		</td>
		<td>
			<INPUT type="text" class="texta" name="min" value="0" size = "8" maxlength="10">
		(e.g. 805, 700mb, 1024kb) <BR>
		</td>
	</tr>
	<tr>
		<td>
			Max size 
		</td>
		<td>
			<INPUT type="text" class="texta" name="max" value="0" size = "8" maxlength="10">
			(0 for any)
		</td>
	</tr>
	<tr>
		<td>
			Media
		</td>
		<td>
			<select name="media" class="texta">
				<option value="any">any</option>
				<option value="video">video</option>
				<option value="audio">audio</option>
				<option value="pic">image</option>
				<option value="doc">document</option>
				<option value="store">archive/image</option>
				<option value="virus">program</option>
			</select>
		</td>
	</tr>
	<tr>
		<td>
			File types 
		</td>
		<td>
			<INPUT type="text" class="texta" name="ext" value="" size = "20" maxlength="100">
			Example: &quot;gif jpeg mp3&quot;
		</td>
	</tr>
</table>
<br>
<DIV align = center> <INPUT type="submit" class = "button" value="Search"></DIV>
</FORM>
<br>
