LIST Potions = TalkWithAnimals, Invisibility
LIST Clues = Skull, Feather
VAR Inventory = (Skull, Invisibility)
VAR Health = 100
VAR can_talk_with_animals = false
VAR is_insible = false

-> Mansion.Car

EXTERNAL transition(to)
=== function transition(to) ===
~ return 0

=== function walking(to) ===
You startk walking to {to}.
~ transition(to)
~ return

=== TClues
= TSkull
    A human skull, why do I have this again?
    -> END
= TFeather
    A Large Feather found insede the dining room, I wounder from which bird it is.
    -> END
=== TPotions
= TTalkWithAnimals
    A potion which allows the consumer to talk with a variaty of animals. Just make sure
    your serroundings do not think you are crazy.
    + [Put it Back] -> END
    + [Drink]
    A take a sip. The potion tastes like Hores, it is afull.
    ~ can_talk_with_animals = true
    -> END
= TInvisibility
    A potion which allows the consumer to stay unseen for the human eye. Not tested
    wtih other speccies.
    + [Put it Back] -> END
    + [Drink]
    You put a small drop on your thoungh. It feels like liking a battery, such a nice feeling
    ~ is_insible = true
    -> END

=== Mansion
= Car
You step outside your car. Its a wired feeling beehing here again.
+ (look_around)[look around]
    It is a strange day. Despite it beeing spring, the sky is one massive gray soup.
    -> Car
+ [go to the mension]
    ~ walking("Mansion.Entrance")
    -> Mansion.Entrance

= Entrance
{not look_around: Just in time you are able to see the door, someone with with a yellow summer dress enters it.}
You climbing the 56 steps up to the door, high water is a dump thing.
-> DONE

-> DONE