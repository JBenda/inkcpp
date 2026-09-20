== ConversationMaire(->flow, ->trigger)
VAR vuTiroir = false
VAR knowMaireHasCalisson = false
-(talk_to_maire)

*{Topics ? DisparitionComtesse} (au maire) – Savez-vous où est la Comtesse ?
    – Ne m'en parlez pas ! Ça fait 48h qu'elle a disparu alors que ce soir une soirée est donnée en son honneur au Chateau de Peyrian ! 
    ->trigger
    
*{Topics ? OuEstVieuxClocher}(au maire) – Savez-vous où est le Vieux Clocher ?
    – Je n'en ai pas la moindre idée...
    ->trigger

* {vuTiroir and LocationOf(Me) == BureauMaire} (au maire) – Qu'est-ce que vous mangez ?
    – Ce sont des calissons au Melon de Cadour, dîtes mois si vous  voulez gouter !
    ~ knowMaireHasCalisson = true
    ->trigger
    
* {knowMaireHasCalisson} (au maire) – Je veux bien gouter un de vos calissons, finalement !
    – Avez plaisir ! Tenez, je vous en donne même une petite boîte. Ça m'évitera de me goinfrer.
    Le Maire sort une petite boîte en métal en forme losange de {LocationOf(Maire) == BureauMaire: son tiroir|sa poche} et vous la tend. #didascalie
    ~hasCalissons = true
    -> trigger

* {Characters ? Comtesse} (au maire) – J'ai retrouvé la Comtesse [!] , elle est au Vieux Clocher !
    – Quoi ! Attendez moi dehors, j'y cours !
    Sans plus attendre, le Maire vous fait sortir de son bureau et s'éloigne. #didascalie
    Vous êtes dans le hall de la Mairie. La porte du bureau du Maire est entrouverte, dans la précipitation il a oublié de la verrouiller. #didascalie
    ~MoveTo(Me, Mairie)
    ~MoveTo(Maire, AncienClocher)
    ->trigger
    
* {Topics ? (CentreAquatique, InondationSuspecte)} (au maire) – Je sais ce que vous avez fait !
    -> confront_maire
    
* {Topics ? CentreAquatique and Moment == Soir} – Je vais tout révéler !
    – Faites faites, je suis déjà passé avant vous et, l'idée est plutôt reçue positivement !
    ->trigger

* {Topics ? ToxicBarrels} – Et que dîtes-vous de ça Monsieur de le Maire ?
    – Mais qu'est-ce que ... ?
    ->final_reveal

-
->flow
->DONE


== ConversationComtesse(->flow, ->trigger)

*\ (à la jeune femme blonde) – Comtesse ?
    -> meet_the_comtesse
*\ (à la jeune femme blonde) – Ludivine ?
    -> meet_the_comtesse
->DONE


== ConversationPretre(->flow, ->trigger)

~Topics -= OuEstPretre

~temp CanTalkToPretre = (LocationOf(Me) == Bar and offered_calissons) or (LocationOf(Me) != Bar)


*(offered_calissons){hasCalissons and ConversationBarmaid.ask_appeler}(au prêtre) – Mon père, j'ai ici quelques calissons...
    L'homme se tourne vers vous et attrape la petite boîte losange que vous lui tendez. Il a le sourire d'un enfant. # didascalie
    ~Topics -= ParlerPretre
    ->trigger


+{not CanTalkToPretre} (au prêtre) – Excusez-moi mon père ?
    {L'homme détourne la tête et d'un geste de la main vous signifie que vous l'importunez. |– Inutile d'insister, j'aime être seul|–Cessez !|L'homme vous ignore complètement.} #didascalie
    ~Topics += ParlerPretre
    ->trigger

*{CanTalkToPretre and (Topics ? OuEstVieuxClocher)}(au prêtre) – Vous pourriez m'indiquer où se trouve le Vieux Clocher ?
    – Bien sûr, votre générosité mérite récompense, ainsi que le dit la Parole de Dieu notre Seigneur.
    Il sort un calepin de sa soutane et au crayon griffone un plan. #didascalie
    ~knowRouteToAncienClocher = true
    – Prenez la route qui sort du village, celle qui est bordée de platanes. Et avec ceci vous devriez trouver votre chemin sans problème. Maintenant, si vous le voulez bien, j'ai à faire.
    Le prêtre sort. #didascalie
    ~MoveTo(Pretre, Eglise)
    ~Topics -= OuEstVieuxClocher
    ->trigger
    
