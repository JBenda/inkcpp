# global_tag
->start
===start
# knot_tag_start
# second_knot_tag_start

Hello # output_tag_h
* a
* [b # choice_tag_b # choice_tag_b_2]->knot2
- World! # output_tag_w
* c # choice_tag_c
* d # choice_tag_d
- ->END

===knot2
# knot_tag_2

Knot2 # output_tag_k
* e
* f #shared_tag # shared_tag_2 [ with detail #choice_tag #choice_tag_2] and content # content_tag # content_tag_2 
* g # choice_tag_g
- out # close_tag->END
