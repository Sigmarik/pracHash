/**
 * @file listworks_.h
 * @author Kudryashov Ilya (kudriashov.it@phystech.edu)
 * @brief Listworks library implementing list data structure.
 * @version 0.1
 * @date 2022-10-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef LISTWORKS_H
#define LISTWORKS_H

#include <stdint.h>

#include "lib/util/dbg/debug.h"
#include "listreports.h"

const char LIST_DUMP_TAG[] = "list_dump";

//* In case of a compilation error on this line check if [list_elen_t LIST_ELEM_POISON] is defined before the library include.
static const list_elem_t ___LIST_ELEM_T_OR_POISON_IS_NOT_DEFINED___ = LIST_ELEM_POISON;
//* Define [list_elem_t] and [list_elem_t LIST_ELEM_POISON] with the value you want the list to consider poisonous.
//* Example:
//*   typedef char list_elem_t;
//*   static const list_elem_t LIST_ELEM_POISON = '\0';
//*   #include "listworks.h"

//* Type that is used to identify elements in raw list buffer.
typedef uintptr_t list_position_t;

/**
 * @brief Primary content of the list with all the linkage.
 * 
 */
struct _ListCell {
    _ListCell* next = NULL;
    _ListCell* prev = NULL;
    list_elem_t content = LIST_ELEM_POISON;
#if OPTIMIZATION_LEVEL < 1
};
#else
} __attribute__((__aligned__(64)));
#endif

/**
 * @brief List data structure.
 * 
 * @param buffer
 * @param first_empty
 * @param size
 * @param capacity
 * @param linearized
 */
struct List {
    _ListCell* buffer = NULL;
    _ListCell* first_empty = NULL;
    size_t size = 0;
    size_t capacity = 0;
    bool linearized = true;
};

/**
 * @brief Initialize list of the specified size.
 * 
 * @param list list to initialize
 * @param capacity max number of elements the list can hold +1 empty element
 * @param err_code variable to use as errno
 */
void List_ctor(List* list, size_t capacity = 1024, int* const err_code = NULL);

/**
 * @brief Destroy the list.
 * 
 * @param list list to uninitialize
 * @param err_code variable to use as errno
 */
void List_dtor(List* list, int* const err_code = NULL);

/**
 * @brief Dtor-capable destructor function.
 * 
 * @param list list to destroy
 */
void List_dtor_void(List* const list);

/**
 * @brief Sort list elements for faster element access.
 * 
 * @param list list to linearize
 * @param err_code variable to use as errno
 */
void List_linearize(List* const list, int* const err_code = NULL);

/**
 * @brief Insert element into the list.
 * 
 * @param list 
 * @param elem element to insert
 * @param position which element to insert after
 * @param err_code variable to use as errno
 */
list_position_t List_insert(List* const list, const list_elem_t elem, const list_position_t position, int* const err_code = NULL);

/**
 * @brief Push element to the back of the list.
 * 
 * @param list pointer to the list
 * @param elem element to push
 * @param err_code variable to use as errno
 * @return 
 */
list_position_t List_push(List* const list, const list_elem_t elem, int* const err_code = NULL);

/**
 * @brief Find position of the index'th element in the list.
 * 
 * @param list 
 * @param index index of the element
 * @param err_code variable to use as errno
 * @return 
 */
list_position_t List_find_position(List* const list, const int index, int* const err_code = NULL);

/**
 * @brief Get element from the list at specified position.
 * 
 * @param list 
 * @param position position of the element
 * @param err_code variable to use as errno
 * @return list_elem_t
 */
list_elem_t List_get(List* const list, const list_position_t position, int* const err_code = NULL);

/**
 * @brief Remove element from the list.
 * 
 * @param list 
 * @param position position of the element
 * @param err_code variable to use as errno
 */
void List_remove(List* const list, const list_position_t position, int* const err_code = NULL);

/**
 * @brief Relocate and increase the size of the list
 * 
 * @param list pointer to the list
 * @param new_size new size of the list
 * @return 0 if relocation was successful, 1 otherwise
 */
int List_inflate(List* const list, size_t new_size, int* const err_code = NULL);

/**
 * @brief Get info about list as binary mask.
 * 
 * @param list 
 * @return list_report_t
 */
list_report_t List_status(List* const list);

/**
 * @brief Dump the list into logs.
 * 
 * @param list
 * @param importance message importance
 */
#define List_dump(list, importance) \
    log_printf(importance, LIST_DUMP_TAG, "Called list dumping.\n"); \
    _List_dump(list, importance, __LINE__, __PRETTY_FUNCTION__, __FILE__); \
    _List_dump_graph(list, importance);

/**
 * @brief [Should only be called by List_dump() macro] Dump the list into logs.
 * 
 * @param list 
 * @param importance message importance
 * @param line line at which the call was at
 * @param func_name name of the top-function
 * @param file_name name of the file where invocation happened
 */
void _List_dump(List* const list, const unsigned int importance, const int line, const char* func_name, const char* file_name);

/**
 * @brief Place an image of the list into log HTML document.
 * 
 * @param list 
 * @param importance
 */
void _List_dump_graph(List* const list, const unsigned int importance);

#endif