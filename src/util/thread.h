#ifndef THREAD_H
#define THREAD_H

#include "type.h"

#define NUM_THREADS 4

// Permanently associate a thread with name and
// generate idx for thread in range [0...NUM_THREADS)
void thread_link(const char* name);

// Returns idx associated with name. Returns -1
// if the name is not associated with any thread
i32  thread_get_id(const char* name);

// Returns name associated with id. Returns -1
// if the id is not associated with a thread or if
// id is not in the range [0...NUM_THREADS)
const char* thread_get_name(i32 id);

// Returns the id of the current thread. Returns -1
// if the id is the current thread is not linked
i32  thread_get_self_id(void);

// Returns the name of the current thread. Returns
// "Unrecognized" if the current thread is not linked
const char* thread_get_self_name(void);

#endif
