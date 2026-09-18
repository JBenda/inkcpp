=== function came_from(-> x) 
    ~ return TURNS_SINCE(x) == 0

=== function seen_recently(-> x)
    ~ return TURNS_SINCE(x) >= 0 && TURNS_SINCE(x) <= 1


=== function listWithCommasAnd(list, if_empty) 
    {LIST_COUNT(list): 
    - 2: 
        	{verbose(LIST_MIN(list))} et {listWithCommasAnd(list - LIST_MIN(list), if_empty)}
    - 1: 
        	{verbose(list)}
    - 0: 
			{if_empty}	        
    - else: 
      		{verbose(LIST_MIN(list))}, {listWithCommasAnd(list - LIST_MIN(list), if_empty)} 
    }
    
=== function is_are(list)
    {LIST_COUNT(list):
        - 1 : est
        - else : sont
    }

=== function toward(term)
{term:
    - Mairie: {PreviousLocation:
        - BureauMaire: sortez dans le hall de
        - else: entrez dans
    }
    - BureauMaire: entrez dans
    - PlaceMairie: retournez vers
    - Eglise: entrez dans
    - Bar: entrez dans
    - AncienClocher: suivez les indications fournies par le prêtre et parvenez jusqu'à
    - Route: empruntez
    - Ecole: entrez dans
    - Chateau: suivez les indications inscrites sur le carton d'invitation et parvenez devant
    - BalServeuse: approchez
    - BalTenor: approchez
    - BalPretre: approchez
    - BalMaire: approchez
    - BalChatelain: approchez
    - SalleBal: {BalLocations ? PreviousLocation:
        - retournez au centre de 
        - else: entrez dans
    }
    - BureauChatelain: entrez dans
    - else: vous dirigez vers
}
    

=== function verbose(term)
{term:

    // Locations
    - Me: Vous
    - Mairie: {CurrentLocation == BureauMaire: le hall de }la Mairie
    - BureauMaire: le Bureau du Maire
    - PlaceMairie: la place du village
    - Eglise: l'église
    - Ecole: l'école
    - Bar: le bar
    - AncienClocher: l'ancien clocher
    - Chateau: le Chateau de Peyrian
    - Route: la route bordée de platanes
    - SalleBal: {BalLocations ? CurrentLocation: le centre de }la salle de bal
    - Vestibule: le hall d'entrée du chateau
    - PlacardElectrique: le placard électrique
    - BalServeuse: la serveuse
    - BalTenor: le ténor
    - BalPretre: le prêtre 
    - BalMaire: le maire
    - BalChatelain: le Châtelain
    - BureauChatelain: le bureau du Châtelain
    
    // Characters
    - Maire: Le Maire
    - Comtesse: {Une jeune femme blonde qui semble attendre quelque chose|La jeune femme blonde}
    - Pretre: Le prêtre
    - Barmaid: {Characters !? Barmaid : Une jeune serveuse blonde|Virginie la serveuse}
    - Tenor: {Characters !? Tenor : Un homme d'une cinquantaine d'années|Roberto le Ténor}
    - Chatelain: Le Châtelain
    - else: {term}
}
