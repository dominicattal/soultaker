#ifndef TRIE_H
#define TRIE_H

#include <stdbool.h>
#include "type.h"

typedef struct Trie Trie;
typedef struct TrieNode TrieNode;

typedef struct TrieNode {
    TrieNode* children[16];
    void* data; // if data is NULL, terminal
} TrieNode;

typedef struct Trie {
    TrieNode* root;
} Trie;

Trie*   trie_create(void);
void    trie_destroy(Trie* trie);
bool    trie_contains(Trie* trie, const char* key);
void    trie_insert(Trie* trie, const char* key, void* data);
void    trie_inserti(Trie* trie, const char* key, i32 data);
void*   trie_get(Trie* trie, const char* key);
i32     trie_geti(Trie* trie, const char* key);

#endif
