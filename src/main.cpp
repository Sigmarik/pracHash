/**
 * @file main.cpp
 * @author Ilya Kudryashov (kudriashov.it@phystech.edu)
 * @brief Hash table test engine.
 * @version 0.1
 * @date 2023-03-14
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <ctype.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <x86intrin.h>

#include "lib/util/dbg/debug.h"
#include "lib/util/argparser.h"
#include "lib/alloc_tracker/alloc_tracker.h"
#include "lib/util/util.h"

#include "utils/config.h"
#include "utils/main_utils.h"

#include "hash/hash_functions.h"
#include "hash/hash_table.hpp"

#include "text_parser/text_parser.h"

#define MAIN

#if OPTIMIZATION_LEVEL >= 1
int simd_comparison_placeholder(__m256i alpha, __m256i beta) { return 0; }
#endif

int main(const int argc, const char** argv) {
    atexit(log_end_program);

    start_local_tracking();
    unsigned int log_threshold = STATUS_REPORTS;
    MAKE_WRAPPER(log_threshold);

    ActionTag line_tags[] = {
        #include "cmd_flags/main_flags.h"
    };
    const int number_of_tags = ARR_SIZE(line_tags);

    parse_args(argc, argv, number_of_tags, line_tags);
    log_init("program_log.html", log_threshold, &errno);
    print_label();

    log_printf(STATUS_REPORTS, "status", "Generating input samples.\n");
    const char* word_list = NULL;
    int alloc_status = posix_memalign((void**)&word_list, 32, TEST_COUNT * MAX_WORD_LENGTH * sizeof(*word_list));
    size_t sample_size = TEST_COUNT;
    _LOG_FAIL_CHECK_(alloc_status == 0, "error", ERROR_REPORTS, return_clean(EXIT_FAILURE), NULL, ENOENT);
    for (char* fill_ptr = (char*) word_list; fill_ptr < word_list + TEST_COUNT * MAX_WORD_LENGTH; ++fill_ptr) {
        *fill_ptr = (char) rand();
    }

    log_printf(STATUS_REPORTS, "status", "Initializing the table.\n");

    HashTable table = {};
    HashTable_ctor(&table, &errno);
    _LOG_FAIL_CHECK_(HashTable_status(&table) == 0, "error", ERROR_REPORTS, {
        log_printf(ERROR_REPORTS, "error", "Table status was %u;\n", HashTable_status(&table));
        return_clean(EXIT_FAILURE);
    }, NULL, ENOMEM);
    track_allocation(table, HashTable_dtor);

    log_printf(STATUS_REPORTS, "status", "Filling table with words.\n");

    for (const char* word_ptr = word_list;
        word_ptr < word_list + sample_size * MAX_WORD_LENGTH;
        word_ptr += MAX_WORD_LENGTH) {

        #if OPTIMIZATION_LEVEL < 1
        HashTable_insert(&table, TESTED_HASH(word_ptr, word_ptr + MAX_WORD_LENGTH), word_ptr, strcmp);
        #else
        HashTable_insert(&table, TESTED_HASH(word_ptr, word_ptr + MAX_WORD_LENGTH),
            _mm256_load_si256((const __m256i*) word_ptr), simd_comparison_placeholder);
        #endif
    }

    log_printf(STATUS_REPORTS, "status", "The table is ready for testing.\n");


    #ifdef DISTRIBUTION_TEST  //* DISTRIBUTION TEST CASE ==============================

    log_printf(STATUS_REPORTS, "status", "Opening distribution output file.\n");

    FILE* out_table = fopen(OUTPUT_TABLE_NAME, "w");
    _LOG_FAIL_CHECK_(out_table, "error", ERROR_REPORTS, return_clean(EXIT_FAILURE), NULL, ENOENT);

    log_printf(STATUS_REPORTS, "status", "Reading distribution data.\n");
    fprintf(out_table, "bucket_id,size\n");

    for (unsigned bucket_id = 0; bucket_id < BUCKET_COUNT; ++bucket_id) {
        fprintf(out_table, "%u,%lu\n", bucket_id, table.contents[bucket_id].size);
    }

    if (out_table) fclose(out_table);

    #endif


    #ifdef PERFORMANCE_TEST  //* PERFORMANCE TEST CASE ==============================
    log_printf(STATUS_REPORTS, "status", "Opening benchmark output file.\n");

    FILE* out_timetable = fopen(OUTPUT_TIMETABLE_NAME, "w");
    _LOG_FAIL_CHECK_(out_timetable, "error", ERROR_REPORTS, return_clean(EXIT_FAILURE), NULL, ENOENT);

    log_printf(STATUS_REPORTS, "status", "Writing header to the file.\n");

    fprintf(out_timetable, "test_id,time\n");

    log_printf(STATUS_REPORTS, "status", "Starting tests.\n");

    for (unsigned test_id = 0; test_id < TEST_COUNT; ++test_id) {
        clock_t start_time = clock();

        for (unsigned repetition_id = 0; repetition_id < TEST_REPETITION; ++repetition_id)
        for (size_t word_id = 0; word_id < sample_size; ++word_id) {
            const char* word_ptr = word_list + word_id * MAX_WORD_LENGTH;

            #if OPTIMIZATION_LEVEL < 1
            HashTable_find_value(&table, TESTED_HASH(word_ptr, word_ptr + MAX_WORD_LENGTH), word_ptr, strcmp);
            #else
            HashTable_find_value(&table, TESTED_HASH(word_ptr, word_ptr + MAX_WORD_LENGTH),
                _mm256_load_si256((const __m256i*) word_ptr), simd_comparison_placeholder);
            #endif
        }

        fprintf(out_timetable, "%u,%ld\n", test_id, clock() - start_time);
    }

    log_printf(STATUS_REPORTS, "status", "Testing is finished. Closing the file.\n");

    if (out_timetable) fclose(out_timetable);

    #endif

    return_clean(errno == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}
