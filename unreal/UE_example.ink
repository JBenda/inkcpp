# date:2025.03.22
LIST Potions = TalkWithAnimals, Invisibility
LIST Clues = Skull, Feather
LIST Knowladge = YellowDress
VAR Inventory = (Skull, TalkWithAnimals)
VAR Health = 100
LIST StatusConditions = CanTalkWithAniamls, IsInvisible

-> Mansion.Car

EXTERNAL transition(to)
=== function transition(to) ===
~ return 0

=== function walking(to) ===
You startk walking to {to}.
~ transition(to)
~ return

=== Wait
-> DONE

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
    {StatusConditions ? (CanTalkWithAniamls): Drinking more of it will not increase the effect.}
    + [Put it Back]
      You put the potion back into your pouch.
      -> END
    + {not (StatusConditions ? CanTalkWithAniamls)} [Drink]
    ~ StatusConditions += CanTalkWithAniamls
    A take a sip. The potion tastes like Hores, it is afull.
    -> END
= TInvisibility
    A potion which allows the consumer to stay unseen for the human eye. Not tested
    wtih other speccies.
    {StatusConditions ? (IsInvisible): Drinking more of it will not increase the effect.}
    + [Put it Back] 
      You put the potion back into your pouch.
      -> END
    + {not (StatusConditions ? (IsInvisible))}[Drink]
    ~ StatusConditions += IsInvisible
    You put a small drop on your thoungh. It feels like liking a battery, such a nice feeling
    -> END

=== Faint
# background:Unconscious
You collapse, the next thing you can remember is how you are given to the ambulance.
No further action for today ...
-> DONE

=== Mansion
= Car
# background:Car
You step outside your car. Its a wired feeling beehing here again.
-> Car_cycle
= Car_cycle
# background:Car
+ (look_around)[look around # Type:Idle]
    It is a strange day. Despite it beeing spring, the sky is one massive gray soup. # style:Gray
    -> Car_cycle
+ [go to the mension]
    ~ walking("Mansion.Entrance")
    -> Entrance

= Entrance
# background:Mansion
{not Mansion.look_around: 
    ~ Knowladge += YellowDress
    Just in time you are able to see the door, someone with with a yellow summer dress enters it. 
}
You're climbing the 56 steps up to the door; high tides are an annoying thing.
-> Entrance_cycle
= Entrance_cycle
# background:Mansion
+ (look_around) [look around # Type:Idle ]
  While watching around you, <>
    {Inventory hasnt Invisibility:
        see a small bottle in the Pot next to the door. 
    -else:
        see nothing of intrest
    }
  -> Entrance_cycle
* {look_around} [Pick up the bottle]
  ~ Inventory += Invisibility
  You pick up the bottle and inspect it more. It is labeld "<Blue>Invisible</>, just this one word written with and edding.
  -> Entrance_cycle
+ (knock)[Knock {knock: again?} # {knock: Type:Danger} ]
    ~ Health -= 20
    "<Red>Ahh</>", you cry while reaching for the door bell. Saying it was charched would be an understatement.
    { Health <= 0: -> Faint}
    -> Entrance_cycle
+ {knock && Knowladge ? (YellowDress)} [Inspect the Door]
    You just saw someone enter, how did they do not get shoked?
    <Gray>It seems the developr run out of time, maybe in the next version we can continue?</>
    -> Entrance_cycle
-> DONE
