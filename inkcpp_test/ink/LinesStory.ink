Line 1

Line 2

-> line_3

== line_3

Line 3

-> line_4

= line_4

Line 4

-> DONE

== Functions ==
Function Line

~ funcTest()

-> DONE

== Tunnels ==
Tunnel Line

-> tunnel_test ->

-> DONE


=== function funcTest()
Function Result


== tunnel_test

Tunnel Result

->->

VAR forceful = 0
VAR evasive = 0

=== function raise(ref x)
	~ x = x + 1

== ignore_functions_when_applying_glue
	"I don't see why," I reply
	~ raise(forceful)
	~ raise(evasive)
	<>.
	-> DONE
