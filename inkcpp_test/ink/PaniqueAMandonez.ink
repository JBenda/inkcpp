INCLUDE world/locations.ink
INCLUDE includes/verbose.ink
INCLUDE story/triggers.ink
INCLUDE world/characters.ink
INCLUDE world/topics.ink
INCLUDE story/conversation.ink
INCLUDE story/investigation.ink




#TITLE: Panique à Mandonez
#AUTHOR: ju / smwhr

-> menu

=== menu

-(opts)
 + [Commencer] #CLEAR
    -> init
 +(credits){not came_from(-> credits)} Informations sur le jeu
 
 Toute ressemblance avec des faits et des personnages existants ou ayant existé serait purement fortuite et ne pourrait être que le fruit d’une pure coïncidence. La ville de Mandonez-Savès n'existe pas et se base sur une vision très biaisée du Gers que je visite régulièrement. 
 
 Ce jeu a été écrit en ink à l'occasion du Concours de Fiction Interactive Francophone 2024. 
 
 N'hésitez pas à signaler tout ce que que vous pensez utile de signaler (bugs, fautes d'orthographe, incohérence, etc), cette œuvre n'ayant pas eu toute l'attention qu'elle aurait dû durant sa phase de test :)
 
 Version 2 (2024-03-08) ; IFID : 32F42657-F7A8-4D02-8324-7FF133DC14A9
    ->opts
    -> init

=== init
Mandonez-Savès, Gers, 5 juillet 1992, 13h

Le bus des Liaisons Occitanes s'arrête au milieu du village.

Vous êtes ici parce que vous avez reçu une lettre :

```
Camille,

Lorsque cette lettre vous trouvera, il sera déjà trop tard.
Retrouvez moi à Mandonez le 5 juillet prochain dans l'Ancien Clocher.
Et vous saurez la Vérité.

Ludivine, Comtesse de Peyrian
```

