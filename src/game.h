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
      - Dynamic allocation (free, check for free space, merge adjacent free space)
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

*/

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

