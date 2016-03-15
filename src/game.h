#ifndef GAME_H
#define GAME_H

#include "platform.h"
#include "asset.h"

/*
    TODO:

    Maths library (largely finished as another project)
      - Still needs capsules and their intersections
      - + OBBs
      - various other collision methods
      - curves (bezier etc)

    Memory Allocator
      x Basic stack allocation
      - Temporary memory blocks
      x Dynamic allocation (free, check for free space, merge adjacent free space)
      - Contiguous block allocation?

    Renderer
      x Take input/commands from a buffer sent from the game layer
      - Light management
      - Camera management? (or should this be purely in the game layer which just sends a view matrix?)
      - Frustrum culling (needs bounding spheres/scene hierarchy)
      - Material / shader switching support
      - Debug drawing
      - Sorting
        - Transparency?

    Assets
      x Basic art loading (meshes, textures)
        - Better OBJ loader?
        - More image formats? (unnecessary with pack file)
        - Fonts/text
      - Load from pack file
        - Custom file format
      - Streaming (dynamic allocation)
      - Resource management (i.e. don't load the same file multiple times (fixed with pack file?))

    Entity System
      - Tracking (table, IDs etc)
      - Updating
      - Collision
        - Collision flags
      - General utility (get relative/canonical position, get ground point, etc)
      - Picking
      - Misc. game features?

    HID System (Input)
      x Basic input
      x Buttons statically assigned to actions
      - Proper input management and key reassignment (e.g. ability to reassign keys to whatever action)

    Audio
      - Basic audio loading and playback
      - Sound effect triggers
      - Music
      - Basic mixer
        - Volume
        - Pitch shifting

    Debugging
      - Logging
      - Graphs
      - In-game console?
      - Misc. drawing

    Scene Management
      - Scene graph
      - Bounding spheres
      - Portals?
      - World chunks?
      - High/low entity update frequency?

    Physics
      - Simulation
        - Collision
        - Force
        - Friction
        - Torque

    Multithreading
      - Work queue?
      - Asset streaming?
      - Audio streaming?

    Animation
      - Skeletal animation?
        - Inverse kinematics?
      - Particle systems?

    Networking?

    AI?
      - Basic A* path finding
      - Navigation meshes?
        - Generation

    Misc.
      - Random number generation
      - SIMD optimisation
      - Move away from CRT?
      - Get rid of GLEW

*/

struct entity_player
{
    vec3 position;
    vec3 velocity;
    mat4 orientation;

    vec3 forward;
    vec3 up;
    vec3 right;
};

struct free_memory
{
    memsize size;
    free_memory *next;
};

struct memory_region
{
    memsize size;
    u8 *base;
    memsize used;
    free_memory *freeList;
};

struct game_state
{
    b32 isInitialised;

    r32 time;

    simple_mesh *pig;
    simple_mesh *tiger;
    simple_mesh *walls;
    simple_mesh *ship;
    simple_mesh *track;

    entity_player player;
    vec3 *cameraTarget;
};

struct transient_state
{
    b32 isInitialised;
    memory_region transientMemory;
};

inline void CreateRegion(memory_region *region, memsize size, u8 *base)
{
    Assert(size > sizeof(free_memory));

    region->size = size;
    region->base = base;
    region->used = 0;
    region->freeList = (free_memory *)base;
    region->freeList->size = size;
    region->freeList->next = nullptr;
}

#define AllocStruct(region, type) (type *)AllocSize(region, sizeof(type))
#define AllocArray(region, count, type) (type *)AllocSize(region, (count)*sizeof(type))
inline void *AllocSize(memory_region *region, memsize size)
{
    Assert(region->used + size <= region->size);
    Assert(size > 0);

    free_memory *prev = nullptr;
    free_memory *curr = region->freeList;
    memsize totalSize = size;

    while(curr != nullptr)
    {
        // NOTE: If current region's size is too small, save current to prev and continue to next
        if(curr->size < size)
        {
            prev = curr;
            curr = curr->next;
            continue;
        }

        // NOTE: If there isn't enough space left for a new free region then consume the entire region
        //       else allocate, then create a new free region and add it to the list
        if(curr->size - size < sizeof(free_memory))
        {
            totalSize = curr->size;

            if(prev != nullptr) { prev->next = curr->next; }
            else { region->freeList = curr->next; }
        }
        else
        {
            free_memory *free = (free_memory *)(curr + size);
            free->size = curr->size - size;
            free->next = curr->next;

            if(prev != nullptr) { prev->next = free; }
            else { region->freeList = free; }
        }

        region->used += totalSize;
        return (void *)curr;
    }

    Assert(!"Region out of memory!");

    return nullptr;
}

inline void DeallocSize(memory_region *region, void *base, memsize size)
{
    Assert(base != nullptr);

    free_memory *prev = nullptr;
    free_memory *curr = region->freeList;

    while(curr != nullptr)
    {
        // NOTE: Check if the current free region address is past the region to the deallocated
        if((uptr)curr >= ((uptr)base + size)) { break; }

        prev = curr;
        curr = curr->next;
    }

    // NOTE: If no previous, current is past the given base address, so free and add to freelist
    //       else if previous is adjacent to the given base, just increase previous' size to overlap
    //       else create a new free region and place it inbetween prev and prev's next
    if(prev == nullptr)
    {
        prev = (free_memory *)base;
        prev->size = size;
        prev->next = region->freeList;
        region->freeList = prev;
    }
    else if(((uptr)prev + prev->size) == (uptr)base)
    {
        prev->size += size;
    }
    else
    {
        free_memory *free = (free_memory *)base;
        free->size = size;
        free->next = prev->next;
        prev->next = free;
        prev = free; // NOTE: This is so that the check below updates the correct node!
    }

    // NOTE: If current isn't null and it's adjacent to the given base, grow prev and update link
    if((curr != nullptr) && ((uptr)curr == ((uptr)base + size)))
    {
        prev->size += curr->size;
        prev->next = curr->next;
    }
    
    region->used -= size;
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

