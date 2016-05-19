#ifndef GAME_H
#define GAME_H

#include "renderer.h"
#include "platform.h"
#include "allocator.h"
#include "asset.h"
#include "asset_loader.h"

/*
    TODO: (- todo, / partial, x done)

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
      / Camera management? (or should this be purely in the game layer which just sends a view matrix?)
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
      / Streaming (dynamic allocation)
      / Resource management (i.e. don't load the same file multiple times)

    Entity System
      x Tracking (table, IDs etc)
      x Updating
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
      x Work queue?
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

enum entity_type
{
    EntityType_Invalid,
    EntityType_Generic,
    EntityType_Ship,
    EntityType_Pig,
    EntityType_Cube,
    EntityType_Player,
};

struct transform
{
    vec3 Position;
    mat4 Orientation;
    vec3 Scale;
};

struct basis
{
    vec3 Forward;
    vec3 Up;
    vec3 Right;
};

struct entity
{
    entity_type Type;
    transform Transform;
    vec3 Velocity;
    mesh *Mesh;
    material *Material;
    b32 Wireframe;
};

struct player
{
    transform Transform;
    vec3 Velocity;
    basis Basis;
    mesh *Mesh;
    material *Material;
};

struct game_state
{
    b32 IsInitialised;

    s64 Time;
    r32 DeltaTime;

    player Player;
    vec3 *CameraTarget;

    entity Entities[1024];
    u32 EntityCount;
};

struct transient_state
{
    b32 IsInitialised;
    memory_region TransientMemory;

    game_assets *Assets;
};

global_variable platform_api Platform;

// IMPORTANT: It is currently assumed that LoadMesh and LoadMaterial were called before this!
//      TODO: Do something about this dependency
static entity *AddEntity(game_state *GameState, entity_type Type,
                         transform Transform, vec3 Velocity,
                         mesh *Mesh, material *Material)
{
    Assert(GameState->EntityCount < ArrayCount(GameState->Entities));
    u32 Idx = GameState->EntityCount++;

    entity *Entity = GameState->Entities + Idx;
    Entity->Type = Type;
    Entity->Transform = Transform;
    Entity->Velocity = Velocity;
    Entity->Mesh = Mesh;
    //Mesh->Meta.References++;
    Entity->Material = Material;
    //Material->Meta.References++;

    return Entity;
}

static void RemoveEntity(memory_region *Region, game_assets *Assets, game_state *GameState, entity *Entity)
{
    Assert(GameState->EntityCount > 0);
    UnloadMesh(Region, Assets, Entity->Mesh);
    UnloadMaterial(Region, Assets, Entity->Material);
    entity Temp = GameState->Entities[GameState->EntityCount - 1];
    GameState->Entities[GameState->EntityCount - 1] = *Entity;
    Entity->Type = Temp.Type;
    Entity->Transform = Temp.Transform;
    Entity->Velocity = Temp.Velocity;
    Entity->Mesh = Temp.Mesh;
    Entity->Material = Temp.Material;
    Entity->Wireframe = Temp.Wireframe;
    --GameState->EntityCount;
    GameState->Entities[GameState->EntityCount] = {};
}

#endif

