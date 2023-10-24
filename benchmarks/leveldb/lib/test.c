// Taken from https://github.com/emnl/leveldb-c-example/blob/master/leveldb_example.c
// Modfiied to use the concord-leveldb wrapper

#include <stdio.h>
#include <leveldb/c.h> 
#include "concord-leveldb.h"
#include "concord.h"
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <x86intrin.h>
#include <time.h>
#include <sys/time.h>

int preempt_array[1000000];
int preempt_iter = 0;

#define NUM_KEYS    15000
#define NUM_TRIALS    100

__thread int concord_preempt_now;
__thread uint64_t concord_preempt_after_cycle = 5000 * 3.3;
__thread uint64_t concord_start_time;

void concord_disable()
{
    concord_preempt_now = -1;
}

void concord_enable()
{
    concord_preempt_now = 0;
}


static inline uint64_t get_ns()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_nsec;
}

void concord_func()
{
    preempt_array[preempt_iter++] = get_ns();
    concord_preempt_now = 0;
}

void concord_rdtsc_func()
{
  concord_start_time = __rdtsc();
}


int main()
{
    leveldb_t *db;
    leveldb_options_t *options;
    leveldb_readoptions_t *roptions;
    leveldb_writeoptions_t *woptions;
    char *err = NULL;
    char *read;
    size_t read_len;

    /******************************************/
    /* OPEN */

    options = leveldb_options_create();
    leveldb_options_set_create_if_missing(options, 1);
    db = leveldb_open(options, "testdb", &err);

    if (err != NULL) {
      fprintf(stderr, "Open fail.\n");
      return(1);
    }

    /* reset error var */
    leveldb_free(err); err = NULL;

    /******************************************/
    /* WRITE */

    woptions = leveldb_writeoptions_create();
    
    cncrd_leveldb_put(db, woptions, "key", 3, "value", 5, &err);


    for (size_t i = 0; i < NUM_KEYS; i++)
    {
      char key[100];
      sprintf(key, "keyz%zu", i);
      cncrd_leveldb_put(db, woptions, key, 8, key, 8, &err);
      /* code */
    }
    

    if (err != NULL) {
      fprintf(stderr, "Write fail.\n");
      return(1);
    }

    leveldb_free(err); err = NULL;


    roptions = leveldb_readoptions_create();
    read = cncrd_leveldb_get(db, roptions, "key", 3, &read_len, &err);

    if (err != NULL) {
      fprintf(stderr, "Read fail.\n");
      return(1);
    }

    assert(strncmp(read, "value", 5) == 0);
    printf("Assert success || read: %s\n", read);   


    
    unsigned long long total_time = 0;
    
    for (int k = 0; k < NUM_TRIALS; k++)
    {
      char test_key[100];
      sprintf(test_key,"keyz%zu",rand() % NUM_KEYS);
      int res = 0;
      unsigned long long before = get_ns();
      // res = simpleloop(8100*5);
      cncrd_leveldb_get(db, roptions, test_key, 8, &read_len, &err);
      // cncrd_leveldb_scan(db,roptions, 'musa');
      unsigned long long after = get_ns();

      total_time += after - before;
    }

    if (err != NULL) {
      fprintf(stderr, "Read fail.\n");
      return(1);
    }
    printf("%llu", (total_time / NUM_TRIALS));
    printf("\nFinished\n");


    leveldb_close(db);
    leveldb_destroy_db(options, "testdb", &err);
    leveldb_free(err); err = NULL;


    return(0);
}