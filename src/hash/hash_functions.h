/**
 * @file firstchar_hash.h
 * @author Kudryashov Ilya (kudriashov.it@phystech.edu)
 * @brief Hash function definitions
 * @version 0.1
 * @date 2023-04-17
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef HASH_FUNCTIONS_H
#define HASH_FUNCTIONS_H

#include "hash.h"

hash_t as_is_hash       (const void* begin, const void* end);
hash_t floor_hash       (const void* begin, const void* end);

hash_t constant_hash    (const void* begin, const void* end);
hash_t first_char_hash  (const void* begin, const void* end);
hash_t length_hash      (const void* begin, const void* end);
hash_t sum_hash         (const void* begin, const void* end);
hash_t left_shift_hash  (const void* begin, const void* end);
hash_t right_shift_hash (const void* begin, const void* end);

hash_t poly_hash        (const void* begin, const void* end);

#if OPTIMIZATION_LEVEL < 2
hash_t murmur_hash      (const void* begin, const void* end);
#else
extern hash_t murmur_hash(const void* begin, const void* end);
#endif

#endif
