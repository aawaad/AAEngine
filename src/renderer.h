#ifndef RENDERER_H
#define RENDERER_H

// NOTE: This is to be overridden in renderer implementations (e.g. renderer_opengl.h)
//struct texture_handle;
//typedef struct texture_handle texture_handle;

// TODO: These shouldn't be here
struct texture_handle
{
    u32 Value;
};

struct mesh_handles
{
    u32 VAO;
    u32 VBuf;
    u32 IBuf;
};

enum texture_filter_type
{
    TextureFilterType_Nearest = GL_NEAREST,
    TextureFilterType_Linear = GL_LINEAR,
};

struct texture;
static texture_handle *RendererCreateTexture(texture *Tex, u32 *Data);
struct mesh;
static mesh_handles *RendererCreateBuffers(mesh *Mesh);

#endif

