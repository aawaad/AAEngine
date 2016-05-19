//#include "AAMath/aamath.h"
#include "game.h"
#include "asset_loader.cpp"
#include "render_group.cpp"
#include "renderer_opengl.cpp"

static void GameUpdate(game_memory *Memory, s64 ElapsedTime, game_input *Input, game_render_commands *RenderCommands)
{
    Assert((&Input->Controllers[0].BtnCount - &Input->Controllers[0].Buttons[0])
            == ArrayCount(Input->Controllers[0].Buttons));

    Assert(sizeof(game_state) <= Memory->PermanentMemorySize);
    game_state *GameState = (game_state *)Memory->PermanentMemory;

    Platform = Memory->PlatformAPI;

    if(!GameState->IsInitialised)
    {
        memory_region Total;
        CreateRegion(&Total, Memory->PermanentMemorySize - sizeof(game_state),
                     (u8 *)Memory->PermanentMemory + sizeof(game_state));

        // TODO: Permanent Memory regions go here

        GameState->Time = 0;
        GameState->DeltaTime = 0;

        GameState->IsInitialised = true;
    }
    GameState->DeltaTime = (r32)(ElapsedTime - GameState->Time) / 1000.0f;
    GameState->Time = ElapsedTime;

    Assert(sizeof(transient_state) <= Memory->TransientMemorySize);
    transient_state *TranState = (transient_state *)Memory->TransientMemory;
    if(!TranState->IsInitialised)
    {
        CreateRegion(&TranState->TransientMemory, Memory->TransientMemorySize - sizeof(transient_state),
                     (u8 *)Memory->TransientMemory + sizeof(transient_state));

        TranState->IsInitialised = true;

        TranState->Assets = (game_assets *)AllocStruct(&TranState->TransientMemory, game_assets);

        transform T;
        /*
        mesh *Mesh = LoadMesh(&TranState->TransientMemory, TranState->Assets, L"../data/helicoid_mobius.obj");
        material *Material = LoadMaterial(&TranState->TransientMemory, TranState->Assets, L"../data/helicoid_mobius.mtl");
        T.Orientation = MAT4_IDENTITY;
        T.Scale = {50, 50, 50};
        AddEntity(GameState, EntityType_Generic, T, {0, 0, 0}, Mesh, Material);
        */

        TranState->Assets->Ship = LoadMesh(&TranState->TransientMemory, TranState->Assets, L"../data/helicoid_ship.obj");
        TranState->Assets->ShipMtl = LoadMaterial(&TranState->TransientMemory, TranState->Assets, L"../data/helicoid_ship.mtl");

        T.Position = {5.0f, 0, 5.0f};
        T.Orientation = Mat4RotationY(PIOVERFOUR * 3);
        T.Scale = Vec3(1.0f, 1.0f, 1.0f);
        AddEntity(GameState, EntityType_Ship, T, {0, 0, 0}, TranState->Assets->Ship, TranState->Assets->ShipMtl);
        T.Position = {-5.0f, 0, 5.0f};
        T.Orientation = Mat4RotationY(-(PIOVERFOUR * 3));
        entity *Entity = AddEntity(GameState, EntityType_Ship, T, {0, 0, 0}, TranState->Assets->Ship, TranState->Assets->ShipMtl);
        Entity->Wireframe = true;
    }

#if 0
    memsize ptrsize = sizeof(WCHAR) * 10;
    WCHAR *ptr  = (WCHAR *)AllocSize(&TranState->TransientMemory, ptrsize);
    WCHAR *ptr2 = (WCHAR *)AllocSize(&TranState->TransientMemory, ptrsize);
    WCHAR *ptr3 = (WCHAR *)AllocSize(&TranState->TransientMemory, ptrsize);
    wcscpy(ptr, L"123456789\0");
    wcscpy(ptr2, L"123456789\0");
    DeallocSize(&TranState->TransientMemory, ptr2, ptrsize);
    DeallocSize(&TranState->TransientMemory, ptr3, ptrsize);
    DeallocSize(&TranState->TransientMemory, ptr, ptrsize);
#endif

    //
    // NOTE: Update
    //

    // IMPORTANT: Remove these!
    vec3 Pfd, Pup, Prt;
    {
        player *P = &GameState->Player;
        game_controller *Con = &Input->Controllers[0];

        local_persist r32 FrameDelay = 0.2f;
        FrameDelay += GameState->DeltaTime;
        if(Con->Start.EndedDown)
        {
            GameState->Player = {};
            P->Transform.Position = {0.0f, 0.75f, -1.0f};
            P->Transform.Orientation = MAT4_IDENTITY;
            P->Basis.Forward = {0.0f, 0.0f, 1.0f};
            P->Basis.Up = {0.0f, 1.0f, 0.0f};
            P->Basis.Right = {1.0f, 0.0f, 0.0f};
            if(FrameDelay > 0.2f)
            {
                GameState->CameraTarget = GameState->CameraTarget ? nullptr : &P->Transform.Position;
                FrameDelay = 0;
            }
        }

        if(Con->ActionUp.EndedDown && FrameDelay > 0.2f)
        {
            mesh *Mesh = LoadMesh(&TranState->TransientMemory, TranState->Assets, L"../data/pig.obj", true);
            material *Material = LoadMaterial(&TranState->TransientMemory, TranState->Assets, L"../data/pig.mtl");
            transform T = {};
            T.Position = {(r32)((GameState->Time / 1000) % 10), 0, 0};
            T.Orientation = MAT4_IDENTITY;
            T.Scale = {1, 1, 1};
            AddEntity(GameState, EntityType_Pig, T, {0, 0, 0}, Mesh, Material);
            FrameDelay = 0;
        }

        if(Con->ActionDown.EndedDown && FrameDelay > 0.2f)
        {
            entity *Entity;
            for(u32 Idx = 0; Idx < GameState->EntityCount; ++Idx)
            {
                Entity = GameState->Entities + Idx;
                if(Entity->Type == EntityType_Pig)
                {
                    RemoveEntity(GameState, Entity);
                    break;
                }
            }
            FrameDelay = 0;
        }

        if(Con->ActionLeft.EndedDown && FrameDelay > 0.2f)
        {
            mesh *Mesh = LoadMesh(&TranState->TransientMemory, TranState->Assets, L"../data/cube.obj");
            material *Material = LoadMaterial(&TranState->TransientMemory, TranState->Assets, L"../data/cube.mtl");
            transform T = {};
            //T.Position = {(r32)((u32)GameState->Time % 10), 0, (r32)((u32)GameState->Time % 10)};
            T.Orientation = MAT4_IDENTITY;
            T.Scale = {1, 1, 1};
            AddEntity(GameState, EntityType_Cube, T, {0, 0, 0}, Mesh, Material);
            FrameDelay = 0;
        }

        if(Con->ActionRight.EndedDown && FrameDelay > 0.2f)
        {
            entity *Entity;
            for(u32 Idx = 0; Idx < GameState->EntityCount; ++Idx)
            {
                Entity = GameState->Entities + Idx;
                if(Entity->Type == EntityType_Cube)
                {
                    RemoveEntity(GameState, Entity);
                    break;
                }
            }
            FrameDelay = 0;
        }

        if(GameState->CameraTarget)
        {
            vec3 DeltaAngles = {};
            r32 ddP = TWOPI * GameState->DeltaTime;
            if(Con->RightShoulder.EndedDown)
            {
                //DeltaAngles.x += 1.0f * GameState->DeltaTime;
                P->Velocity.z += 0.5f * GameState->DeltaTime;
            }
            if(Con->LeftShoulder.EndedDown)
            {
                //DeltaAngles.x -= 1.0f * GameState->DeltaTime;
                P->Velocity.z -= 0.5f * GameState->DeltaTime;
            }
            if(Con->MoveRight.EndedDown)
            {
                //DeltaAngles.y += 4.0f * GameState->DeltaTime;
                P->Velocity.x += ddP * GameState->DeltaTime;
            }
            if(Con->MoveLeft.EndedDown)
            {
                //DeltaAngles.y -= 4.0f * GameState->DeltaTime;
                P->Velocity.x -= ddP * GameState->DeltaTime;
            }
            if(Con->RightShoulder.EndedDown)
            {
                //P->Velocity.z += 1.0f * GameState->DeltaTime;
            }
            if(Con->LeftShoulder.EndedDown)
            {
                //P->Velocity.z -= 1.0f * GameState->DeltaTime;
            }
            if(!Con->RightShoulder.EndedDown && !Con->LeftShoulder.EndedDown)
            {
                if(abs(P->Velocity.z) > EPSILON)
                {
                    P->Velocity.z *= 0.95f;
                }
                else
                {
                    P->Velocity.z = 0;
                }
            }
            if(!Con->MoveRight.EndedDown && !Con->MoveLeft.EndedDown)
            {
                if(abs(P->Velocity.x) > EPSILON)
                {
                    P->Velocity.x *= 0.9f;
                }
                else
                {
                    P->Velocity.x = 0;
                }
            }

            r32 MaxSpeed = 50.0f * GameState->DeltaTime;
            r32 MinSpeed = 10.0f * GameState->DeltaTime;
            if(P->Velocity.z > MaxSpeed) { P->Velocity.z = MaxSpeed; }
            if(P->Velocity.z < -MinSpeed) { P->Velocity.z = -MinSpeed; }

            r32 MaxTurn = PI * GameState->DeltaTime;
            if(P->Velocity.x > MaxTurn) { P->Velocity.x = MaxTurn; }
            if(P->Velocity.x < -MaxTurn) { P->Velocity.x = -MaxTurn; }

            vec3 Fwd = Normalized(P->Transform.Orientation * Vec4(P->Basis.Forward, 0)).xyz;
            P->Transform.Position += Fwd * P->Velocity.z;
            vec3 Up = Normalized(P->Transform.Orientation * Vec4(P->Basis.Up, 0)).xyz;
            vec3 Right = Normalized(Cross(Fwd, Up));
            Pfd = Fwd;
            Pup = Up;
            Prt = Right;

            quat Rot = QuatAxisAngle(Fwd, DeltaAngles.z);
            Rot *= QuatAxisAngle(Up, P->Velocity.x);//DeltaAngles.y);
            Rot *= QuatAxisAngle(Right, DeltaAngles.x);
            P->Transform.Orientation *= Mat4Rotation(Rot);
        }
    }

    //
    // NOTE: Render
    //

    Clear(RenderCommands, {0.1f, 0.1f, 0.1f, 1.0f});
    RenderCommands->Projection = Perspective(45.0f + Input->Mouse.Y * 45.0f, (r32)RenderCommands->Width/(r32)RenderCommands->Height, 0.1f, 1000.0f);

    if(GameState->CameraTarget)
    {
        //vec3 Cam = ((Mat4Translation(GameState->Player.Position) * GameState->Player.Orientation) * Vec4(0, 2.0f, -5.0f, 0)).xyz;
        player *P = &GameState->Player;
        RenderCommands->View = LookAt(P->Transform.Position + Pfd * -5.0f + Pup * 2.0f, P->Transform.Position, Pup);
    }
    else
    {
        r32 Limit = TWOPI;//3.0f * PIOVERFOUR;
        r32 Offset = -PIOVERTWO;
        RenderCommands->View = LookAt(Vec3(cosf(Clamp((Input->Mouse.X * 2.0f - 1.0f) * Limit, -Limit, Limit) + Offset) * -6.0f, 4.0f,
                                           sinf(Clamp((Input->Mouse.X * 2.0f - 1.0f) * Limit, -Limit, Limit) + Offset) * 6.0f),
                                           Vec3(0, 1, 0),
                                           //GameState->CameraTarget ? *GameState->CameraTarget : Vec3(0, 1, 0),
                                           Vec3(0, 1, 0));
    }

    // NOTE: This currently goes through every single entity and renders it
    for(u32 Idx = 0; Idx < GameState->EntityCount; ++Idx)
    {
        entity *Entity = GameState->Entities + Idx;
        switch(Entity->Type)
        {
            case EntityType_Generic:
            case EntityType_Ship:
            case EntityType_Cube:
            {
                if(Entity->Wireframe)
                    Wireframe(RenderCommands, true);

                mat4 T = Mat4Translation(Entity->Transform.Position);
                mat4 R = Entity->Transform.Orientation;
                mat4 S = Mat4Scaling(Entity->Transform.Scale);
                PushMesh(RenderCommands, T * R * S, Entity->Mesh, Entity->Material);

                if(Entity->Wireframe)
                    Wireframe(RenderCommands, false);
            } break;

            case EntityType_Pig:
            {
                static r32 a = 0;
                a += GameState->DeltaTime;
                mat4 PigSca = Mat4Scaling(1.0f + sinf(4.0f * a) * 0.25f);
                mat4 PigRot = Mat4RotationY(a);
                mat4 PigTrs = Mat4Translation(Entity->Transform.Position);
                mat4 PigWorld = PigRot * PigTrs * PigSca;

                PushMesh(RenderCommands, PigWorld, Entity->Mesh, Entity->Material);
            } break;

            case EntityType_Player:
            {
            } break;
        }
    }

    //PushMesh(RenderCommands, MAT4_IDENTITY, TranState->Assets->Ship, TranState->Assets->ShipMtl);

    if(GameState->CameraTarget)
    {
        PushMesh(RenderCommands, Mat4Translation(GameState->Player.Transform.Position) * GameState->Player.Transform.Orientation,
                TranState->Assets->Ship, TranState->Assets->ShipMtl);
    }
}
