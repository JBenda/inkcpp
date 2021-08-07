[##](##) general construction:

+ A list is a bit vector!
+ Further we will call each bit vector which represents a list a _flag_.
+ Each flag is tide to a number, which is equal to the  position in the bit vector, which we will call _flag value_
+ Each flag has a string representation, further be called _key_.
+ Each multi list is associated to zero ore more list types
	+ when adding a new element the list type of that element get added to associated lists
	+ when removing the last element of an list type then the type get removed from associated lists
		+ __except:__ the list of associated types will be empty, than nothing happen
		+ -> only an empty initialized list has no d list
+ each list key will be an element variable at global scope -> no two keys can have the same name
+ a list_type is a variable defined with `LIST` therefore mentioned in the `declList` section of the InkJson

## List operators

+ `+(list lh,list rh)` lh = lh ∪ rh
+ `+(list, fag)` add the element to the list
+ `L^(list lh, flag rh)` (interesection) creates an empty list or a list with rh set if it's part of lh
+ `L^(list lh, list rh)` (interesection) = lh ∩ rh
+ `-(list lh, list rh)` remove elements lh = lh / (lh ∩ rh)
+ `-(list lh, flag rh)` removes element from list
+ `+(list l, int i)` l << i (bits get shifted out)
+ `-(list l, int i)` l >> i (bits get shifted out)
+ `LIST_COUNT(list l)` returns the number of flags sets
+ `LIST_MIN(list l)` returns the flag with the lowest numerical representation which is set
+ `LIST_MAX(list l)` return the flag with the highest numerical representation which is set
+ `lrnd(list l)` return a random flag which is set
	+ for the user it is `LIST_RANDOM`, but in InkJson it is called `lrnd`
+ `LIST_ALL(list l)` returns a list which all elements of associated list of l
+ `LIST_INVERT(list l)` returns a list which each flag is toggeld of all associated lists
+ `<(list lh, list rh)` return true if `LIST_MAX(lh) < LIST_MIN(rh)`
+ `>(list lh, list rh)` returns true if `LIST_MIN(lh) > LIST_MAX(rh)`
+ `==(list lh, list rh)` returns true if the setted flags of both are equal
+ `!=(list lh, list rh)` return true if `lh == rh` returns false
+ `>=(list lh, list rh)` returns true if `LIST_MAX(lh) >= LIST_MAX(rh) && LIST_MIN(lh) >= LIST_MIN(rh)`
+ `<=(list lh, list rh)` returns true if `LIST_MAX(lh) <= LIST_MAX(rh) && LIST_MIN(lh) <= LIST_MIN(rh)`
+ `?(list lh, entry rh)` returns true if rh is element of lh
+ `?(list lh, list rh)` returns true if every flag of rh is set in lh
+ `!?(list lh, entry rh)` returns `not ?(lh,rh)`
+ `!?(list lh, list rh)` returns `not ? (lh,rh)`

## Datatype

+ Each list has a id(`listId`), this mapping is arbitrary. To allow usage as array index we start with 0 an increased it for each list_type
+ A flag is identified by `flagId`, it's correspond to the flag value. (flagId - min flagId of that list = flag value)

### list_element

`uint32_t` with bit[0-15] are the listId and bit[16-31] the flag value.

### list

`uint32_t` position in list_table

### list_table

Datatype to manage lists (similar to string_table)

It contains:

+ a reference to the string_table
+ A array `keys` which maps each flagId to an key, who lives in string_table
+ A array `flag_start` which maps each listId to an offset, which leads to value 0 for that list in the entry bitmap.
+ A array `fids` which maps each listId to the flagId for the value 0 of that list (used to get key from keys).
+ A array of entries, each entry contains:
	+ a bitmap, where 1 represent that the list is associated to the corresponding list
	+ a bitmap, where 1 represent that the list contains the flag
	+ the two bitmaps are WORD align for better memory access
