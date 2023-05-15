/**
 * @file listreports.h
 * @author Kudryashov Ilya (kudriashov.it@phystech.edu)
 * @brief Reporting utils for listworks.
 * @version 0.1
 * @date 2022-10-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef LISTREPORTS_H
#define LISTREPORTS_H

typedef int list_report_t;
enum ListStatus {
    LIST_NULL =             1 << 0,
    LIST_BIG_SIZE =         1 << 1,
    LIST_NULL_CONTENT =     1 << 2,
    LIST_INV_FREE =         1 << 3,
    LIST_INV_CONNECTIONS =  1 << 4,
};

static const char* const LIST_STATUS_DESCR[] = {
    "List pointer was invalid.",
    "List size was bigger than its capacity.",
    "List buffer pointer was invalid.",
    "List pointer to the first empty cell was invalid.",
    "List element connections were invalid.",
};

#endif