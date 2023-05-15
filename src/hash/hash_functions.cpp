#include "hash_functions.h"

#include <string.h>
#include <inttypes.h>
#include <x86intrin.h>
#include <math.h>

#include "lib/util/dbg/debug.h"
#include "src/utils/config.h"

static hash_t cycle_left(hash_t num, unsigned short shift) {
    hash_t prefix = (num >> (sizeof(hash_t) * 8 - shift));

    return (num << shift) + prefix;
}

static hash_t cycle_right(hash_t num, unsigned short shift) {
    hash_t suffix = (num & ((1ull << shift) - 1)) << (sizeof(hash_t) * 8 - shift);

    return (num >> shift) + suffix;
}

static const hash_t BIG_PRIME = 95966417;
static const hash_t HASH_STEP_MULTIPLIER = 52196849;

hash_t ident_hash(const void* begin, const void* end) {
    return *(hash_t*)begin;
}

hash_t mult_hash(const void* begin, const void* end) {
    return *(hash_t*)begin * BIG_PRIME;
}

hash_t floor_hash(const void* begin, const void* end) {
    double* double_ptr = (double*) begin;
    return (hash_t) (*double_ptr);
}

hash_t constant_hash(const void* begin, const void* end)    { SILENCE_UNUSED(begin); SILENCE_UNUSED(end); return 1; }
hash_t first_char_hash(const void* begin, const void* end)  { SILENCE_UNUSED(end); return (hash_t) *(char*)begin; }
hash_t length_hash(const void* begin, const void* end)      { return strnlen((char*) begin, (size_t) ((char*) end - (char*) begin)); }

hash_t sum_hash(const void* begin, const void* end) {
    hash_t sum = 0;
    for (char* ptr = (char*) begin; ptr < (char*) end; ++ptr) {
        sum += (hash_t) *ptr;
    }
    return sum;
}

hash_t left_shift_hash(const void* begin, const void* end) {
    hash_t sum = 0;
    for (char* ptr = (char*) begin; ptr < (char*) end; ++ptr) {
        sum = cycle_left(sum, 1) ^ (hash_t) *ptr;
    }
    return sum;
}

hash_t right_shift_hash(const void* begin, const void* end) {
    hash_t sum = 0;
    for (char* ptr = (char*) begin; ptr < (char*) end; ++ptr) {
        sum = cycle_right(sum, 1) ^ (hash_t) *ptr;
    }
    return sum;
}

hash_t poly_hash(const void* begin, const void* end) {
    hash_t answer = 0;
    for (const char* ptr = (const char*)begin; ptr < end; ++ptr) {
        answer += (hash_t) *ptr;
        answer *= HASH_STEP_MULTIPLIER;
        answer %= BIG_PRIME;
    }
    return answer;
}

#if OPTIMIZATION_LEVEL < 2
hash_t murmur_hash(const void* begin, const void* end) {
    hash_t value = 0xBAADF00DDEADBEEF;

    for (const hash_t* segment = (hash_t*) begin; segment < (hash_t*) end; ++segment) {
        hash_t current = *segment;
        current = cycle_left(current * 0xDED15DED, 31) * 0xCADAB8A9;
        current ^= value;
        current = cycle_left(current, 15) * 0x112C13AB + 0x314159265358979;
        value = current;
    }

    return value;
}
#elif OPTIMIZATION_LEVEL < 3
asm(R"(.global murmur_hash)"                    "\n");
asm(R"(.type murmur_hash, @function)"           "\n");
asm(R"(murmur_hash:)"                           "\n");
asm(R"(  movabsq $0xBAADF00DDEADBEEF, %rax"     "\n");
asm(R"(  movl    $0xCADAB8A9, %r8d"             "\n");
asm(R"(  leaq    334931268(%r8), %r11)"         "\n");
asm(R"(  movabsq $8027858709520580608, %r9"     "\n");
asm(R"(  movabsq $0x314159265358979, %r10"      "\n");
asm(R"(.LBB0_2:"                                "\n");
asm(R"(  movq    (%rdi), %rcx)"                 "\n");
asm(R"(  movq    %rcx, %rdx)"                   "\n");
asm(R"(  imulq   %r11, %rdx)"                   "\n");
asm(R"(  shrq    $33, %rdx)"                    "\n");
asm(R"(  imulq   %r9, %rcx)"                    "\n");
asm(R"(  orq     %rdx, %rcx)"                   "\n");
asm(R"(  imulq   %r8, %rcx)"                    "\n");
asm(R"(  xorq    %rax, %rcx)"                   "\n");
asm(R"(  rolq    $15, %rcx)"                    "\n");
asm(R"(  imulq   $0x112C13AB, %rcx, %rax"       "\n");
asm(R"(  addq    %r10, %rax)"                   "\n");
asm(R"(  addq    $8, %rdi)"                     "\n");
asm(R"(  cmpq    %rsi, %rdi)"                   "\n");
asm(R"(  jb      .LBB0_2)"                      "\n");
asm(R"(.LBB0_3:"                                "\n");
asm(R"(  retq)"                                 "\n");
#endif
