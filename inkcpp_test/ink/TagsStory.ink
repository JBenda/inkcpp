# global_tag
-> global_tags_only
===global_tags_only
First line has global tags only
Second line has one tag # tagged
# tag next line
# more tags
Third line has two tags
# above
Fourth line has three tags # side # across
->start
===start
# knot_tag_start
# second_knot_tag_start #third_knot_tag

Hello # output_tag_h
Second line has no tags
* a
* [b # choice_tag_b # choice_tag_b_2] -> knot2.sub
- World! # output_tag_w
* c # choice_tag_c
* d # choice_tag_d
- ->END

=== Test
Knot2 # output_tag_k
->->

===knot2
= sub
# knot_tag_2
-> Test ->
* e
* f #shared_tag # shared_tag_2 [ with detail #choice_tag #choice_tag_2] and content # content_tag # content_tag_2 
* g # choice_tag_g
- out # close_tag
->END
