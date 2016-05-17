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

        /*
        GameState->Walls = LoadMesh(L"../data/room_thickwalls.obj");
        GameState->Pig = LoadMesh(L"../data/pig.obj", true);
        GameState->Tiger = LoadMesh(L"../data/tiger.obj", true);
        GameState->Ship = LoadMesh(L"../data/helicoid_ship.obj");
        GameState->Track = LoadMesh(L"../data/helicoid_mobius.obj");
        */

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

        TranState->Assets->Ship = LoadMesh(&TranState->TransientMemory, L"../data/helicoid_ship.obj");
        TranState->Assets->ShipMtl = LoadMaterial(&TranState->TransientMemory, L"../data/helicoid_ship.mtl");
    }

#if 0
    memsize ptrsize = sizeof(u32) * 1024;
    void *ptr = AllocSize(&TranState->TransientMemory, ptrsize);
    void *ptr2 = AllocSize(&TranState->TransientMemory, ptrsize);
    void *ptr3 = AllocSize(&TranState->TransientMemory, ptrsize);
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
            P->Position = {0.0f, 0.75f, -1.0f};
            P->Orientation = MAT4_IDENTITY;
            P->Forward = {0.0f, 0.0f, 1.0f};
            P->Up = {0.0f, 1.0f, 0.0f};
            P->Right = {1.0f, 0.0f, 0.0f};
            if(FrameDelay > 0.2f)
            {
                GameState->CameraTarget = GameState->CameraTarget ? nullptr : &P->Position;
                FrameDelay = 0;
            }
        }

        if(GameState->CameraTarget)
        {
            vec3 DeltaAngles = {};
            r32 ddP = TWOPI * GameState->DeltaTime;
            if(Con->MoveUp.EndedDown)
            {
                //DeltaAngles.x += 1.0f * GameState->DeltaTime;
                P->Velocity.z += 1.0f * GameState->DeltaTime;
            }
            if(Con->MoveDown.EndedDown)
            {
                //DeltaAngles.x -= 1.0f * GameState->DeltaTime;
                P->Velocity.z -= 1.0f * GameState->DeltaTime;
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
            r32 MaxTurn = PI * GameState->DeltaTime;
            if(P->Velocity.x > MaxTurn) { P->Velocity.x = MaxTurn; }
            if(P->Velocity.x < -MaxTurn) { P->Velocity.x = -MaxTurn; }

            vec3 Fwd = Normalized(P->Orientation * Vec4(P->Forward, 0)).xyz;
            P->Position += Fwd * P->Velocity.z;
            vec3 Up = Normalized(P->Orientation * Vec4(P->Up, 0)).xyz;
            vec3 Right = Normalized(Cross(Fwd, Up));
            Pfd = Fwd;
            Pup = Up;
            Prt = Right;

            quat Rot = QuatAxisAngle(Fwd, DeltaAngles.z);
            Rot *= QuatAxisAngle(Up, P->Velocity.x);//DeltaAngles.y);
            Rot *= QuatAxisAngle(Right, DeltaAngles.x);
            P->Orientation *= Mat4Rotation(Rot);
        }
    }
    
    mat4 PigSca = Mat4Scaling(1.0f + sinf(4.0f * GameState->Time) * 0.25f);
    mat4 PigRot = Mat4RotationY(sinf((r32)GameState->Time));
    mat4 PigTrs = Mat4Translation(4.0f, 0, 0);
    mat4 PigWorld = PigRot * PigTrs * PigSca;

    //
    // NOTE: Render
    //

    Clear(RenderCommands, {0.1f, 0.1f, 0.1f, 1.0f});
    RenderCommands->Projection = Perspective(45.0f + Input->Mouse.Y * 45.0f, (r32)RenderCommands->Width/(r32)RenderCommands->Height, 0.1f, 1000.0f);

    if(GameState->CameraTarget)
    {
        //vec3 Cam = ((Mat4Translation(GameState->Player.Position) * GameState->Player.Orientation) * Vec4(0, 2.0f, -5.0f, 0)).xyz;
        player *P = &GameState->Player;
        RenderCommands->View = LookAt(P->Position + Pfd * -5.0f + Pup * 2.0f, P->Position, Pup);
    }
    else
    {
        r32 Limit = 3.0f * PIOVERFOUR;
        r32 Offset = -PIOVERTWO;
        RenderCommands->View = LookAt(Vec3(cosf(Clamp((Input->Mouse.X * 2.0f - 1.0f) * Limit, -Limit, Limit) + Offset) * -6.0f, 4.0f,
                                           sinf(Clamp((Input->Mouse.X * 2.0f - 1.0f) * Limit, -Limit, Limit) + Offset) * 6.0f),
                                           Vec3(0, 1, 0),
                                           //GameState->CameraTarget ? *GameState->CameraTarget : Vec3(0, 1, 0),
                                           Vec3(0, 1, 0));
    }

    /*
    PushMesh(RenderCommands, GameState->Walls, MAT4_IDENTITY);
    PushMesh(RenderCommands, GameState->Pig, PigWorld);
    PushMesh(RenderCommands, GameState->Tiger, Mat4Translation(0, 0.75f, 0));
    PushMesh(RenderCommands, GameState->Track, Mat4Translation(0, 0, 60.0f) * Mat4Scaling(100.0f));
    */
    PushMesh(RenderCommands, Mat4Translation( 5.0f, 0, 5.0f) * Mat4RotationY(PIOVERFOUR * 3),
            TranState->Assets->Ship, TranState->Assets->ShipMtl);
    Wireframe(RenderCommands, true);
    PushMesh(RenderCommands, Mat4Translation(-5.0f, 0, 5.0f) * Mat4RotationY(-(PIOVERFOUR * 3)),
            TranState->Assets->Ship, TranState->Assets->ShipMtl);
    Wireframe(RenderCommands, false);
    if(GameState->CameraTarget)
        PushMesh(RenderCommands, Mat4Translation(GameState->Player.Position) * GameState->Player.Orientation,
                 TranState->Assets->Ship, TranState->Assets->ShipMtl);
}
