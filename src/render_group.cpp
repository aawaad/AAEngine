#include "render_group.h"

inline void *PushRenderElement(game_render_commands *Commands, u32 Size)
{
    void *Result = 0;

    if((Commands->PushBufferSize + Size) < (Commands->MaxPushBufferSize))
    {
        Result = Commands->PushBufferBase + Commands->PushBufferSize;
        Commands->PushBufferSize += Size;
    }
    else
    {
        Assert(!"Ran out of render commands memory!");
    }

    return Result;
}

inline void PushMesh(game_render_commands *Commands, mat4 World, mesh *Mesh, material *Mat)
{
    render_entry_mesh *Entry = (render_entry_mesh *)PushRenderElement(Commands, sizeof(render_entry_mesh));
    if(Entry)
    {
        Entry->Header.Type = RenderEntry_Mesh;
        Entry->World = World;
        Entry->Mesh = Mesh;
        Entry->Material = Mat;
    }
}

inline void Clear(game_render_commands *Commands, vec4 Colour)
{
    render_entry_clear *Entry = (render_entry_clear *)PushRenderElement(Commands, sizeof(render_entry_clear));
    if(Entry)
    {
        Entry->Header.Type = RenderEntry_Clear;
        Entry->Colour = Colour;
    }
}

inline void Wireframe(game_render_commands *Commands, b32 Enabled)
{
    render_entry_wireframe *Entry = (render_entry_wireframe *)PushRenderElement(Commands, sizeof(render_entry_wireframe));
    if(Entry)
    {
        Entry->Header.Type = RenderEntry_Wireframe;
        Entry->Enabled = Enabled;
    }
}

