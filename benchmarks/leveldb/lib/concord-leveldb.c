#include "concord-leveldb.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

// The case for shinjuku
#if 0
#define PRE_PROTECTCALL \
    { asm volatile("cli" :::); }
#define POST_PROTECTCALL \
    { asm volatile("sti" :::); }
#else
#define PRE_PROTECTCALL \
    {}
#define POST_PROTECTCALL \
    {}
#endif

char *cncrd_leveldb_get(leveldb_t *db, const leveldb_readoptions_t *options, const char *key, size_t keylen,
                        size_t *vallen, char **errptr) {
    PRE_PROTECTCALL;
    char *ret = leveldb_get(db, options, key, keylen, vallen, errptr);
    POST_PROTECTCALL;
    return ret;
}

void cncrd_leveldb_put(leveldb_t *db, const leveldb_writeoptions_t *options, const char *key, size_t keylen,
                       const char *val, size_t vallen, char **errptr) {
    leveldb_put(db, options, key, keylen, val, vallen, errptr);
}

void cncrd_leveldb_delete(leveldb_t *db, const leveldb_writeoptions_t *options, const char *key, size_t keylen,
                          char **errptr) {
    leveldb_delete(db, options, key, keylen, errptr);
}

leveldb_iterator_t *cncrd_leveldb_create_iterator(leveldb_t *db, const leveldb_readoptions_t *options) {
    return leveldb_create_iterator(db, options);
}

void cncrd_leveldb_iter_destroy(leveldb_iterator_t *iter) { leveldb_iter_destroy(iter); }

void cncrd_leveldb_iter_seek_to_first(leveldb_iterator_t *iter) { leveldb_iter_seek_to_first(iter); }

void cncrd_leveldb_iter_seek_to_last(leveldb_iterator_t *iter) { leveldb_iter_seek_to_last(iter); }

void cncrd_leveldb_iter_seek(leveldb_iterator_t *iter, const char *k, size_t klen) { leveldb_iter_seek(iter, k, klen); }

void cncrd_leveldb_iter_next(leveldb_iterator_t *iter) { leveldb_iter_next(iter); }

void cncrd_leveldb_iter_prev(leveldb_iterator_t *iter) { leveldb_iter_prev(iter); }

// Custom functions for concord-leveldb

// Scan
int cncrd_leveldb_scan(leveldb_t *db, const leveldb_readoptions_t *roptions, const char *search) {
    PRE_PROTECTCALL;
    leveldb_iterator_t *iter = leveldb_create_iterator(db, roptions);
    POST_PROTECTCALL;

    PRE_PROTECTCALL;
    leveldb_iter_seek_to_first(iter);
    POST_PROTECTCALL;

    int i = 0;
    int counter = 0;

    while (1) {
        PRE_PROTECTCALL;
        if (!leveldb_iter_valid(iter)) {
            break;
        }
        POST_PROTECTCALL;

        size_t value_len;

        PRE_PROTECTCALL;
        char *value_ptr = leveldb_iter_key(iter, &value_len);
        POST_PROTECTCALL;
        i++;
        PRE_PROTECTCALL;
        leveldb_iter_next(iter);
        POST_PROTECTCALL;
    }

    PRE_PROTECTCALL;
    leveldb_iter_destroy(iter);
    POST_PROTECTCALL;

    return i;
}

int simpleloop(int k) {
    for (int i = 0; i < k; i++) {
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
    }
    return 1;
}