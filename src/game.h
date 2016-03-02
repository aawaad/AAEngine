#ifndef GAME_H
#define GAME_H

#include "platform.h"
#include "asset.h"

struct player
{
    vec3 position;
    vec3 velocity;
    mat4 orientation;

    vec3 forward;
    vec3 up;
    vec3 right;
};

struct memory_region
{
    memsize size;
    u8 *base;
    memsize used;
};

struct game_state
{
    b32 isInitialised;

    r32 time;

    simple_mesh *pig;
    simple_mesh *tiger;
    simple_mesh *walls;

    player tigerPlayer;
    vec3 *cameraTarget;
};

struct transient_state
{
    b32 isInitialised;
    memory_region transientMemory;
};

inline void CreateRegion(memory_region *region, memsize size, u8 *base)
{
    region->size = size;
    region->base = base;
    region->used = 0;
}

#define PushStruct(region, type) (type *)PushSize(region, sizeof(type))
#define PushArray(region, count, type) (type *)PushSize(region, (count)*sizeof(type))
inline void *PushSize(memory_region *region, memsize size)
{
    Assert(region->used + size <= region->size);
    void *result = region->base + region->used;
    region->used += size;

    return result;
}

inline void ZeroSize(memsize size, void *base)
{
    u8 *byte = (u8 *)base;
    while(size--)
    {
        *byte++ = 0;
    }
}

#endif

