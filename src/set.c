#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "set.h"

// source: https://stackoverflow.com/questions/664014/what-integer-hash-function-are-good-that-accepts-an-integer-hash-key/12996028#12996028
// I just needed some hash function that seemed good. I don't know how this will affect performance,
// but it does not matter for now.
static uint64_t hash_default(uint64_t x) {
    x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
    x = x ^ (x >> 31);
    return x;
}

static inline uint64_t hash(const Set *set, uint64_t key);
static inline int find_slot(const Set *set, uint64_t key, uint64_t *out_idx);
static inline int lookup(const Set *set, uint64_t key, uint64_t *out_idx);
static inline void resize(Set *set);
static inline void rehash(Set *set);

int set_init(Set *set, size_t init_cap, uint64_t (*hash_func)(uint64_t)) {
    Entry *buf = calloc(init_cap, sizeof(Entry));
    if (!buf) {
        perror("set_init");
        abort();
    }
    // marks all as free...
    set->hash = hash_func ? hash_func : hash_default;
    set->entries = buf;
    set->size = 0;
    set->capacity = init_cap;
    set->load_factor = 0.75;
    return 0;
}

void set_uninit(Set *set) {
    set->hash = NULL;
    free(set->entries);
    set->entries = NULL;
    set->size = 0;
    set->capacity = 0;
    set->load_factor = 0;
}

int set_put(Set *set, uint64_t key) {
    assert(set->size <= set->capacity && "Size has outgrown the capacity");
    if (set->size >= set->capacity * set->load_factor) {
        resize(set);
        rehash(set);
    }
    uint64_t idx = 0;
    int found_slot = find_slot(set, key, &idx);
    // don't replace. value is the same as key, so there is no use in replacing it.
    // just return
    if (found_slot && set->entries[idx].free == S_FREE) {
        set->entries[idx].keyval = key;
        set->entries[idx].free = S_OCCUPIED;
        set->entries[idx].used = S_USED;
        set->size++;
        return 1;
    } else {
        return 0;
    }
}

int set_remove(Set *set, uint64_t key) {
    if (set->size == 0) {
        return 0;
    }
    uint64_t idx = 0;
    int found_slot = lookup(set, key, &idx);
    if (found_slot && set->entries[idx].free == S_OCCUPIED && set->entries[idx].keyval == key) {
        set->entries[idx].keyval = 0;
        set->entries[idx].free = S_FREE;
        set->size--;
        return 1;
    } else {
        return 0;
    }
}

void set_clear(Set *set) {
    memset(set->entries, S_FREE, sizeof(Entry) * set->capacity);
    set->size = 0;
}

int set_contains(const Set *set, uint64_t key) {
    uint64_t idx = 0;
    int found_slot = lookup(set, key, &idx);
    return found_slot && set->entries[idx].free == S_OCCUPIED && set->entries[idx].keyval == key;
}

Entry *set_entries(const Set *set) {
    return set->entries;
}

size_t set_size(const Set *set) {
    return set->size;
}

size_t set_capacity(const Set *set) {
    return set->capacity;
}

int set_recently_resized(Set *set) {
    int res = set->resized;
    set->resized = 0;
    return res;
}

static inline uint64_t hash(const Set *set, uint64_t key) {
    return set->hash(key) % (uint64_t)set->capacity;
}

static inline int find_slot(const Set *set, uint64_t key, uint64_t *out_idx) {
    uint64_t idx = hash(set, key);
    uint64_t stop = idx;
    int find = 1;
    while (set->entries[idx].free == S_OCCUPIED && set->entries[idx].keyval != key) {
        idx = (idx + 1) % (uint64_t)set->capacity;
        if (idx == stop) {
            find = 0; // set is full
            break;
        }
    }
    *out_idx = idx;
    return find;
}

static inline int lookup(const Set *set, uint64_t key, uint64_t *out_idx) {
    uint64_t idx = hash(set, key);
    uint64_t stop = idx;
    int find = 1;
    while (set->entries[idx].used == S_USED && set->entries[idx].keyval != key) {
        idx = (idx + 1) % (uint64_t)set->capacity;
        if (idx == stop) {
            find = 0; // set is full
            break;
        }
    }
    *out_idx = idx;
    return find;
}

static inline void resize(Set *set) {
    size_t old_cap = set->capacity;
    size_t new_cap = old_cap * (size_t)2;
    Entry *tmp = realloc(set->entries, new_cap * sizeof(Entry));
    if (!tmp) {
        perror("resize");
        abort();
    }
    // in case the resizing factor changes, keep the expression at the end
    // this ensures the other slots are marked as free
    set->entries = tmp;
    set->capacity = new_cap;
    set->resized = 1;
}

static inline void rehash(Set *set) {
    Entry *tmp = malloc(set->capacity * sizeof(Entry));
    if (!tmp) {
        perror("rehash");
        abort();
    }
    memcpy(tmp, set->entries, set->capacity * sizeof(Entry));
    set_clear(set);
    for (size_t i = 0; i < set->capacity; i++) {
        if (tmp[i].free == S_OCCUPIED) {
            set_put(set, tmp[i].keyval);
        }
    }
    free(tmp);
}
