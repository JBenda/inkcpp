VAR PreviousLocation = ()
VAR CurrentLocation = ()

LIST Locations = Bus, PlaceMairie, Mairie, BureauMaire, Ecole, Eglise, Bar, ArriereBar, AncienClocher, Route, Chateau, PlacardElectrique, SalleBal, BalServeuse, BalTenor, BalPretre, BalMaire, BalChatelain, BureauChatelain, Vestibule, NoWhere

VAR knowRouteToAncienClocher = false
VAR knowRouteToChateau = false
VAR knowArriereBar = false

LIST Moment = Jour, Soir
~ Moment = Jour

VAR BalLocations = (BalServeuse, BalTenor, BalPretre, BalMaire, BalChatelain)

=== exits(room, ->next)

~temp Exits = ()
{room:
    - PlaceMairie:
        ~ Exits = (Mairie, Eglise, Bar, Route)
    - Mairie:
        ~ Exits = (PlaceMairie, Ecole)
        {LocationOf(Maire) == BureauMaire or LocationOf(Maire) == AncienClocher:
            - ~Exits += (BureauMaire)
        }
    - BureauMaire:
        ~ Exits = (Mairie)
    - Eglise:
        ~ Exits = (PlaceMairie)
    - Ecole: 
        ~ Exits = (Mairie, PlaceMairie)
    - Bar:
        ~ Exits = (PlaceMairie)
    - ArriereBar:
        ~ Exits = (Bar)
    - Route:
        ~ Exits = (PlaceMairie)
        {knowRouteToAncienClocher:
            ~Exits += (AncienClocher)
        }
        {knowRouteToChateau:
            ~Exits += (Chateau)
        }
    - AncienClocher:
        ~ Exits = (PlaceMairie)
    - Chateau: 
        ~ Exits = (PlaceMairie, SalleBal)
    - SalleBal:
        ~ Exits = BalLocations + (Vestibule)
        {LocationOf(Chatelain) != BalChatelain:
            ~ Exits -= BalChatelain
        }
    -BalServeuse:
        ~ Exits = (SalleBal)
    - BalTenor:
        ~ Exits = (SalleBal)
    - BalPretre:
        ~ Exits = (SalleBal)
    - BalMaire:
        ~ Exits = (SalleBal)
    - BalChatelain:
        ~ Exits = (SalleBal)
    - Vestibule:
        ~ Exits = (SalleBal, BureauChatelain, PlacardElectrique, PlaceMairie)
    - PlacardElectrique:
        ~ Exits = (Vestibule)
    - BureauChatelain:
        ~ Exits = (Vestibule)
    
}
<-listExits(Exits, next)
->DONE


=== function describe(room)

{room:
	- PlaceMairie: {Moment:
	    -Jour:{<> Le soleil écrasant plonge Mandonez sur Saves dans la torpeur estivale. Les rayons ardents filtrent à travers les feuillages des vieux platanes. Les motifs dansants de lumière et d'ombre se projettent sur le sol poussiéreux. Les maisons de pierre aux volets colorés bordent la place. Faute de vent, le drapeau tricolore pend tristement sur la Mairie de briques roses. Quelques tables en fer forgé sont sorties devant un bar dont l'enseigne est illisible. Par delà les platanes, le clocher de l'église domine l'horizon sous le ciel azur.  #CLASS: lettrine |<> { shuffle:
    	    - Le soleil écrasant baigne la petite place de Mandonez.
    	    - Les platanes au feuillage épais offrent un peu de répis face à la chaleur de plomb.
    	    - Le clocher de l'église domine l'horizon sous le ciel azur.
    	    - Les façades de pierre aux volets colorés bordent la place.
    	    - Sous le soleil brûlant, la place semble déserte et paisible.
    	    }}# didascalie
	    -Soir:{<> À Mandonez, le soleil s'est couché, plongeant le village dans la quiétude. Les silhouettes sombres des platanes bruissent sous la brise du soir. Le clocher de l'église se découpe nettement contre le ciel étoilé. #CLASS: lettrine|<> { shuffle:
	        - Le clocher de l'église se découpe nettement contre le ciel étoilé.
	        - Une douce brise nocturne caresse les feuilles des arbres endormis.
	        }} #didascalie
	    }
	- Mairie: {<> Il n'y a personne dans la salle qui sert de hall. Un guéridon est disposé dans un coin où traînent de vieux prospectus jaunis. #CLASS: lettrine | }À gauche, une plaque indique le bureau du Maire. À droite, une grille en fer forgé donne sur {ce qui semble être une|la} cour de l'école.{LocationOf(Maire) == BureauMaire or LocationOf(Maire) == AncienClocher: La porte du bureau du Maire est entrouverte{LocationOf(Maire) == AncienClocher:, dans la précipitation il a oublié de la verrouiller}.| La porte du bureau du Maire est fermée.} # didascalie
	- Eglise: {<> Le grand clocher de l'église domine le village. #CLASS: lettrine |<>} Une fois à l'intérieur vos yeux mettent quelques secondes à s'habituer à l'obscurité.# didascalie
	- Bar: {<> #CLASS: lettrine|}# didascalie
	- Route: {not knowRouteToAncienClocher: Vous n'avez pas envie de vous aventurer en dehors de la ville pour le moment}<> #CLASS: lettrine # didascalie
	- AncienClocher: <> #CLASS: lettrine # didascalie
	- Ecole: {<> Les signes de l'inondation dans l'école sont évident.  Les murs sont marqués par l'humidité, le sol est recouvert de débris, et les salles de classe sont en désordre avec des fournitures éparpillées.| <> Le sol est boueux.} La structure menace de s'effondrer, s'aventurer plus loin pourrait s'avérer dangereux. #didascalie
	- ArriereBar: L'arrière boutique du bar est une terrasse en bois semi-ouverte vers l'extérieur dont les pilotis trempent dans le cours d'eau. Quelques fûts de bière attendent d'être consommés. #didascalie
	- Chateau: {<> Le château de Peyrian se dresse sur une colline, majestueux et ancien. Ses murs de pierre et ses tourelles témoignent de son passé riche et ancien. Entouré de jardins soignés, il respire l'élégance et le charme.|} Le portail est ouvert vers la salle de bal. #didascalie
	- BureauChatelain: <> Il y a des documents sur le bureau du châtelain, ainsi qu'un plan affiché au mur. Une armoire est entrouverte. #didascalie
}

=== listExits(list, ->next)
    ~ temp current_ctc = LIST_MIN(list)
    
    { LIST_COUNT(list) > 0 :
        <- listExits(list - current_ctc, next)
        + [Vers {verbose(current_ctc)}... #exit]
            ~PreviousLocation = CurrentLocation
            ~MoveTo(Me, current_ctc)
            -> next
        
    } 
