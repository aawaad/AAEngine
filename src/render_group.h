#ifndef RENDER_GROUP_H
#define RENDER_GROUP_H

enum render_entry_type
{
    render_entry_type_clear,
    render_entry_type_mesh,
};

struct render_entry_header
{
    render_entry_type type;
};

struct render_entry_clear
{
    render_entry_header header;
    vec4 colour;
};

struct render_entry_mesh
{
    render_entry_header header;
    mat4 world;
    simple_mesh *mesh;
};

#endif

