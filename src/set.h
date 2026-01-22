#pragma once

#include <stddef.h>
#include <stdint.h>

#define S_OCCUPIED 1
#define S_USED 1
#define S_UNUSED 0
#define S_FREE 0

typedef struct _entry_s {
    uint64_t keyval;
    int free;
    int used;
} Entry;

// the entry is a 64 bit unsigned integer. return itself
#define S_ENTRY_KEY(entry) ((entry).keyval)
#define S_ENTRY_VALUE(entry) ((entry).keyval)

// this is a dictionary. the key is also the value
typedef struct _set_s {
    Entry *entries;
    size_t capacity;
    size_t size;
    uint64_t (*hash)(uint64_t);
    double load_factor;
    int resized;
} Set;

// hash_func may be null. this tells the implementation to use
// the default hash function for 64 bit integers that is implemented
// by this set (also called hash_default)
int set_init(Set *set, size_t init_cap, uint64_t (*hash_func)(uint64_t));
void set_uninit(Set *set);
int set_put(Set *set, uint64_t key);
int set_remove(Set *set, uint64_t key);
void set_clear(Set *set);
// reduntant? just check if set_put is false?
int set_contains(const Set *set, uint64_t key);
Entry *set_entries(const Set *set);
size_t set_size(const Set *set);
size_t set_capacity(const Set *set);
// Returns true if the set has been resized since the
// last time this function has been called. Returns zero
// otherwise
int set_recently_resized(Set *set);