* {Topics ? CentreAquatique and Moment == Soir} – Que pensez-vous du projet de centre aquatique ?
    – Si l'école publique ferme, je réfléchirai à ouvrir une école privée dans le presbytère. C'est du travail, mais c'est important que les petits du village puisse avoir une éducation près de chez eux.
    ->trigger
    
* {Topics ? ToxicBarrels} – Regardez ces photos que ce je viens de prendre Monsieur le Curé...
    – Bonté divine, qu'est-ce donc ?
    ->final_reveal

-
->flow
->DONE


== ConversationBarmaid(->flow, ->trigger)

VAR servi = false

+(ask_servir){Characters hasnt Barmaid} (à la serveuse) – Qui êtes vous ?
    – Moi c'est Virginie, je peux vous servir quelque chose ?
    ~Characters += Barmaid
    ->trigger
    
+{seen_recently(->ask_servir) and not servi} (à la serveuse) – Volontiers, un pastis s'il vous plaît ?
    – Vous venez d'ailleurs vous pour demander un pastis. Mais j'ai ça. Voilà pour vous.
    ~servi = true
    ->trigger
    
+{ask_servir and not servi} (à la serveuse) – Ça ira... Un verre d'eau peut-être [?] il fait chaud dehors
    – C'est sûr qu'il y a un sacré cagnard. Bah hésitez pas si vous avez besoin de quelqu'chose !
    ~servi = true
    ->trigger
    
*{Topics ? OuEstVieuxClocher} (à la serveuse) – Vous sauriez où est le Vieux Clocher ?
    – Je m'intéresse pas trop à ces vieilleries moi. C'est pas l'Église ? Elle est assez vieille. Attendez, le vieux Prêtre il doit savoir aussi.
    ~Topics+=OuEstPretre
    ->trigger
    
*(ask_appeler){Topics ? OuEstPretre} (à la serveuse) – Vous sauriez où est le Prêtre ?
    – Il est souvent là c'est vrai ! Mais là non. Je lui passe un coup de fil, il sera là dans un moment. Par contre, quand il est ici, il aime pas parler, donc préparez une contrepartie !
    ->trigger

+{Topics ? ParlerPretre and Topics !? Calissons} (à la serveuse) – Le prêtre ne veut pas me parler...    
    – Je vous l'ai dit, quand il est ici, il ne veut parler à personne sans une bonne raison !
    -> trigger

+{Topics ? ParlerPretre and Topics ? Calissons} (à la serveuse) – Vous pensez que des calissons[...] seraient une bonne raison de me parler ?
    – Je ne sais pas, je ne connais pas ses goûts !
    {LocationOf(Pretre) == LocationOf(Barmaid): 
        - Du coin de l'œil vous sentez que vous avez brièvement capté l'attention du prêtre. #didascalie
    }

    -> trigger

*{Topics ? Inondation} (à la serveuse) – Vous avez entendu parler de l'inondation de l'école ?
    – C'est arrivé la semaine passée oui. J'étais chez ma mère avec ma fille. Quand je suis revenue, ils nous ont dit que le centre de loisirs était fermé, alors j'ai renvoyé la petite chez sa grand-mère.
    ->trigger
*{Topics ? Inondation and knowArriereBar and Locations !? ArriereBar} (à la serveuse) – Votre arrière boutique donne sur le préau de l'école ?
    – Tout à fait. Vous voulez jeter un œil ?
    La serveuse vous fait passer derrière le bar et ouvre une porte vers l'extérieur. #discalie
    ~ MoveTo(Me, ArriereBar)
    ~ Locations += ArriereBar
    ~ MoveTo(Barmaid, ArriereBar)
    ~ describe(ArriereBar)
    ->trigger

+{Topics ? Inondation and knowArriereBar and Locations ? ArriereBar and LocationOf(Barmaid) == Bar} (à la serveuse) – Ça vous ennuie si je retourne voir l'arrière boutique ?
    – Pas de souci, allez-y.
    ~ MoveTo(Me, ArriereBar)
    ~ describe(ArriereBar)
    ->trigger
    
* {Topics ? CentreAquatique and Moment == Soir} – Vous avez entendu parler du projet de centre aquatique ?
    – Le Maire m'en parlait il y a quelques minutes, c'est une super initiave. C'est vrai que ça va me compliquer un peu la vie avec la petite, l'école sera plus à côté, mais ça va me rapporter plein de clients !
    ->trigger
