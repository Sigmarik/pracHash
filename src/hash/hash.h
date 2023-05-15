/**
 * @file hash.h
 * @author Kudryashov Ilya (kudriashov.it@phystech.edu)
 * @brief Definitions useful for standardized hash-related workflow.
 * @version 0.1
 * @date 2023-04-17
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef HASH_H
#define HASH_H

typedef unsigned long long hash_t;

typedef hash_t hash_fn_t(const void* begin, const void* end);
#define HASH_FUNCTION(name) hash_t name(const void* begin, const void* end)

#endif
