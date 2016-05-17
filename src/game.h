#ifndef GAME_H
#define GAME_H

#include "renderer.h"
#include "platform.h"
#include "allocator.h"
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

struct player
{
    vec3 Position;
    vec3 Velocity;
    mat4 Orientation;

    vec3 Forward;
    vec3 Up;
    vec3 Right;
};

struct game_state
{
    b32 IsInitialised;

    s64 Time;
    r32 DeltaTime;

    player Player;
    vec3 *CameraTarget;
};

struct transient_state
{
    b32 IsInitialised;
    memory_region TransientMemory;

    game_assets *Assets;
};

global_variable platform_api Platform;

#endif

