/**
 * @file listworks.h
 * @author Kudryashov Ilya (kudriashov.it@phystech.edu)
 * @brief .cpp - style file for listworks library.
 * @version 0.1
 * @date 2022-10-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//* Function headers are specified in the file listworks_.h

#ifndef LISTWORKS_HPP
#define LISTWORKS_HPP

#include "listworks_.h"

#include <time.h>

#include "list_config.h"

_ListCell* _List_ptr_by_index(List* list, size_t index, int id);

void List_ctor(List* list, size_t capacity, int* const err_code) {
    _LOG_FAIL_CHECK_(check_ptr(list), "error", ERROR_REPORTS, return, err_code, EFAULT);

    #if OPTIMIZATION_LEVEL < 1  //! WARNING: THIS PREPROCESSING CODE IS TASK-SPECIFIC!
    list->buffer = (_ListCell*) calloc(capacity, sizeof(*list->buffer));
    #else
    list->buffer = NULL;
    int alloc_status = posix_memalign((void**)&list->buffer, 32, capacity * sizeof(*list->buffer));
    _LOG_FAIL_CHECK_(alloc_status == 0, "error", ERROR_REPORTS, return, err_code, ENOMEM);
    #endif

    _LOG_FAIL_CHECK_(list->buffer, "error", ERROR_REPORTS, return, err_code, ENOMEM);

    for (size_t id = 0; id < capacity; ++id) {
        _ListCell* cell = list->buffer + id;
        *cell = _ListCell {};
        cell->next = cell + 1;
        cell->prev = cell - 1;
    }

    list->buffer[0].next = list->buffer;
    list->buffer[0].prev = list->buffer;
    list->buffer[capacity - 1].next = list->buffer + 1;
    list->buffer[1].prev = list->buffer + capacity - 1;

    list->capacity = capacity;
    list->first_empty = list->buffer + 1;
    list->size = 0;

    _LOG_FAIL_CHECK_(List_status(list) == 0, "error", ERROR_REPORTS, return, err_code, EAGAIN);
}

void List_dtor(List* list, int* const err_code) {
    _LOG_FAIL_CHECK_(List_status(list) == 0, "error", ERROR_REPORTS, return, err_code, EFAULT);

    free(list->buffer);
    
    list->buffer = NULL;
    list->capacity = 0;
    list->first_empty = NULL;
    list->size = 0;
}

void List_dtor_void(List* const list) { List_dtor(list, NULL); }

void List_linearize(List* const list, int* const err_code) {
    _LOG_FAIL_CHECK_(List_status(list) == 0, "error", ERROR_REPORTS, return, err_code, EFAULT);

    _ListCell* cell = list->buffer->next;
    size_t index = 0;

    while (cell != list->buffer) {
        _ListCell* target_spot = list->buffer + (index++) + 1;

        target_spot->next->prev = cell;
        if (target_spot == cell->next) {
            target_spot->prev = target_spot;
            cell->next = cell;
        } else {
            target_spot->prev->next = cell;
            cell->next->prev = target_spot;
            cell->prev->next = target_spot;
        }

        _ListCell cell_copy = *cell;
        *cell = *target_spot;
        *target_spot = cell_copy;

        cell = target_spot->next;
    }

    list->first_empty = list->buffer + list->size + 1;

    for (size_t id = 1; id < list->capacity; ++id) {
        list->buffer[id].next = list->buffer + id + 1;
        list->buffer[id].prev = list->buffer + id - 1;
    }

    list->buffer[list->size].next = list->buffer;
    list->buffer->prev = list->buffer + list->size;

    list->buffer[list->size + 1].prev = list->buffer + list->capacity - 1;
    list->buffer[list->capacity - 1].next = list->buffer + list->size + 1;

    list->linearized = true;

    _LOG_FAIL_CHECK_(List_status(list) == 0, "error", ERROR_REPORTS, return, err_code, EAGAIN);
}

list_position_t List_insert(List* const list, const list_elem_t elem, const list_position_t position, int* const err_code) {
    _LOG_FAIL_CHECK_(List_status(list) == 0,          "error", ERROR_REPORTS, return 0, err_code, EFAULT);
    _LOG_FAIL_CHECK_(position < list->capacity,       "error", ERROR_REPORTS, return 0, err_code, EINVAL);
    _LOG_FAIL_CHECK_(list->first_empty->next != NULL, "error", ERROR_REPORTS, return 0, err_code, ENOMEM);

    _ListCell* pasted_cell = NULL;

    if (list->linearized && (list->buffer + position == list->buffer->next || 
                             list->buffer + position == list->buffer->prev)) {

        if (list->buffer + position == list->buffer->prev) {

            pasted_cell = list->first_empty;

            pasted_cell->prev->next = pasted_cell->next;
            pasted_cell->next->prev = pasted_cell->prev;

            list->first_empty = list->first_empty->next;

        } else {

            pasted_cell = list->first_empty->prev;

            list->first_empty->prev = pasted_cell->prev;

            pasted_cell->prev->next = pasted_cell->next;
            pasted_cell->next->prev = pasted_cell->prev;

        }
    } else {
        list->linearized = false;

        pasted_cell = list->first_empty;

        list->first_empty->next->prev = list->first_empty->prev;
        list->first_empty->prev->next = list->first_empty->next;

        list->first_empty = list->first_empty->next;
    }

    pasted_cell->content = elem;

    _ListCell* prev_nbor = list->buffer + position;
    _ListCell* next_nbor = prev_nbor->next;
    
    pasted_cell->next = next_nbor;
    pasted_cell->prev = prev_nbor;
    prev_nbor->next = pasted_cell;
    next_nbor->prev = pasted_cell;

    ++list->size;

    _LOG_FAIL_CHECK_(List_status(list) == 0, "error", ERROR_REPORTS, return 0, err_code, EAGAIN);

    return (list_position_t)(pasted_cell - list->buffer);
}

list_position_t List_push(List* const list, const list_elem_t elem, int* const err_code) {
    if (list->size >= list->capacity - 1) {
        if (List_inflate(list, list->capacity * 2, err_code)) return 0;
    }

    return List_insert(list, elem, List_find_position(list, -1), err_code);
}

list_position_t List_find_position(List* const list, const int index, int* const err_code) {
    _LOG_FAIL_CHECK_(List_status(list) == 0, "error", ERROR_REPORTS, return 0, err_code, EFAULT);

    _LOG_FAIL_CHECK_((-(int)list->size <= index && index < (int)list->size) || list->size == 0, "error", ERROR_REPORTS, {
        log_printf(ERROR_REPORTS, "error", "Requested index was %d with size %lld.\n", index, (long long) list->size);
        return 0;
    }, err_code, EFAULT);

    if (list->size == 0) return 0;

    if (list->linearized) {
        long long delta = index + (long long)(list->capacity - 1);
        _ListCell* count_start = list->buffer->prev;

        if (index >= 0) {
            delta = index - 1;
            count_start = list->buffer->next;
        }

        return (unsigned long long)(count_start - list->buffer + delta) % (list->capacity - 1) + 1;
    }

    _ListCell* current = index >= 0 ? 
        list->buffer->next : list->buffer->prev;

    for (int id = 0; id < index; ++id) {
        current = index >= 0 ? current->next : current->prev;
    }

    return (list_position_t)(current - list->buffer);
}

list_elem_t List_get(List* const list, const list_position_t position, int* const err_code) {
    _LOG_FAIL_CHECK_(List_status(list) == 0,          "error", ERROR_REPORTS, return LIST_ELEM_POISON, err_code, EFAULT);
    _LOG_FAIL_CHECK_(position < list->capacity,       "error", ERROR_REPORTS, return LIST_ELEM_POISON, err_code, EINVAL);
    _LOG_FAIL_CHECK_(list->first_empty->next != NULL, "error", ERROR_REPORTS, return LIST_ELEM_POISON, err_code, ENOMEM);

    return (list->buffer + position)->content;
}

void List_remove(List* const list, const list_position_t position, int* const err_code) {
    _LOG_FAIL_CHECK_(List_status(list) == 0,          "error", ERROR_REPORTS, return, err_code, EFAULT);
    _LOG_FAIL_CHECK_(position < list->capacity,       "error", ERROR_REPORTS, return, err_code, EINVAL);
    _LOG_FAIL_CHECK_(list->first_empty->next != NULL, "error", ERROR_REPORTS, return, err_code, ENOMEM);
    _LOG_FAIL_CHECK_(list->size > 0,                  "error", ERROR_REPORTS, return, err_code, ENOENT);

    #if OPTIMIZATION_LEVEL < 1  //! WARNING: THIS PREPROCESSING CODE IS TASK-SPECIFIC!
    _LOG_FAIL_CHECK_(list->buffer[position].content != LIST_ELEM_POISON, "error", ERROR_REPORTS, return, err_code, EFAULT);
    #endif

    _ListCell* cell = list->buffer + position;

    cell->prev->next = cell->next;
    cell->next->prev = cell->prev;

    if (list->linearized && (cell->next == list->buffer || cell->prev == list->buffer)) {
        if (cell->next == list->buffer) {
            cell->next = list->first_empty;
            cell->prev = list->first_empty->prev;
            list->first_empty->prev->next = cell;
            list->first_empty->prev = cell;
            list->first_empty = cell;
        } else {
            cell->next = list->first_empty;
            cell->prev = list->first_empty->prev;
            list->first_empty->prev->next = cell;
            list->first_empty->prev = cell;
        }
    } else {
        list->linearized = false;

        cell->next = list->first_empty;
        cell->prev = list->first_empty->prev;
        cell->next->prev = cell;
        cell->prev->next = cell;

        list->first_empty = cell;
    }

    cell->content = LIST_ELEM_POISON;
    --list->size;

    _LOG_FAIL_CHECK_(List_status(list) == 0, "error", ERROR_REPORTS, return, err_code, EAGAIN);
}

int List_inflate(List* const list, size_t new_capacity, int* const err_code) {
    _LOG_FAIL_CHECK_(List_status(list) == 0,    "error", ERROR_REPORTS, return 1, err_code, EFAULT);
    _LOG_FAIL_CHECK_(new_capacity > list->size, "error", ERROR_REPORTS, return 1, err_code, EINVAL);

    List new_list = {};
    List_ctor(&new_list, new_capacity, err_code);
    _LOG_FAIL_CHECK_(List_status(&new_list) == 0, "error", ERROR_REPORTS, return 1, err_code, EFAULT);

    _ListCell* current_cell = list->buffer->next;
    for (size_t id = 0; id < list->size; ++id) {
        List_insert(&new_list, current_cell->content, List_find_position(&new_list, -1, err_code), err_code);
        current_cell = current_cell->next;
    }

    List_dtor(list, err_code);

    *list = new_list;

    return 0;
}

list_report_t List_status(List* const list) {
    _LOG_FAIL_CHECK_(list, "error", ERROR_REPORTS, return LIST_NULL, NULL, 0);

    list_report_t report = 0;

    if (list->size >= list->capacity) report |= LIST_BIG_SIZE;
    if (!check_ptr(list->buffer))     report |= LIST_NULL_CONTENT;
    else if (list->first_empty <= list->buffer || list->first_empty > list->buffer + list->capacity)
        report |= LIST_INV_FREE;
    
    #ifdef _DEBUG
    for (_ListCell* cell = list->buffer; cell < list->buffer + list->capacity; ++cell) {
        if (cell->prev == NULL || cell->next == NULL ||
            cell->prev->next != cell || cell->next->prev != cell) report |= LIST_INV_CONNECTIONS;
    }
    #endif

    return report;
}

void _List_dump(List* const list, const unsigned int importance, const int line, const char* func_name, const char* file_name) {
    _log_printf(importance, LIST_DUMP_TAG, " ----- List dump in function %s of file %s (%lld): ----- \n",
                func_name, file_name, (long long) line);

    list_report_t status = List_status(list);

    _log_printf(importance, LIST_DUMP_TAG, "List at %p:\n", list);

    _log_printf(importance, LIST_DUMP_TAG, "\tStatus: %s\n", status ? "CORRUPT" : "OK");

    for (int error_id = 0; error_id < (int)sizeof(LIST_STATUS_DESCR) / (int)sizeof(LIST_STATUS_DESCR[0]); ++error_id) {
        if (status & (1 << error_id)) {
            _log_printf(importance, LIST_DUMP_TAG, "\t\t%s\n", LIST_STATUS_DESCR[error_id]);
        }
    }

    _log_printf(importance, LIST_DUMP_TAG, "List:\n");

    _log_printf(importance, LIST_DUMP_TAG, "\tfirst empty = %lld,\n", (long long) (list->first_empty - list->buffer));
    _log_printf(importance, LIST_DUMP_TAG, "\tsize =        %lld,\n", (long long) list->size);
    _log_printf(importance, LIST_DUMP_TAG, "\tcapacity =    %lld,\n", (long long) list->capacity);
    _log_printf(importance, LIST_DUMP_TAG, "\tlinearized =  %d,\n", list->linearized);

    _log_printf(importance, LIST_DUMP_TAG, "\tbuffer at %p:\n", list->buffer);

    for (size_t id = 0; id < list->capacity; id++) {
        unsigned char* data_start = (unsigned char*)(list->buffer + id);
        #if OPTIMIZATION_LEVEL < 1  //! WARNING: THIS PREPROCESSING CODE IS TASK-SPECIFIC!
        _log_printf(importance, LIST_DUMP_TAG, "\t\t[%5ld] = %02X %02X %02X %02X (%s), next [%lld], prev [%lld]\n", (long) id,
            data_start[0], data_start[1], data_start[2], data_start[3],
            list->buffer[id].content == LIST_ELEM_POISON ? "POISON" : "VALUE",
            (long long) (list->buffer[id].next - list->buffer), (long long) (list->buffer[id].prev - list->buffer));
        #endif
    }
}

static int PictCount = 0;

void _List_dump_graph(List* const list, unsigned int importance) {
    _LOG_FAIL_CHECK_(List_status(list) == 0, "error", ERROR_REPORTS, return, NULL, 0);

    FILE* temp_file = fopen(LIST_TEMP_DOT_FNAME, "w");
    
    _LOG_FAIL_CHECK_(temp_file, "error", ERROR_REPORTS, return, NULL, 0);

    fputs("digraph G {\n", temp_file);
    fputs(  "\trankdir=LR\n"
            "\tlayout=dot\n"
            "\tsplines=ortho\n"
            , temp_file);

    for (size_t id = 0; id < list->capacity; ++id) {
        unsigned char* data = (unsigned char*)&(list->buffer + id)->content;
        _ListCell* cell = list->buffer + id;
        fprintf(temp_file, LIST_VERTEX_FORMAT);
    }

    for (size_t id = 0; id < list->capacity - 1; ++id) {
        fprintf(temp_file, "\tV%d->V%d [weight=999999999 color=none]\n", (int)id, (int)id + 1);
    }

    for (size_t id = 0; id < list->capacity; ++id) {
        fprintf(temp_file, "\tV%ld->V%ld [arrowsize=0.3]\n", (long int)id, list->buffer[id].next - list->buffer);
    }

    fputc('}', temp_file);
    fclose(temp_file);

    if (system("mkdir -p " LIST_LOG_ASSET_FOLD_NAME)) return;

    time_t raw_time = 0;
    time(&raw_time);

    char pict_name[LIST_PICT_NAME_SIZE] = "";
    sprintf(pict_name, LIST_LOG_ASSET_FOLD_NAME "/pict%04d_%ld.png", ++PictCount, raw_time);

    char draw_request[LIST_DRAW_REQUEST_SIZE] = "";
    sprintf(draw_request, "dot -Tpng -o %s " LIST_TEMP_DOT_FNAME, pict_name);

    if (system(draw_request)) return;

    _log_printf(importance, "list_img_dump", "\n<img src=\"%s\">\n", pict_name);
}

#endif