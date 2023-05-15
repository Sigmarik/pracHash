/**
 * @file main_utils.h
 * @author Kudryashov Ilya (kudriashov.it@phystech.edu)
 * @brief Main program utilities.
 * @version 0.1
 * @date 2023-03-14
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef MAIN_UTILS_H
#define MAIN_UTILS_H

#include <cstring>
#include <ctype.h>

#include "common_utils.h"

/**
 * @brief Print program label and build date/time to console and log.
 * 
 */
void print_label();

/**
 * @brief Fill buffer with random data
 * 
 * @param begin 
 * @param end 
 */
void generate_data(void* begin, void* end);

#endif