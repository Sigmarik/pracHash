CC = g++
PROFILER = valgrind

CPP_BASE_FLAGS = -I./ -I./include/ -ggdb3 -std=c++2a -O2 -pie -march=corei7 -mavx2	\
-Wall -Wextra -Weffc++				 	 											\
-Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations				\
-Wcast-align -Wchar-subscripts -Wconditionally-supported							\
-Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral		\
-Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op				\
-Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith		\
-Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo				\
-Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn				\
-Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default	\
-Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast		\
-Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers				\
-Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector					\
-Wstack-usage=8192

CPP_SANITIZER_FLAGS = -fcheck-new 													\
-fsized-deallocation -fstack-protector -fstrict-overflow -flto-odr-type-merging		\
-fno-omit-frame-pointer -fPIE -fsanitize=address,bool,${strip 						\
}bounds,enum,float-cast-overflow,float-divide-by-zero,${strip 						\
}integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,${strip 		\
}returns-nonnull-attribute,shift,signed-integer-overflow,undefined,${strip 			\
}unreachable,vla-bound,vptr

CPP_DEBUG_FLAGS = -D _DEBUG

CPPFLAGS = $(CPP_BASE_FLAGS)

BLD_FOLDER = build
ASSET_FOLDER = assets
LOGS_FOLDER = logs

BUILD_LOG_NAME = build.log

DEFAULT_CASE_FLAGS = -D TESTED_HASH=first_char_hash -D DISTRIBUTION_TEST
CASE_FLAGS = $(DEFAULT_CASE_FLAGS)

MAIN_BLD_NAME = hash_testcase
BLD_VERSION = 0.1
BLD_PLATFORM = linux
BLD_TYPE = dev
BLD_FORMAT = .out

BLD_SUFFIX = _v$(BLD_VERSION)_$(BLD_TYPE)_$(BLD_PLATFORM)$(BLD_FORMAT)
MAIN_BLD_FULL_NAME = $(MAIN_BLD_NAME)$(BLD_SUFFIX)

PROJ_DIR = .

LIB_OBJECTS = lib/util/argparser.o 				\
			  lib/util/dbg/logger.o 			\
			  lib/util/dbg/debug.o 				\
			  lib/alloc_tracker/alloc_tracker.o	\
			  lib/speaker.o   					\
			  lib/util/util.o

all: main

OPTIMIZATION_LEVEL = 0

CORE_MAIN_OBJECTS = src/main.o 					\
			   src/utils/main_utils.o 			\
			   src/hash/hash_functions.cpp		\
			   src/text_parser/text_parser.cpp	\
			   src/utils/common_utils.o $(LIB_OBJECTS)

ifeq ($(OPTIMIZATION_LEVEL), 3)
MAIN_OBJECTS = $(CORE_MAIN_OBJECTS) src/hash/asm_replacement.o
else
MAIN_OBJECTS = $(CORE_MAIN_OBJECTS)
endif
main: asset $(addprefix $(PROJ_DIR)/, $(MAIN_OBJECTS))
	@mkdir -p $(BLD_FOLDER)
	@echo Assembling files $(MAIN_OBJECTS)
	@$(CC) $(addprefix $(PROJ_DIR)/, $(MAIN_OBJECTS)) $(CPPFLAGS) -o $(BLD_FOLDER)/$(MAIN_BLD_FULL_NAME)

bmark: asset
	make CASE_FLAGS="-D TESTED_HASH=murmur_hash -D OPTIMIZATION_LEVEL=$(OPTIMIZATION_LEVEL) -D PERFORMANCE_TEST" CPPFLAGS="$(CPP_BASE_FLAGS)"

pfile: asset
	make CASE_FLAGS="-D TESTED_HASH=murmur_hash -D OPTIMIZATION_LEVEL=$(OPTIMIZATION_LEVEL) -D TEST_COUNT=10 -D TEST_REPETITION=1 -D PERFORMANCE_TEST" CPPFLAGS="$(CPP_BASE_FLAGS)"

asset:
	@mkdir -p $(BLD_FOLDER)
	@cp -r $(ASSET_FOLDER)/. $(BLD_FOLDER)

run: asset
	@cd $(BLD_FOLDER) && exec ./$(MAIN_BLD_FULL_NAME) $(ARGS)

FLAGS = $(CPPFLAGS) $(CASE_FLAGS)

%.o: %.cpp
	@echo Building file $^
	@$(CC) $(FLAGS) -c $^ -o $@ > $(BUILD_LOG_NAME)

LST_NAME = asm_listing.log
%.o: %.s
	@echo Building assembly file $^
	@nasm -f elf64 -l $(LST_NAME) $^ -o $@ > $(BUILD_LOG_NAME)

PROFILER_OUTPUT_FILE = callgrind.log
PROFILER_FLAGS = --tool=callgrind --callgrind-out-file=$(PROFILER_OUTPUT_FILE)
ANNOTATOR = callgrind_annotate
ANNOTATOR_FLAGS = --auto=yes
ANNOTATOR_OUTPUT = profile.log

profile: $(BLD_FOLDER)/$(ANNOTATOR_OUTPUT)

$(BLD_FOLDER)/$(PROFILER_OUTPUT_FILE): $(BLD_FOLDER)/$(MAIN_BLD_FULL_NAME)
	cd $(BLD_FOLDER) && $(PROFILER) $(PROFILER_FLAGS) ./$(MAIN_BLD_FULL_NAME) $(ARGS)

$(BLD_FOLDER)/$(ANNOTATOR_OUTPUT): $(BLD_FOLDER)/$(PROFILER_OUTPUT_FILE)
	$(ANNOTATOR) $(ANNOTATOR_FLAGS) $(BLD_FOLDER)/$(PROFILER_OUTPUT_FILE) > $(BLD_FOLDER)/$(ANNOTATOR_OUTPUT)

debug: $(BLD_FOLDER)/$(MAIN_BLD_FULL_NAME)
	cd $(BLD_FOLDER) && radare2 -d ./$(MAIN_BLD_FULL_NAME) $(ARGS)

clean:
	@find . -type f -name "*.o" -delete
	@rm -rf ./$(LOGS_FOLDER)/$(BUILD_LOG_NAME)

rmbld:
	@rm -rf $(BLD_FOLDER)
	@rm -rf $(TEST_FOLDER)

rm:
	@make clean
	@make rmbld