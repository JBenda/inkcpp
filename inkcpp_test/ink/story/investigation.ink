=== InvestigateBureauMaire(->flow, ->trigger)
* {LocationOf(Maire) != BureauMaire} [Ouvrir le tiroir # investigate] Vous ouvrez le tiroir #didascalie.
    Vous trouvez une demi-douzaine de boîtes de calissons. #didascalie
    ->trigger

+ {Topics ? ComplotEcole and LocationOf(Maire) == BureauMaire} [Ouvrir l'armoire d'archives # investigate]
    Vous ne pouvez pas inspecter l'armoire pendant que le Maire est là. #didascalie
    ->trigger
    
* {Topics ? ComplotEcole and LocationOf(Maire) != BureauMaire} [Ouvrir l'armoire d'archives # investigate]
    Vous profitez de l'absence du Maire pour fouiller l'armoire d'archives. #didascalie
    Un document en particulier attire votre attention : un permis de démolir l'école de Mandonez pour faire construire au même emplacement un centre de loisirs aquatiques ! #didascalie
    ~Topics += CentreAquatique
    ->trigger

-
->flow
->DONE


=== InvestigateEcole(->flow, ->trigger)
* {Topics ? ComplotEcole} [Observer l'affaissement # investigate]
    De l'autre côté de ce qui était un préau, vous apercevez la Saves, la petite rivière qui longe Mandonez et qui lui donne son qualificatif, "Savès". # didascalie
    Le cours d'eau est réduit à son strict minimum, pas étonnant en période de sécheresse. #didascalie
    Dans le prolongement du préau, derrière un grillage, vous apercevez des fûts de bière, ce doit être l'arrière boutique du bar. #didascalie
    ~knowArriereBar = true
    -> trigger
-
->flow
->DONE


=== InvestigateArriereBar(->flow, ->trigger)
* {Topics ? Inondation} [Observer l'affaissement de l'école # investigate]
    La différence est saisissante : là où l'école semble avoir été frappée par un cataclysme, avec des dégâts considérables, le bar lui semble avoir été épargné alors que seuls quelques mètres les séparent. #didascalie
    ~Topics -= Inondation
    ~Topics += InondationSuspecte
    
    -> trigger
-
->flow
->DONE

=== InvestigatePlacardElectrique(->flow, ->trigger)
+(cut_light) [Couper le disjoncteur général #investigate]
    Vous pressez {|à nouveau }l'interrupteur général du tableau électrique. Tout le chateau est plongé dans l'obscurité. # didascalie
    Des cris retentissent dans la salle de bal. # didascalie
    Vous entendez des pas qui s'approchent, vous remettez en route la lumière et vous faufilez dans le vestibule. # didascalie
    ~MoveTo(Me, Vestibule)
    {ConversationTenor.promesse:
        Vous espérez que Roberto a eu le temps de faire son "tour de magie". #didascalie
        ~stolenCleChatelain = true
    }
    -> trigger

-
->flow
->DONE

=== InvestigateBureauChatelain(->flow, ->trigger)

* [Observer le plan # investigate] 
    Vous observez le plan, probablement celui du nouveau centre aquatique? #didascalie
    Pas du tout, il s'agit manifestement d'un plan pour une nouvelle école ! On voit nettement la forme des salles de classe. #didascalie
    ->trigger
    
* [Fouiller parmi les documents # investigate] 
    Parmi les documents se trouve la correspondance avec le Maire. Il y a ici assez pour incriminer ce dernier et le Châtelain. #didascalie
    ->trigger
    
* [Chercher dans l'armoire # investigate]
    Au milieu des documents comptables, un bip électronique. Il est un peu sale. Une diode rouge indique qu'il est inactif. #didascalie
    ~BipToxicBarrels = true
    -> trigger
    

-
->flow
->DONE
