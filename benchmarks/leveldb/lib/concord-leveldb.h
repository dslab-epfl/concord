#include <leveldb/c.h>

#ifdef __cplusplus
extern "C" {
#endif

char *cncrd_leveldb_get(leveldb_t *db, const leveldb_readoptions_t *options, const char *key, size_t keylen,
                        size_t *vallen, char **errptr);

void cncrd_leveldb_put(leveldb_t *db, const leveldb_writeoptions_t *options, const char *key, size_t keylen,
                       const char *val, size_t vallen, char **errptr);

void cncrd_leveldb_delete(leveldb_t *db, const leveldb_writeoptions_t *options, const char *key, size_t keylen,
                          char **errptr);

leveldb_iterator_t *cncrd_leveldb_create_iterator(leveldb_t *db, const leveldb_readoptions_t *options);

void cncrd_leveldb_iter_destroy(leveldb_iterator_t *iter);

void cncrd_leveldb_iter_seek_to_first(leveldb_iterator_t *iter);
void cncrd_leveldb_iter_seek_to_last(leveldb_iterator_t *iter);

void cncrd_leveldb_iter_seek(leveldb_iterator_t *iter, const char *k, size_t klen);

void cncrd_leveldb_iter_next(leveldb_iterator_t *iter);
void cncrd_leveldb_iter_prev(leveldb_iterator_t *iter);

int cncrd_leveldb_scan(leveldb_t *db, const leveldb_readoptions_t *roptions, const char *search);

int simpleloop(int k);

#ifdef __cplusplus
}
#endif
