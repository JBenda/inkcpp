->Start
EXTERNAL SetBrightness(x)
=== function SetBrightness(x) ===
      ~ return

EXTERNAL GetGreeting()
=== function GetGreeting() ===
	~ return "Hey, "

VAR brightness = 50

VAR date = "???"

LIST background = (a), b, c

=== Start ===
{GetGreeting()}, your personal assistent here.  # Story Start
Today is the {date}
Why we don't start with some customisation options:
* [Yes]
	-> Settings ->
	How The story looks now? much better :)
	->DONE
* [I Don't like you.]
	Then Bye
	->DONE

=== Settings ===

= All
Let Start with the color.
->Color->
The illumination seems off?
->Brightness->
And now some custom background :)
->Background->
->->

= Color
Which color do like?
+ [Magenta # c:mag]
	A wired choice ... # setColor_255_0_255
+ [Cyan # c:cya]
	A think this will be a intense experience. # setColor_0_255_255
+ [Yellow # c:yel]
	A delighting decision. # setColor_255_255_0
- Do You Like your decision?
+ [Yes] ->->
+ [No] -> Color

= Brightness
Do you like the Brightness level?
+ [A bit more please.]
	~ brightness += 5
+ [A bit less.]
	~ brightness -= 5
+ [The settings {is fine| is now to my compliance!}]
	->->
-   ~ SetBrightness(brightness)
	->Brightness

= Background
>> SetBg({background})
How do you like this image?
+ {background < LIST_MAX(LIST_ALL(background))} [Mhe the next please.]
	~ background++
+ {background > LIST_MIN(LIST_ALL(background))} [Can you show me the previous please?]
	~ background--
+ [This is great]
	->->
- -> Background
