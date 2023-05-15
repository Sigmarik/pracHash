#include "main_utils.h"

#include <stdlib.h>
#include <stdarg.h>

#include "lib/util/dbg/logger.h"
#include "lib/util/dbg/debug.h"

#include "src/utils/config.h"

void print_label() {
    printf("Hash table test engine by Ilya Kudryashov.\n");
    printf("Hash table implementation & test engine.\n");
    printf("Build from\n%s %s\n", __DATE__, __TIME__);
    log_printf(ABSOLUTE_IMPORTANCE, "build info", "Build from %s %s.\n", __DATE__, __TIME__);
}

static void fill_segment(void* data);

void generate_data(void* begin, void* end) {
    for (char* segment = (char*)begin; segment < end; segment += MAX_WORD_LENGTH) {
        memset(segment, 0, MAX_WORD_LENGTH);
        fill_segment((void*) segment);
    }
}

static void fill_segment(void* data) {
    #ifdef GEN_INT
        *(unsigned*) data = rand();
    #endif

    #ifdef GEN_DOUBLE
        double gen = (double) rand() / 1000.0;
        fflush(stdout);
        *(double*) data = gen;
    #endif

    #ifdef GEN_STRING
        unsigned length = 0;
        for (int throw_id = 0; throw_id < MAX_WORD_LENGTH; ++throw_id) length += rand() & 1;

        for (unsigned id = 0; id < length; ++id) {
            ((char*) data)[id] = (char)(97 + rand() % (122 - 97));
        }
    #endif
}
