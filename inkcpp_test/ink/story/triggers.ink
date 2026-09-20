VAR EcoleOpen = false
VAR triedGrille = false
VAR ChatelainOpen = false
VAR BipToxicBarrels = false

=== triggers

{TURNS_SINCE(->ConversationBarmaid.ask_appeler):
- 4 : 
    ~MoveTo(Pretre, Bar)
    {LocationOf(Me) == Bar: Un prêtre en soutane entre dans le bar et s'installe à ce qui semble être sa place habituelle. #didascalie}
}
{ConversationMaire.talk_to_maire >= 2 and LocationOf(Me) == LocationOf(Maire) and LocationOf(Me) == BureauMaire and not vuTiroir: 
    - Le Maire recule son fauteuil, farfouille dans un tiroir de son bureau et attrape quelque chose qu'il porte à sa bouche. #didascalie
        ~vuTiroir = true
}

{ConversationMaire.talk_to_maire == 5 and hasCalissons and Characters !? Comtesse:
    - {LocationOf(Me) == BureauMaire:
        - Le Maire se lève pour vous signaler que c'est la fin de cet entretien et vous raccompagne dans le vestibule de la Mairie.
        ~MoveTo(Me, Mairie)
        
        }
      {LocationOf(Maire) == BureauMaire:
        ~MoveTo(Maire, Chateau)
      }
}

{LocationOf(Me) == Ecole  and not EcoleOpen:
    - ~MoveTo(Me, Mairie)
        Vous ne pouvez pas, la grille est fermée.
        ~triedGrille = true
        -> loop
}

{Topics ? (CentreAquatique, InondationSuspecte):
    -{once:
        - Avec vos suspicions sur la nature de l'inondation de l'école et le document trouvé dans les archives du Maire, vous avez assez d'éléments pour tenter une confrontation avec ce dernier. # didascalie
        ~ MoveTo(Maire, BureauMaire)
        {LocationOf(Me) == BureauMaire:
            - C'est justement le moment que choisit le Maire pour entrer dans son bureau ! #didascalie
        }
    }
}

{ (Bar, Eglise) ? LocationOf(Me)  and Moment == Soir:
    - Vous ne pouvez pas, {verbose(LocationOf(Me))} est {fermé|verrouillé|inaccessible} pour la soirée.
    ~MoveTo(Me, PlaceMairie)
        -> loop
}

{LocationOf(Me) == BureauChatelain and not ChatelainOpen:
    - ~MoveTo(Me, Vestibule)
    Vous ne pouvez pas entrer, la porte est fermée.
    -> loop
}


-
->->

=== triggers_aftermove

{LocationOf(Me) == Mairie and LocationOf(Tenor) == Mairie and not EcoleOpen:
    - 
    – Je vous ouvre, déclare Roberto, {Topics ? Inondation: comme je vous ai dit, il y a eu une sacrée inondation, donc faites attention|mais faites attention, l'école a subit une inondation}.
    ~Topics += Inondation
    ~EcoleOpen = true
    La grille qui donne sur la cour de l'école grince sur ses gonds. # didascalie
    – Je vous fais confiance pour refermer, je dois vous laisser je dois me préparer pour le bal de ce soir.
    Roberto le ténor s'éloigne. #didascalie
    ~MoveTo(Tenor, SalleBal)
}

{LocationOf(Me) == Bar and PreviousLocation == ArriereBar:
    - { LocationOf(Barmaid) == ArriereBar:
        - La serveuse vous suit et referme la porte derrière vous. #didascalie
        - else: La serveuse vous fait signe de fermer la porte. #didascalie
    }
    ~ MoveTo(Barmaid, Bar)
}

VAR stolenCleChatelain = false
{LocationOf(Me) == LocationOf(Chatelain):
    {not stolenCleChatelain:
    - 
        \ Vous remarquez une clé autour du cou du Châtelain. #didascalie
        ~ Topics += CleChatelain
    -else: 
        \ Vous remarquez que la clé a disparu du cou du Châtelain. # didascalie
    }
}

{LocationOf(Me) == LocationOf(Tenor) and stolenCleChatelain:
    \ Roberto vous tend la clé. #didascalie
    – Faites ce que vous avez à faire, je m'éclipse !
    ~ BalLocations -= BalTenor
    ~MoveTo(Tenor, NoWhere)
    ~MoveTo(Me, SalleBal)
    ~ChatelainOpen = true
}

{LocationOf(Me) == Ecole and BipToxicBarrels:
    Dans votre poche le bip trouvé dans le bureau du Châtelain se met à vibrer.
    * Déclencher le bip #investigate
        -> ouverture_caveau
}

-
->->
