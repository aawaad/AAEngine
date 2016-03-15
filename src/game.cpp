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
        gameState->ship = LoadMesh(L"../data/helicoid_ship.obj");
        gameState->track = LoadMesh(L"../data/helicoid_mobius.obj");

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

    // IMPORTANT: Remove these!
    vec3 pfd, pup, prt;

    gameState->time += input->deltaTime;

    {
        entity_player *p = &gameState->player;
        game_controller *con = &input->controllers[0];

        local_persist r32 frameDelay = 0.2f;
        frameDelay += input->deltaTime;
        if(con->start.endedDown)
        {
            gameState->player = {};
            p->position = {0.0f, 0.75f, -1.0f};
            p->orientation = MAT4_IDENTITY;
            p->forward = {0.0f, 0.0f, 1.0f};
            p->up = {0.0f, 1.0f, 0.0f};
            p->right = {1.0f, 0.0f, 0.0f};
            if(frameDelay > 0.2f)
            {
                gameState->cameraTarget = gameState->cameraTarget ? nullptr : &p->position;
                frameDelay = 0;
            }
        }

        if(gameState->cameraTarget)
        {
            vec3 deltaAngles = {};
            if(con->moveUp.endedDown)
            {
                //deltaAngles.x += 1.0f * input->deltaTime;
                p->velocity.z += 2.0f * input->deltaTime;
            }
            if(con->moveDown.endedDown)
            {
                //deltaAngles.x -= 1.0f * input->deltaTime;
                p->velocity.z -= 2.0f * input->deltaTime;
            }
            if(con->moveRight.endedDown)
            {
                //deltaAngles.y += 4.0f * input->deltaTime;
                p->velocity.x += 0.2f * input->deltaTime;
            }
            if(con->moveLeft.endedDown)
            {
                //deltaAngles.y -= 4.0f * input->deltaTime;
                p->velocity.x -= 0.2f * input->deltaTime;
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
            if(!con->moveRight.endedDown && !con->moveLeft.endedDown)
            {
                if(abs(p->velocity.x) > EPSILON)
                {
                    p->velocity.x *= 0.9f;
                }
                else
                {
                    p->velocity.x = 0;
                }
            }
            if(p->velocity.x > 0.1f) { p->velocity.x = 0.1f; }

            vec3 fwd = Normalized(p->orientation * Vec4(p->forward, 0)).xyz;
            p->position += fwd * p->velocity.z;
            vec3 up = Normalized(p->orientation * Vec4(p->up, 0)).xyz;
            vec3 right = Normalized(Cross(fwd, up));
            pfd = fwd;
            pup = up;
            prt = right;

            quat rot = QuatAxisAngle(fwd, deltaAngles.z);
            rot *= QuatAxisAngle(up, p->velocity.x);//deltaAngles.y);
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
    renderCommands->projection = Perspective(45.0f + input->mouse.y * 45.0f, (r32)renderCommands->width/(r32)renderCommands->height, 0.1f, 1000.0f);

    if(gameState->cameraTarget)
    {
        //vec3 cam = ((Mat4Translation(gameState->player.position) * gameState->player.orientation) * Vec4(0, 2.0f, -5.0f, 0)).xyz;
        entity_player *p = &gameState->player;
        renderCommands->view = LookAt(p->position + pfd * -5.0f + pup * 2.0f, p->position, pup);
    }
    else
    {
        r32 limit = 3.0f * PIOVERFOUR;
        r32 offset = -PIOVERTWO;
        renderCommands->view = LookAt(Vec3(cosf(Clamp((input->mouse.x * 2.0f - 1.0f) * limit, -limit, limit) + offset) * -6.0f, 4.0f,
                                           sinf(Clamp((input->mouse.x * 2.0f - 1.0f) * limit, -limit, limit) + offset) * 6.0f),
                                           Vec3(0, 1, 0),
                                           //gameState->cameraTarget ? *gameState->cameraTarget : Vec3(0, 1, 0),
                                           Vec3(0, 1, 0));
    }
    
    PushMesh(renderCommands, gameState->walls, MAT4_IDENTITY);
    PushMesh(renderCommands, gameState->pig, pigWorld);
    PushMesh(renderCommands, gameState->tiger, Mat4Translation(0, 0.75f, 0));
    PushMesh(renderCommands, gameState->track, Mat4Translation(0, 0, 60.0f) * Mat4Scaling(100.0f));
    if(gameState->cameraTarget)
        PushMesh(renderCommands, gameState->ship, Mat4Translation(gameState->player.position) * gameState->player.orientation);
}
