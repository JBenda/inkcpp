# global_tag
->start
===start
# knot_tag_start
# second_knot_tag_start

Hello # output_tag_h
* a
* b->knot2 # choice_tag_b
- World! # output_tag_w
* c # choice_tag_c
* d # choice_tag_d
- ->END

===knot2
# knot_tag_2

Knot2 # output_tag_k
* e
* f # choice_tag_f
- out->END # close_tag