-(opts)
* [Observer le village # investigate]
    ~ describe(PlaceMairie)
    Le chauffeur émet un grognement d'impatience. # didascalie
    -> opts
+ [Descendre # action]
    ~ describe(PlaceMairie) 
    
    À peine avez-vous posé le pied au sol que les portes du bus se referment derrière vous. #didascalie
    Le bus redémarre. #didascalie
    ~MoveTo(Me, PlaceMairie)

-
-> loop


=== loop

<- exits(CurrentLocation, ->on_exit)

~temp CharactersInRoom = CharactersIn(LocationOf(Me)) - (Me)

{CharactersInRoom ? Maire :  <-ConversationMaire(->on_flow, ->on_choice)}
{CharactersInRoom ? Comtesse :  <-ConversationComtesse(->on_flow, ->on_choice)}
{CharactersInRoom ? Pretre :  <-ConversationPretre(->on_flow, ->on_choice)}
{CharactersInRoom ? Barmaid :  <-ConversationBarmaid(->on_flow, ->on_choice)}
{CharactersInRoom ? Tenor :  <-ConversationTenor(->on_flow, ->on_choice)}
{CharactersInRoom ? Chatelain :  <-ConversationChatelain(->on_flow, ->on_choice)}
{CurrentLocation == BureauMaire: <-InvestigateBureauMaire(->on_flow, ->on_choice)}
{CurrentLocation == Ecole: <-InvestigateEcole(->on_flow, ->on_choice)}
{CurrentLocation == ArriereBar: <-InvestigateArriereBar(->on_flow, ->on_choice)}
{CurrentLocation == PlacardElectrique: <-InvestigatePlacardElectrique(->on_flow, ->on_choice)}
{CurrentLocation == BureauChatelain: <-InvestigateBureauChatelain(->on_flow, ->on_choice)}

//Gather
+[Attendre # wait]
    Le temps s'écoule lentement. #didascalie
    ->on_choice

+ ->
-(on_flow)
~temp NeedExits = TURNS_SINCE(->exits) > 0
{NeedExits: <- exits(CurrentLocation, ->on_exit)}
+{NeedExits}[Attendre # wait]
    Le temps s'écoule lentement. #didascalie
    ->on_choice
->DONE
-(on_choice)
-> triggers -> loop

=== on_exit
-> triggers ->
Vous {toward(LocationOf(Me))} {verbose(LocationOf(Me))}.

~describe(LocationOf(Me))

{Locations hasnt LocationOf(Me):
  ~Locations+=LocationOf(Me)
}

~temp CharactersInRoom = CharactersIn(LocationOf(Me)) - (Me) //do not display that we are here !
{LocationOf(Me) == SalleBal:
    - ~CharactersInRoom = CharactersIn(BalLocations)
}

{BalLocations ? LocationOf(Me) and LIST_COUNT(LocationOf(Me)) == 1:
    - ~CharactersInRoom = ()
}


<>{LIST_COUNT(CharactersInRoom) > 0:
 - \ {listWithCommasAnd(CharactersInRoom, "")} {is_are(CharactersInRoom)} là. # didascalie
}

-> triggers_aftermove ->

-> loop


=== meet_the_comtesse
~Characters += Comtesse
La jeune fille sursaute. Elle ne vous avait pas vue. #didascalie
– Camille ! Je désespérais de vous voir arriver. Je n'ai pas beaucoup de temps alors écoutez moi bien. Tout le monde me recherche, mais ce n'est pas le plus important. Ne vous inquiétez pas pour moi. Je voulais me montrer à vous pour vous rassurer : il ne m'est rien arrivé. Pas encore en tout cas. Je vais fuir. Mais vous pouvez faire quelque chose pour moi : mes parents ont fait quelque chose de terrible, il faut que vous découvriez quoi. Ça a un rapport avec l'école, le Maire est dans le coup. Il cache quelque chose. Trouvez ce que c'est et révélez ce que vous aurez découvert ! Je dois y aller maintenant.
~Topics+=ComplotEcole

* – C'est compris !
    La Comtesse vous adresse un dernier signe avant de s'éloigner. #didascalie
    ~ MoveTo(Comtesse,NoWhere)
    ~ MoveTo(Maire, BureauMaire)
    ~ MoveTo(Tenor, Bar)

-
->loop


=== confront_maire

– Et quoi donc ?
    – Vous avez provoqué une inondation dans l'école afin de justifier sa destruction pendant que vous complotiez pour faire construire un centre aquatique à la place !
    – J'avoue que vous avez bien joué sur ce coup là. Mais qu'importe. Tout est déjà en marche, rien ne peut arrêter ce projet maintenant. Et même, je vais être beau joueur : voilà un carton pour le bal au Chateau de Peyrian, ce soir. Avec ou sans la petite Ludivine, que je n'ai pas trouvée d'ailleurs, il aura lieu. Et je vous mets au défi de trouver qui que ce soit qui s'oppose au projet !
    Le Maire vous tend un carton avec le plan vers le chateau. # didalie
    ~knowRouteToChateau = true
    – Allez ouste, on se voit tout à l'heure !
    Le Maire vous empoigne et vous met à la porte de son bureau. # didascalie
    ~ MoveTo(Me, Mairie)
    ~ MoveTo(Maire, BalMaire)
    ~ MoveTo(Barmaid, BalServeuse)
    ~ MoveTo(Pretre, BalPretre)
    ~ MoveTo(Chatelain, BalChatelain)
    ~ MoveTo(Tenor, BalTenor)
    Vous vous retrouvez dans le hall de la Mairie. <>
    ~describe(Mairie)
    ~ Moment = Soir
-
->loop


=== ouverture_caveau

Un pan de mur discret, camouflé par des étagères en apparence anodines, s'ouvre silencieusement. Derrière le faux mur, des barils métalliques sont soigneusement alignés. Leur surface est maculée de rouille, témoignant du passage du temps et de la négligence. #didascalie
Leur marquage ne laisse aucun doute quant à leur contenance : des têtes de mort stylisées et la mention "TOXIQUE". Qu'est-ce qu'ils peuvent bien faire là ?! #didascalie
Vous prenez quelques photos avec votre téléphone. Voilà qui va faire changer d'avis les villageois. #didascalie
    ~ Topics = ToxicBarrels

-
->loop


=== final_reveal

L'agitation parcourt la salle lorsque vous répondez d'une voix sonore : #didascalie
+ – Ce sont des fûts de produits toxiques cachés sous l'école.
-

– Je suis en mesure de tout vous expliquer, déclara solennellement le châtelain. "Il m'était impossible de confier au maire que mes parents, les anciens châtelains, avaient dissimulé des produits toxiques sous l'école. J'ai donc consenti à utiliser l'intégralité de ma fortune pour racheter l'établissement, feignant un projet de transformation en centre aquatique. À l'origine, mon intention était simplement de reconstruire l'école. Malheureusement, le maire n'était pas réceptif à cette idée. C'est alors que j'ai été contraint de jouer de ses instincts les plus sombres pour atteindre mes fins.

– Tout s'explique !

L'assistance se retourne vers la voix venue du dehors. # didascalie

+ Mais qui vient de parler ?
-

C'est Ludivine, la Comtesse de Peyrian qui vient d'apparaître par la porte fenêtre.

– Je suis profondément désolé, ma chérie. Je n'aurais jamais dû t'impliquer dans tout cela", déclare le Châtelain à sa fille avec une lueur de regret dans le regard, c'était un fardeau que je portais seul, et je regrette sincèrement de t'avoir entraînée dans cette affaire. Pardonne-moi.
-
+ Épilogue # title
    -> epilogue
    
=== epilogue

Mandonez-Savès, Gers, 16 juillet 1992, 13h

L'école, désormais débarrassée des fûts toxiques et rénovée grâce aux dons de la Comtesse rouvre ses portes avec une kermesse exceptionnelle. Les rires résonnent à nouveau dans la cour de récréation.

Dans l'ombre du château qui domine le village, la douceur de vivre reprend ses droits. L'ancien Maire a été arrêté pour corruption et à la terrasse du bar de Virginie, on se demande qui lui succèdera !


->END
