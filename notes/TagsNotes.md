Tags are accessible via runner functions:

- `bool has_tags()` are there any tags now?
- `size_t num_tags()` how many tags are there right now?
- `const char* get_tag(size_t i)` get string value of tag

Tags are acquired until a choice appears. After choosing the acquired tags will
be cleared.
```
# Tag1
# Tag 2
Some Text # Tag3
* A # Tag 4
* B # Tag 5
- out # Tag6
```

Will produce:
> Some Text: Tag1, Tag2, Tag3
> * A
> * B
> <1
> A
> out: Tag4, Tag6
