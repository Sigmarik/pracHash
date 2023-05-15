#include "text_parser.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "src/utils/config.h"

size_t read_words(const char* file_name, const char** buffer_ptr) {
    int fd = open(file_name, O_RDONLY);

    _LOG_FAIL_CHECK_(fd != -1, "error", ERROR_REPORTS, return 0, NULL, ENOENT);

    struct stat st = {};
    fstat(fd, &st);

    _LOG_FAIL_CHECK_(st.st_size % MAX_WORD_LENGTH == 0, "error", ERROR_REPORTS, return 0, NULL, EINVAL);

    *buffer_ptr = (const char*) mmap(NULL, (size_t) st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    return (size_t) st.st_size / (size_t) MAX_WORD_LENGTH;
}
