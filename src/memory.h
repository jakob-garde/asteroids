#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <cstdio>
#include <cstdint>
#include <cassert>
#include <cstring>


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
    T *Add(T element) {
        assert(len < cap);

        arr[len++] = element;
        return LastPtr();
    }
    T *LastPtr() {
        if (len) {
            return arr + len - 1;
        }
        else {
            return NULL;
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


#endif
