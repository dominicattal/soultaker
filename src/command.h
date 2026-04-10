#ifndef COMMAND_H
#define COMMAND_H

// parses command, executes it, then returns string on the heap (so it should be freed using string_free)
// this should run on the game/gui thread. as such, commands that
// talk to window may be sent as an event instead
char* command_parse(char* command);

/*
One of the thing that annoyed me about the Quake2 source code was the lack of good documentation for it, so im gonna be sure to make it good here :)

*/

#endif
