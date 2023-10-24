// Taken from https://github.com/emnl/leveldb-c-example/blob/master/leveldb_example.c
// Modfiied to use the concord-leveldb wrapper

#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <leveldb/c.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <x86intrin.h>

#include "concord-leveldb.h"
#include "concord.h"
#include "dl-helpers.h"

#define NUM_KEYS 15000

__thread int concord_preempt_now;
__thread uint64_t concord_preempt_after_cycle = 5000 * 3.3;
__thread uint64_t concord_start_time;

void concord_disable() { concord_preempt_now = -1; }

void concord_enable() { concord_preempt_now = 0; }

void concord_func() { concord_preempt_now = 0; }

void concord_rdtsc_func() { concord_start_time = __rdtsc(); }

int main(int argc, char *argv[]) {
    concord_disable();
    assert(argc == 2);
    const char *plugin_file = argv[1];
    printf("Loading plugin: %s\n", plugin_file);
    dlerror();
    char *err = NULL;
    void *plugin = dlopen(plugin_file, RTLD_NOW);
    if ((err = dlerror())) {
        printf("Error loading plugin: %s\n", err);
        exit(-1);
    }
    assert(plugin);

    *(void **)(&dl_cncrd_leveldb_get) = dlsym(plugin, STRINGIFY(cncrd_leveldb_get));
    if ((err = dlerror())) {
        printf("Error loading cncrd_leveldb_get symbol: %s\n", err);
        exit(-1);
    }
    assert(dl_cncrd_leveldb_get);

    leveldb_t *db;
    leveldb_options_t *options;
    leveldb_readoptions_t *roptions;
    leveldb_writeoptions_t *woptions;
    char *read;
    size_t read_len;

    /******************************************/
    /* OPEN */

    concord_enable();
    concord_start_time = __rdtsc();

    options = leveldb_options_create();
    leveldb_options_set_create_if_missing(options, 1);
    db = leveldb_open(options, "testdb", &err);

    if (err != NULL) {
        fprintf(stderr, "Open fail.\n");
        return (1);
    }

    /* reset error var */
    leveldb_free(err);
    err = NULL;

    /******************************************/
    /* WRITE */

    woptions = leveldb_writeoptions_create();

    cncrd_leveldb_put(db, woptions, "key", 3, "value", 5, &err);

    for (size_t i = 0; i < NUM_KEYS; i++) {
        char key[100];
        sprintf(key, "keyz%zu", i);
        cncrd_leveldb_put(db, woptions, key, 8, key, 8, &err);
    }

    if (err != NULL) {
        fprintf(stderr, "Write fail.\n");
        return (1);
    }

    leveldb_free(err);
    err = NULL;

    roptions = leveldb_readoptions_create();
    read = dl_cncrd_leveldb_get(db, roptions, "key", 3, &read_len, &err);

    if (err != NULL) {
        fprintf(stderr, "Read fail.\n");
        return (1);
    }

    assert(strncmp(read, "value", 5) == 0);
    printf("Assert success || read: %s\n", read);

    leveldb_close(db);
    leveldb_destroy_db(options, "testdb", &err);
    leveldb_free(err);
    err = NULL;

    return (0);
}