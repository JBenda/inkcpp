# Container Metadata

Even though the inkcpp binary compiler flattens the hierarchical JSON structure into a flat sequence of commands with diverts, we still need to maintain some metadata about the container structure so we can properly record visit counts.

## Container Instructions Layout

Only containers which have visit or turn tracking flags are recorded in any way. All other containers may as well no longer exist.

The first command at the start of any container is a {{START_CONTAINER_MARKER}}.

Inline child container are written in-place as the interpreter should enter them normally when it reaches them.

Named containers specified in a container's meta-data are compiled in order after we finish writing out all commands for the parent container.
* Before writing any of them, we write a {{DIVERT}} command which jumps to {{END_CONTAINER_MARKER}} for the parent below. This way we don't wander into these child containers when we run out of content in our own container.
* After writing each named child, we write a similar {{DIVERT}} as above. This way, when any of these containers finish, the interpreter does not wander into their siblings.
* Both these diverts are written with a special {{DIVERT_IS_FALLTHROUGH}} flag. Its important will be clear later.

After writing all these named children, the last command, a {{END_CONTAINER_MARKER}} is written.

## Container Start/End Instruction

{{START_CONTAINER_MARKER}} and {{END_CONTAINER_MARKER}} both have a {{container_t}} argument to uniquely identify the container starting or ending. These numbers have no relation to the original container names. They are assigned sequentially starting from 0 to each container which requires tracking.

The {{START}} command pushes that container ID onto the current container stack. {{END}} pops them.

TODO: Notes on implied DONE and the done IP cache.

## Container Map

A "container map" is included in the ink binary metadata before any instructions. It's an array in the following format:

32-bit instruction offset (offset_t)
32-bit container id (container_t)

The array is terminated with a 0xFFFFFFFF.

Each container has two entries in the array: one for its start and one for its end. There is no distinction between start and end in the array.

The "start" offset points to the instruction just after the {{START_CONTAINER_MARKER}} command. If you were to jump to it, you'd avoid hitting the {{START_CONTAINER_MARKER}} next step.

The "end" offset points to the instruction just after the {{END_CONTAINER_MARKER}} instruction. If you were to jump to it, you'd avoid hitting the {{END_CONTAINER_MARKER}} next step.

The container map is used for handling visit counts that result from an interpreter jump (caused by a {{DIVERT}} or selecting a choice). In normal execution, the {{START_CONTAINER_MARKER}} and {{END_CONTAINER_MARKER}} are pushing/popping container indicies and updating visit counts. If we jump, we're passing over all these commands. We need a way of both updating the visit counts (for each new container we enter) and updating our own container stack so the interpreter knows where it is in the container hierarchy.

### Jump Algorithm

1. Determine if we are jumping forward (dest > ip) or backward (ip < dest)
2. Linear search the container map, starting from the beginning for a forward jump or the end for a backward jump
	* Find the entry just "before" the current pointer. 


### Special: "Falling Through" Diverts

TODO: Can we simplify jump algorithm here? Are there certain guarantees?