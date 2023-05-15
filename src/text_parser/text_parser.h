/**
 * @file text_parser.h
 * @author Kudryashov Ilya (kudriashov.it@phystech.edu)
 * @brief Module for parsing text
 * @version 0.1
 * @date 2023-04-24
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef TEXT_PARSER_H
#define TEXT_PARSER_H

#include "lib/util/dbg/debug.h"

/**
 * @brief Read word list from file
 * 
 * @param file_name name of the file to read
 * @param buffer_ptr variable to store buffer address to
 * @return number of words read (0 if failed)
 */
size_t read_words(const char* file_name, const char** buffer_ptr);

#endif
