Operations are everything which works only on the evaluation stack!
(eg. ADD, NOT, EQUAL, etc.)

To use them include `executioner.h`, this will take care of including the other
needed header in the correct order.

The `executioner` need to instantiation time all resources needed for all
supported operations (a string_table for now).

Then you can execute a command with the call operator of the `executioner`.
`void operator()(Command, eval_stack&)`

The executioner then iterates through a compile time created and optimization
list of all types. To determine how many arguments needed
(with `size_t command_num_args(Command)`) and then finds the matching operator
for the value type.

Type casting is solved with a cast matrix, where each entry is defined with:
`template<> constexpr value_type cast<value_type,value_type> = value_type`
`template<> constexpr value_type cast<t1,t2> = resulting_type`

! At the moment we must ensure that `(int)(t1) < (int)(t2)`!

The gain:

* Operation handling can be separated in different files
  (`numeric_operations.h, string_operations.h`). The value class only knows what
  data it contains, and not how to handle it => adding new types or operators
  is know possible without changing many `switch cases`.
* Values are acquired with `get<value_type>()` witch allows changing type
  without breaking unnoticed code.
* Not one huge file.
* The `executioner` knows the resources for the operations
  => no need to pass a string_table to each add.
* everything is without virtualisation => no/negligible runtime overhead.
