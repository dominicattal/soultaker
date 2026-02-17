#include <stdlib.h>
#include <string.h>
#include "trie.h"
#include "log.h"
#include "malloc.h"

static TrieNode* tnode_create(void)
{
    return st_calloc(1, sizeof(TrieNode));
}

static void tnode_destroy(TrieNode* node)
{
    if (node == NULL) 
        return;
    for (i32 i = 0; i < 16; i++)
        tnode_destroy(node->children[i]);
    st_free(node);
}

Trie* trie_create(void)
{
    return st_calloc(1, sizeof(Trie));
}

void trie_destroy(Trie* trie)
{
    tnode_destroy(trie->root);
    st_free(trie);
}

bool trie_contains(Trie* trie, const char* key)
{
    TrieNode* node;
    i32 n, i, c;
    node = trie->root;
    if (node == NULL)
        return false;
    n = strlen(key);
    for (i = 0, c = key[i]; i < n; i++, c = key[i])
        if ((node = node->children[c/16]) == NULL || (node = node->children[c%16]) == NULL)
            return false;
    return node->data != NULL;
}

void* trie_get(Trie* trie, const char* key)
{
    TrieNode* node;
    i32 n, i, c;
    node = trie->root;
    if (node == NULL)
        return NULL;
    n = strlen(key);
    for (i = 0, c = key[i]; i < n; i++, c = key[i])
        if ((node = node->children[c/16]) == NULL || (node = node->children[c%16]) == NULL)
            return NULL;
    return node->data;
}

void trie_insert(Trie* trie, const char* key, void* data)
{
    TrieNode* node;
    i32 n, i, c, d1, d2;
    if (trie->root == NULL)
        trie->root = tnode_create();
    node = trie->root;
    n = strlen(key);
    for (i = 0, c = key[0]; i < n; i++, c = key[i]) {
        d1 = c / 16, d2 = c % 16;
        if (node->children[d1] == NULL)
            node->children[d1] = tnode_create();
        node = node->children[d1];
        if (node->children[d2] == NULL)
            node->children[d2] = tnode_create();
        node = node->children[d2];
    }
    if (node->data != NULL) {
        log_write(WARNING, "Inserting key %s failed: data exists", key);
        return;
    }
    node->data = data;
}
