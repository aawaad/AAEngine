#include "render_group.h"

inline void *PushRenderElement(game_render_commands *commands, u32 size)
{
    void *result = 0;

    if((commands->pushBufferSize + size) < (commands->maxPushBufferSize))
    {
        result = commands->pushBufferBase + commands->pushBufferSize;
        commands->pushBufferSize += size;
    }
    else
    {
        Assert(!"Ran out of render commands memory!");
    }

    return result;
}

inline void PushMesh(game_render_commands *commands, simple_mesh *mesh, mat4 world)
{
    render_entry_mesh *entry = (render_entry_mesh *)PushRenderElement(commands, sizeof(render_entry_mesh));
    if(entry)
    {
        entry->header.type = render_entry_type_mesh;
        entry->world = world;
        entry->mesh = mesh;
    }
}

inline void Clear(game_render_commands *commands, vec4 colour)
{
    render_entry_clear *entry = (render_entry_clear *)PushRenderElement(commands, sizeof(render_entry_clear));
    if(entry)
    {
        entry->header.type = render_entry_type_clear;
        entry->colour = colour;
    }
}

