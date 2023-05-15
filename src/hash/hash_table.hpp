/**
 * @file hash_table.hpp
 * @author Kudryashov Ilya (kudriashov.it@phystech.edu)
 * @brief The most basic hash table.
 * @version 0.1
 * @date 2023-04-17
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef HASH_TABLE_HPP
#define HASH_TABLE_HPP

#include <x86intrin.h>

#include "src/utils/config.h"

#if OPTIMIZATION_LEVEL < 1
typedef const char* HT_ELEM_T;
const HT_ELEM_T HT_ELEM_POISON = NULL;
typedef HT_ELEM_T list_elem_t;
#else
typedef __m256i HT_ELEM_T __attribute__((__aligned__(32)));
const HT_ELEM_T HT_ELEM_POISON = _mm256_set1_epi8(0);
typedef HT_ELEM_T list_elem_t __attribute__((__aligned__(32)));
#endif

static const list_elem_t LIST_ELEM_POISON = HT_ELEM_POISON;

#include "lib/list/listworks.h"

static const size_t DFLT_HT_CELL_SIZE = 256;

typedef unsigned ht_status_t;

typedef int ht_compar_fn_t(HT_ELEM_T alpha, HT_ELEM_T beta);

enum HT_STATUS {
    HT_NULL         = 1 << 0,
    HT_NO_CONTENT   = 1 << 1,
    HT_BROKEN_CELL  = 1 << 3,
};

struct HashTable {
    size_t size = 0;
    List* contents = NULL;
};


//* DECLARATIONS

/**
 * @brief Construct hash table data structure
 * 
 * @param table pointer to the table
 * @param err_code pointer to the errno-functioning variable 
 */
void HashTable_ctor(HashTable* table, ERROR_MARKER);

/**
 * @brief Destroy the table
 * 
 * @param table pointer to the table to destroy
 */
void HashTable_dtor(HashTable* table);

/**
 * @brief Get status of the hash table
 * 
 * @param table pointer to the table
 * @return ht_status_t
 */
ht_status_t HashTable_status(const HashTable* table);

/**
 * @brief Insert an element 
 * 
 * @param table pointer to the table
 * @param hash hash of the new element
 * @param value value of the element
 * @param err_code pointer to the errno-functioning variable
 */
void HashTable_insert(HashTable* table, hash_t hash, HT_ELEM_T value, ht_compar_fn_t comparator, ERROR_MARKER);

/**
 * @brief Get the list of elements matching specified hash from the table
 * 
 * @param table pointer to the tables
 * @param hash hash to search for
 * @return pointer to the list where all elements match specified hash
 */
List* HashTable_find(const HashTable* table, hash_t hash);

/**
 * @brief Find element in hash table by its hash and value
 * 
 * @param table hash table to search in
 * @param hash hash of the element
 * @param value exact value of the element
 * @param comparator comparator function between elements (should return 0 on equality)
 * @return pointer to the element cell in table (NULL if the element was not found)
 */
HT_ELEM_T* HashTable_find_value(const HashTable* table, hash_t hash, HT_ELEM_T value, ht_compar_fn_t* comparator);


//* IMPLEMENTATIONS ==============================

void HashTable_ctor(HashTable* table, err_anchor_t err_code) {
    _LOG_FAIL_CHECK_(table, "error", ERROR_REPORTS, return, err_code, EINVAL);

    table->contents = (List*) calloc(BUCKET_COUNT, sizeof(*table->contents));

    if (!table->contents) {
        *err_code = ENOMEM;
        return;
    }

    table->size = 0;

    for (size_t id = 0; id < BUCKET_COUNT; ++id) {
        table->contents[id] = {};
        log_printf(STATUS_REPORTS, "status", "Initialising bucket with id %lu.\n", id);
        List_ctor(&table->contents[id], DFLT_HT_CELL_SIZE, err_code);
        if (List_status(&table->contents[id]) != 0) {
            for (size_t rem_id = 0; rem_id < id; ++rem_id) List_dtor(table->contents + rem_id);
            *table = {};
            return;
        }
    }
}

void HashTable_dtor(HashTable* table) {
    _LOG_FAIL_CHECK_(HashTable_status(table) == 0, "error", ERROR_REPORTS, return, NULL, EINVAL);

    for (size_t id = 0; id < BUCKET_COUNT; id++) {
        List_dtor(&table->contents[id], NULL);
    }

    free(table->contents);
}

ht_status_t HashTable_status(const HashTable* table) {
    if (!table) return HT_NULL;
    if (!table->contents) return HT_NO_CONTENT;

    #ifdef _DEBUG
    ht_status_t status = 0;
    for (size_t id = 0; id < BUCKET_COUNT; ++id) {
        if (List_status(&table->contents[id])) status |= HT_BROKEN_CELL;
    }
    #endif

    return 0;
}

void HashTable_insert(HashTable* table, hash_t hash, HT_ELEM_T value, ht_compar_fn_t comparator, err_anchor_t err_code) {
    _LOG_FAIL_CHECK_(HashTable_status(table) == 0, "error", ERROR_REPORTS, return, NULL, EINVAL);

    if (HashTable_find_value(table, hash, value, comparator)) return;

    List* list = &table->contents[hash % BUCKET_COUNT];

    List_push(list, value, err_code);

    ++table->size;
}

List* HashTable_find(const HashTable* table, hash_t hash) {
    _LOG_FAIL_CHECK_(HashTable_status(table) == 0, "error", ERROR_REPORTS, return NULL, NULL, EINVAL);
    return &table->contents[hash % BUCKET_COUNT];
}

HT_ELEM_T* HashTable_find_value(const HashTable* table, hash_t hash, HT_ELEM_T value, ht_compar_fn_t* comparator) {
    _LOG_FAIL_CHECK_(HashTable_status(table) == 0, "error", ERROR_REPORTS, return NULL, NULL, EINVAL);

    List* bucket = &table->contents[hash % BUCKET_COUNT];
    _ListCell* iterator = &bucket->buffer[0];

    #if OPTIMIZATION_LEVEL == 0
    for (size_t elem_id = 0; elem_id < bucket->size; ++elem_id, ++iterator) {
        if (iterator->content != HT_ELEM_POISON && comparator(iterator->content, value) == 0) return &iterator->content;
    }
    #endif

    #if OPTIMIZATION_LEVEL >= 1
    __m256i search_word = value;

    for (size_t elem_id = 0; elem_id < bucket->size; ++elem_id, ++iterator) {
        __m256i word = iterator->content;
        if (_mm256_testc_si256(word, search_word)) {
            return &iterator->content;
        }
    }
    #endif

    return NULL;
}

#endif