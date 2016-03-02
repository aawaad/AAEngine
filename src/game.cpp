//#include "AAMath/aamath.h"
#include "game.h"
#include "asset_loader.cpp"
#include "render_group.cpp"
#include "renderer_opengl.cpp"

static void GameUpdate(game_memory *memory, game_input *input, game_render_commands *renderCommands)
{
    Assert((&input->controllers[0].btnCount - &input->controllers[0].buttons[0])
            == ArrayCount(input->controllers[0].buttons));

    Assert(sizeof(game_state) <= memory->permanentMemorySize);
    game_state *gameState = (game_state *)memory->permanentMemory;

    if(!gameState->isInitialised)
    {
        memory_region total;
        CreateRegion(&total, memory->permanentMemorySize - sizeof(game_state),
                     (u8 *)memory->permanentMemory + sizeof(game_state));

        // TODO: Permanent memory regions go here

        gameState->walls = LoadMesh(L"../data/room_thickwalls.obj");
        gameState->pig = LoadMesh(L"../data/pig.obj", true);
        gameState->tiger = LoadMesh(L"../data/tiger.obj", true);

        gameState->isInitialised = true;
    }

    Assert(sizeof(transient_state) <= memory->transientMemorySize);
    transient_state *tranState = (transient_state *)memory->transientMemory;
    if(!tranState->isInitialised)
    {
        CreateRegion(&tranState->transientMemory, memory->transientMemorySize - sizeof(transient_state),
                     (u8 *)memory->transientMemory + sizeof(transient_state));

        tranState->isInitialised = true;
    }

    //
    // NOTE: Update
    //

    gameState->time += input->deltaTime;

    {
        player *p = &gameState->tigerPlayer;
        game_controller *con = &input->controllers[0];

        if(con->start.endedDown)
        {
            gameState->tigerPlayer = {};
            p->position = {0.0f, 0.75f, -1.0f};
            p->orientation = MAT4_IDENTITY;
            p->forward = {0.0f, 0.0f, 1.0f};
            p->up = {0.0f, 1.0f, 0.0f};
            p->right = {1.0f, 0.0f, 0.0f};
            gameState->cameraTarget = &p->position;
        }

        if(gameState->cameraTarget)
        {
            vec3 deltaAngles = {};
            if(con->moveUp.endedDown)
            {
                //deltaAngles.x += 1.0f * input->deltaTime;
                p->velocity.z += 1.0f * input->deltaTime;
            }
            if(con->moveDown.endedDown)
            {
                //deltaAngles.x -= 1.0f * input->deltaTime;
                p->velocity.z -= 1.0f * input->deltaTime;
            }
            if(con->moveRight.endedDown)
            {
                deltaAngles.y += 4.0f * input->deltaTime;
            }
            if(con->moveLeft.endedDown)
            {
                deltaAngles.y -= 4.0f * input->deltaTime;
            }
            if(con->rightShoulder.endedDown)
            {
                //p->velocity.z += 1.0f * input->deltaTime;
            }
            if(con->leftShoulder.endedDown)
            {
                //p->velocity.z -= 1.0f * input->deltaTime;
            }
            if(!con->rightShoulder.endedDown && !con->leftShoulder.endedDown)
            {
                if(abs(p->velocity.z) > EPSILON)
                {
                    p->velocity.z *= 0.9f;
                }
                else
                {
                    p->velocity.z = 0;
                }
            }

            vec3 fwd = Normalized(p->orientation * Vec4(p->forward, 0)).xyz;
            p->position += fwd * p->velocity.z;
            vec3 up = Normalized(p->orientation * Vec4(p->up, 0)).xyz;
            vec3 right = Normalized(Cross(fwd, up));

            quat rot = QuatAxisAngle(fwd, deltaAngles.z);
            rot *= QuatAxisAngle(up, deltaAngles.y);
            rot *= QuatAxisAngle(right, deltaAngles.x);
            p->orientation *= Mat4Rotation(rot);
        }
    }
    
    mat4 pigSca = Mat4Scaling(1.0f + sinf(4.0f * gameState->time) * 0.25f);
    mat4 pigRot = Mat4RotationY(gameState->time);
    mat4 pigTrs = Mat4Translation(4.0f, 0, 0);
    mat4 pigWorld = pigRot * pigTrs * pigSca;

    //
    // NOTE: Render
    //

    Clear(renderCommands, {0.1f, 0.1f, 0.1f, 1.0f});
    renderCommands->projection = Perspective(45.0f + input->mouse.y * 45.0f, (r32)renderCommands->width/(r32)renderCommands->height, 0.1f, 100.0f);
    r32 limit = 3.0f * PIOVERFOUR;
    r32 offset = -PIOVERTWO;
    renderCommands->view = LookAt(Vec3(cosf(Clamp((input->mouse.x * 2.0f - 1.0f) * limit, -limit, limit) + offset) * -6.0f, 4.0f,
                                       sinf(Clamp((input->mouse.x * 2.0f - 1.0f) * limit, -limit, limit) + offset) * 6.0f),
                                       //Vec3(0, 1, 0),
                                       gameState->cameraTarget ? *gameState->cameraTarget : Vec3(0, 1, 0),
                                       Vec3(0, 1, 0));
    
    PushMesh(renderCommands, gameState->walls, MAT4_IDENTITY);
    PushMesh(renderCommands, gameState->pig, pigWorld);
    if(gameState->cameraTarget)
        PushMesh(renderCommands, gameState->tiger, Mat4Translation(gameState->tigerPlayer.position) * gameState->tigerPlayer.orientation);//Mat4Translation(0, 0.75f, -1.0f));
}
