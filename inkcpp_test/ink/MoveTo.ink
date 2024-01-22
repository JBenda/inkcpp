-> Dialog

LIST Form = (Humen), Owl, Tiger

=== Transformations
= Nothing
	Hard you could transform if you would like
	->END
= ToOwl
	You Shrink and you get feathers. And, now you are a Owl
	~ Form = Owl
	->END
= ToTiger
	Your body growths, and growth, you feel the muscels building.
	And there you are, a tiger.
	{ Form == Owl:
		you puke a few feathers, where are they coming from?
	}
	~ Form = Tiger
	->END

=== Dialog
= introduction
	Lava kadaver a very boring introduction
	-> core

= core
You are head to head to the minister
* {Form == Tiger} [Roar] Grrrrh, Roarrr
	The people are quit confused
* {Form == Humen} Hellow mister menistery
* ->
	It seems you are out of options, you shold give up
	-> END
- -> core
	
