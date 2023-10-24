#define STRINGIFY(x) #x

#define LOAD_SYMBOL(plugin, symbol)                                                \
    dlerror();                                                                     \
    decltype(&symbol) symbol = (decltype(symbol))dlsym(plugin, STRINGIFY(symbol)); \
    {                                                                              \
        const char *err = NULL;                                                    \
        if ((err = dlerror())) {                                                   \
            std::cerr << "Error loading symbol: " << err << std::endl;             \
            exit(-1);                                                              \
        }                                                                          \
        assert(symbol && "Error loading symbol.");                                 \
    }

char *(*dl_cncrd_leveldb_get)(leveldb_t *db, const leveldb_readoptions_t *options, const char *key, size_t keylen,
                              size_t *vallen, char **errptr);

void (*dl_cncrd_leveldb_put)(leveldb_t *db, const leveldb_writeoptions_t *options, const char *key, size_t keylen,
                             const char *val, size_t vallen, char **errptr);

void (*dl_cncrd_leveldb_delete)(leveldb_t *db, const leveldb_writeoptions_t *options, const char *key, size_t keylen,
                                char **errptr);

leveldb_iterator_t *(*dl_cncrd_leveldb_create_iterator)(leveldb_t *db, const leveldb_readoptions_t *options);

void (*dl_cncrd_leveldb_iter_destroy)(leveldb_iterator_t *iter);

void (*dl_cncrd_leveldb_iter_seek_to_first)(leveldb_iterator_t *iter);
void (*dl_cncrd_leveldb_iter_seek_to_last)(leveldb_iterator_t *iter);

void (*dl_cncrd_leveldb_iter_seek)(leveldb_iterator_t *iter, const char *k, size_t klen);

void (*dl_cncrd_leveldb_iter_next)(leveldb_iterator_t *iter);
void (*dl_cncrd_leveldb_iter_prev)(leveldb_iterator_t *iter);

int (*dl_cncrd_leveldb_scan)(leveldb_t *db, const leveldb_readoptions_t *roptions, const char *search);

int (*dl_simpleloop)(int k);