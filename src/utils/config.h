/**
 * @file config.h
 * @author Kudryashov Ilya (kudriashov.it@phystech.edu)
 * @brief List of constants used inside the main program.
 * @version 0.1
 * @date 2022-11-01
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef MAIN_CONFIG_H
#define MAIN_CONFIG_H

#include <stdlib.h>

static const int NUMBER_OF_OWLS = 10;

static const char OUTPUT_TABLE_NAME[] = "output.csv";
static const char OUTPUT_TIMETABLE_NAME[] = "bmark.csv";

static const unsigned MAX_WORD_LENGTH = 32;

#ifndef OPTIMIZATION_LEVEL
#define OPTIMIZATION_LEVEL 0
#endif

#ifndef BUCKET_COUNT
    static const unsigned BUCKET_COUNT = 1000;
#endif

#ifndef TEST_COUNT
    static const unsigned TEST_COUNT = 100000;
#endif
#endif