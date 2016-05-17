#ifndef RENDER_GROUP_H
#define RENDER_GROUP_H

enum render_entry_type
{
    RenderEntry_Clear,
    RenderEntry_Wireframe,
    RenderEntry_Mesh,
};

struct render_entry_header
{
    render_entry_type Type;
};

struct render_entry_clear
{
    render_entry_header Header;
    vec4 Colour;
};

struct render_entry_wireframe
{
    render_entry_header Header;
    b32 Enabled;
};

struct render_entry_mesh
{
    render_entry_header Header;
    mat4 World;
    mesh *Mesh;
    material *Material;
};

#endif

