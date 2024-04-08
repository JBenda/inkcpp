LIST list = (A=2),B,(C=5),Z=-1
{list}
{list: yes}
{not list}
{list and true}
{not list or true}
{true or not list}
{true and list}
VAR x = A
~ x += B
// expect  list "A, B"
// expect list "A"
~ x -= 1
{x}
// expect list "B", since A got moved out
~ x += 1
{x}
VAR y = A
~ y = list(3)
>{y}
~ y = list(4)
// list with no named element
>{y} {y+1}
> {LIST_ALL(y)}
// list element out of range
~ y = list(1)
>{y} {y+1}
~ y = list(6)
>{y} {y-1}
{LIST_RANGE(LIST_ALL(y), 3, 5)}
~ y = list(5)
>{y} >{y+1} >{(y+1)-1} >{(y-1)+1}
>{list} >{list+1} >{(list+1)+1}
~list += 1
>{list}
~list += 1
>{list}
~list += 1
>{list} >{list(-1)} >{list(-1) + 3} >{list(-1)+2+1}
Hey
