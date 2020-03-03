In which I figure out what in the nine fucks ink is doing to whitespace so I can emulate it.

There seem to be _numerous_ places in which ink deals with whitespace so I will just document each here and try to figure out what they're doing and why.

# Push To Output Stream

Ink processes whitespace in strings as they are added to the output stream. See _PushToOutputStream_ and _PushToOutputStreamIndividual_ in StoryState.cs.

## PushToOutputStream

Calls _TrySplittingHeadTailWhitespace_ on incoming strings in case they contain newlines. I'm not really sure *how* this could happen since newlines are split off by the Ink compiler already. How could a newline possibly be in an incoming string? I guess maybe if it's stored in a variable? Seems like quite the edge case. 

DECISION: NOT IMPLEMENTING. Will review if the case comes up.

After running this split, the split strings are sent to _PushToOutputStreamIndividual_.

## PushToOutputStreamIndividual

Lots going on here.

First, if the incoming element is glue, trim newlines from the output stream.

ALREADY IMPLEMENTED. See *basic_stream::append(const data&)*.

Otherwise, for text:

1. We trim whitespace at the _beginning_ and _ending_ of function calls (not tunnels). This is made more complicated by glue.

