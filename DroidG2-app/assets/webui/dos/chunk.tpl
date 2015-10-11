<td>

#if $state = empty 
	.
#endif

#if $state = full
	&#x2591;
#endif

#if $state = valid
	&#x2588;
#endif

#if $state = invalid
	X
#endif

#if $state = active
	A
#endif

#if $state = mixed
	&#x2592;
#endif

</td>