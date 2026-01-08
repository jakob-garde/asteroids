#ifndef __MEMORY_H__
#define __MEMORY_H__


// for release builds, we should set DEBUG_BUILD = 0
#ifndef DEBUG_BUILD
#define DEBUG_BUILD 1
#endif


#include <cstdio>
#include <cstdint>
#include <cassert>
#include <cstring>
#include <math.h>


typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float f32;
typedef double f64;


struct MArena {
    u8 *mem;
    u64 cap;
    u64 used;
};

MArena ArenaCreate(void *mem, u64 capacity = 0) {
    MArena a = {};
    a.cap = capacity;
    a.mem = (u8*) mem;
    return a;
}

void *ArenaAlloc(MArena *a, u64 len, bool zerod = true) {
    assert(a->cap >= a->used + len && "ArenaAlloc: capaciry exceeded");

    void *result = a->mem + a->used;
    a->used += len;
    memset(result, 0, len);

    return result;
}

void *ArenaPush(MArena *a, void *data, u32 len) {
    void *dest = ArenaAlloc(a, len);
    memcpy(dest, data, len);
    return dest;
}

void ArenaClear(MArena *a) {
    a->used = 0;
}


template<typename T>
struct Array {
    T *arr = NULL;
    u32 len = 0;
    u32 cap = 0;

    inline
    void Add(T element) {
        assert(len < cap);

        arr[len++] = element;
    }
    void AddSafe(T element) {
        if (len < cap) {
            arr[len++] = element;
        }
    }
};

template<class T>
Array<T> InitArray(MArena *a, u32 max_len) {
    Array<T> _arr = {};
    _arr.len = 0;
    _arr.cap = max_len;
    _arr.arr = (T*) ArenaAlloc(a, sizeof(T) * max_len);
    return _arr;
}


// TODO: hashing: hash, random, map
// TODO: deg2rot, rot2deg


u64 Hash(u64 x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x);
    return x;
}
void Kiss_SRandom(u64 state[7], u64 seed) {
    if (seed == 0) seed = 1;
    state[0] = seed | 1; // x
    state[1] = seed | 2; // y
    state[2] = seed | 4; // z
    state[3] = seed | 8; // w
    state[4] = 0;        // carry
}
u64 Kiss_Random(u64 state[7]) {
    state[0] = state[0] * 69069 + 1;
    state[1] ^= state[1] << 13;
    state[1] ^= state[1] >> 17;
    state[1] ^= state[1] << 5;
    state[5] = (state[2] >> 2) + (state[3] >> 3) + (state[4] >> 2);
    state[6] = state[3] + state[3] + state[2] + state[4];
    state[2] = state[3];
    state[3] = state[6];
    state[4] = state[5] >> 30;
    return state[0] + state[1] + state[3];
}
u64 g_kiss_randstate[7];

u32 RandInit(u32 seed = 0) {
    if (seed == 0) {
        seed = (u32) Hash(42);
    }
    Kiss_SRandom(g_kiss_randstate, seed);
    Kiss_Random(g_kiss_randstate); // flush the first one

    return seed;
}

#ifndef ULONG_MAX
#  define ULONG_MAX ( (u64) 0xffffffffffffffffUL )
#endif

f32 Rand01() {
    f32 randnum = (f32) Kiss_Random(g_kiss_randstate);
    randnum /= (f32) ULONG_MAX + 1;
    return randnum;
}

f32 RandPM1() {
    f32 randnum = (f32) Kiss_Random(g_kiss_randstate);
    randnum = 2.0f * randnum / ((f32) ULONG_MAX + 1) - 1;
    return randnum;
}

bool RandBin() {
    u64 num =  Kiss_Random(g_kiss_randstate) % 2;
    return num == 1;
}

s32 Rand(u64 max) {
    u64 num =  Kiss_Random(g_kiss_randstate) % max;
    return (s32) num;
}

#endif
