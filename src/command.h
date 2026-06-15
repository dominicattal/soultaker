#ifndef COMMAND_H
#define COMMAND_H

// parses command, executes it, then returns string on the heap (so it should be freed using string_free)
// this should run on the game/gui thread. as such, commands that
// talk to window may be sent as an event instead
char* command_parse(char* command);

/*
One of the thing that annoyed me about the Quake2 source code was the lack of good documentation for it, so im gonna be sure to make it good here :)

map [map_name]
> load selected map

toggle [flag]
> toggle a flag, which can be one of
    spatial_hash_lines
    debug
    camera_lock

set [var_name] [arg1] [arg2] ... 
> set a variable. different types require different args
> var_name can be one of
    camera_target: vec2
    tps: i32

pause
> pause the game

resume
> resume the game

*/

#endif
