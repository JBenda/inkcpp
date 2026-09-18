LIST Items = A, B

-> main

=== main
~ temp Exits = (B)
<- listExits(Exits, -> after)
-> DONE

=== after
Arrived.
-> END

=== listExits(list, ->next)
    ~ temp current_ctc = LIST_MIN(list)
    { LIST_COUNT(list) > 0 :
        <- listExits(list - current_ctc, next)
        + [Vers {current_ctc}... #exit]
            -> next
    }
