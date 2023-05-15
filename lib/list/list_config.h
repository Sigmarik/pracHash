/**
 * @file list_config.h
 * @author Kudryashov Ilya (kudriashov.it@phystech.edu)
 * @brief List of constants used inside listworks library.
 * @version 0.1
 * @date 2022-10-30
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef LIST_CONFIG_H
#define LIST_CONFIG_H

#define LIST_TEMP_DOT_FNAME  "temp.dot"
#define LIST_LOG_ASSET_FOLD_NAME "log_assets"

#define LIST_VALUE_COLOR "none"
#define LIST_POISON_COLOR "grey90"

#if OPTIMIZATION_LEVEL < 1  //! WARNING: THIS PREPROCESSING CODE IS TASK-SPECIFIC!
#define LIST_VERTEX_FORMAT "\tV%d[shape=plaintext label=<<TABLE\n" \
    "\t\tBORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\">\n" \
    "\t\t<TR><TD PORT=\"head\" BGCOLOR=\"%s\">Cell %d</TD></TR>\n" \
    "\t\t<TR><TD BGCOLOR=\"%s\">%02X %02X %02X %02X</TD></TR>\n" \
    "\t\t<TR><TD PORT=\"bottom\">P:%ld N:%ld</TD></TR></TABLE>>]\n", (int)id, \
    cell==list->first_empty || cell==list->buffer ? LIST_POISON_COLOR : LIST_VALUE_COLOR, \
    (int)id, cell->content==LIST_ELEM_POISON ? LIST_POISON_COLOR : LIST_VALUE_COLOR, \
    data[0], data[1], data[2], data[3], cell->prev-list->buffer, cell->next-list->buffer
#else
#define LIST_VERTEX_FORMAT "\tV%d", (int)id
#endif

const size_t LIST_PICT_NAME_SIZE = 128;
const size_t LIST_DRAW_REQUEST_SIZE = 256;

#endif