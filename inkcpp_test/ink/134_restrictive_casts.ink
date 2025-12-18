VAR b = true    
VAR i = 1
VAR f = 1.0
VAR t = "text"
LIST l = (A), B
VAR d = ->Knot

// Input with initial values
{b} {i} {f} {t} {l} {d}

// Cast during evaluation
{b+0.5} {i+0.5} {f+0.5} {t+0.5} {l+1}

// Cast by variable redefinition
~b = b + 0.5
~i = i + 0.5
~f = f + 0.5
~t = t + 0.5
~l = l + 1
{b} {i} {f} {t} {l}
->DONE

===Knot
->DONE

===Fail0
{l + true}
->DONE

===Fail1
{l + 0.5}
->DONE

===Fail2
{d + 0.5}
->DONE

===Fail3
~l = l + true
{l}
->DONE

===Fail4
~l = l + 0.5
{l}
->DONE

===Fail5
~d = d + 0.5
{d}
->DONE
