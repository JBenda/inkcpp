LIST Characters = (Me), Maire, Comtesse, Pretre, Barmaid, Tenor, Chatelain

VAR LocMaire = (BureauMaire)
VAR LocComtesse = (AncienClocher)
VAR LocPretre = ()
VAR LocBarmaid = (Bar)
VAR LocTenor = (Eglise)
VAR LocChatelain = (SalleBal)

=== function LocationOf(Character)
{Character:
- Me:
    ~return CurrentLocation
- Maire:
    ~return LocMaire
- Comtesse:
    ~return LocComtesse
- Pretre:
    ~return LocPretre
- Barmaid:
    ~return LocBarmaid
- Tenor:
    ~return LocTenor
- Chatelain:
    ~return LocChatelain
}

=== function MoveTo(Character, Location)
{Character:
- Me:
    ~ PreviousLocation = CurrentLocation
    ~ CurrentLocation = Location
- Maire:
    ~ LocMaire = Location
- Comtesse:
    ~ LocComtesse = Location
- Pretre:
    ~ LocPretre = Location
- Barmaid:
    ~ LocBarmaid = Location
- Tenor:
    ~ LocTenor = Location
- Chatelain:
    ~ LocChatelain = Location
}

=== function CharactersIn(Location)
~temp characters = ()
{Location ? LocationOf(Me): 
    ~characters+=Me
}
{Location ? LocationOf(Maire): 
    ~characters+=Maire
}
{Location ? LocationOf(Comtesse): 
    ~characters+=Comtesse
}
{Location ? LocationOf(Pretre): 
    ~characters+=Pretre
}
{Location ? LocationOf(Barmaid): 
    ~characters+=Barmaid
}
{Location ? LocationOf(Tenor): 
    ~characters+=Tenor
}
{Location ? LocationOf(Chatelain): 
    ~characters+=Chatelain
}
~return characters
    