* {Topics ? InondationSuspecte and Moment == Soir} – Vous ne trouvez pas ça bizarre que votre bar n'ait pas été inondé ?
    – La météo moi j'y comprends rien de toute manière !
    ->trigger


* {Topics ? ToxicBarrels} – Regardez ce que j'ai trouvé dans l'école ?
    – Boudu mais ce sont ... ?
    ->final_reveal

-
->flow
->DONE



=== ConversationTenor(->flow, ->trigger)

~temp adresseTenor = "{Characters has Tenor:au ténor|à l'homme}"

*(vieuxclocher1) {Topics ? OuEstVieuxClocher and LocationOf(Me) == Eglise} ({adresseTenor}) – C'est bien ici le Vieux Clocher ?
    – Ah non du tout !
    ->trigger
    
*(vieuxclocher2) {Topics ? OuEstVieuxClocher and vieuxclocher1} ({adresseTenor}) – Et vous sauriez où est le Vieux Clocher ?
    – Moi non, mais vous pourriez demander au Prêtre, il en sait en rayon sur le coin, c'est le plus vieil habitant du village.
    ~Topics+=OuEstPretre
    ->trigger
    
*{Topics ? OuEstPretre and not ConversationBarmaid.ask_appeler} ({adresseTenor}) – Et vous sauriez où est le Prêtre ?
    – Justement, ça fait bientôt une heure que je l'attends. Il doit être en train de boire un coup au bar.
    ->trigger
    
+(related_to_pretre){Characters hasnt Tenor} ({adresseTenor}) – Qui êtes vous ?
    – Je m'appelle Roberto, j'anime la chorale de l'Église pour Monsieur le Curé et je donne des cours de chant à l'école. Mais je suis avant tout ténor à l'Opéra de Toulouse.
    ~Characters += Tenor
    ->trigger
    
+{related_to_pretre and Topics ? ParlerPretre} ({adresseTenor}) – Si vous cotoyez souvent Monsieur le Curé [...], vous savez peut-être ce qui pourrait lui délier la langue ?
    – Haha, il ne veut pas vous parler ? Apportez-lui des calissons, c'est son pêché mignon !
    ~Topics += Calissons
    ->trigger
    
*{related_to_pretre and Topics ? ComplotEcole and not triedGrille} ({adresseTenor}) – Vous avez mentionné que vous enseignez à l'école [...] ?
    — Oui mais elle est fermée pour le moment, il y a eu une sacrée inondation.
    ~ Topics += Inondation
    ->trigger

*{related_to_pretre and Topics ? ComplotEcole and triedGrille} ({adresseTenor}) – Avez-vous un moyen d'accéder à l'école [?], la grille semble fermée.
    – Bien sûr, j'ai la clé ! Je peux vous ouvrir si vous voulez, rejoignez moi là-bas.
    Roberto le Ténor se lève et quitte le bar. #didascalie
    ~ MoveTo(Tenor, Mairie)
    ->trigger

* {Topics ? CentreAquatique and Moment == Soir} – Vous avez entendu parler du projet de centre aquatique ?
    – Je viens de l'apprendre. C'est terrible cette destruction de l'école... J'aimais tant leur apprendre le chant à ces petits chérubins !
    ->trigger
* {Topics ? InondationSuspecte and Moment == Soir} – L'inondation de l'école me paraît très suspecte...
    – Oh ! Si vous avez besoin de moi, n'hésitez pas ! En attendant, je prépare mon tour de magie.
    ~Topics+=MagicTrick
    ->trigger
*(disparition) {Topics ? MagicTrick and Moment == Soir} – Vous savez faire de la magie ?
    – Ma spécialité, ce sont les disparitions d'objets !
    ->trigger
    
* (promesse){Topics ? MagicTrick and Topics ? CleChatelain and disparition and Moment == Soir} – Vous sauriez subtiliser la clé que le Châtelain a autour du cou ?
    – Pas en pleine lumière, mais si vous trouvez le moyen de nous plonger brièvement dans l'obscurité, je vous récupère ça !
    ->trigger

-
->flow
->DONE


== ConversationChatelain(->flow, ->trigger)
* {Topics ? ToxicBarrels} – Monsieur le Châtelain, qu'est-ce que vous cachez ?
    – De quoi parlez-vous ?
    – De ce que je viens de trouver caché dans l'école...
    ->final_reveal
->DONE
