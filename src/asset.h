#ifndef ASSET_H
#define ASSET_H

#include "renderer.h"

enum asset_state
{
    AssetState_Unloaded,
    AssetState_Loading,
    AssetState_Loaded,
    AssetState_Locked,
};

struct asset_meta
{
    WCHAR *Filename;
    u32 References;
    asset_state State;
};

enum texture_type
{
    TextureType_1D,
    TextureType_2D,
    TextureType_3D,
    TextureType_Cubemap,
};

//enum texture_filter_type;
//{
//    TextureFilterType_Nearest,
//    TextureFilterType_Linear,
//};

enum texture_addr_mode
{
    TextureMode_Wrap,
    TextureMode_Clamp,
};

struct texture
{
    asset_meta Meta;
    texture_type Type;
    texture_filter_type MinFilterType;
    texture_filter_type MagFilterType;
    texture_addr_mode WrapModeU;
    texture_addr_mode WrapModeV;
    u32 Width;
    u32 Height;

    texture_handle *Handle;

	// TODO: Remove this and handle renderer-specific stuff generically somehow
	//       Maybe have the renderer/platform define a struct which this uses
    //u32 Handle;

    // NOTE: I want to no longer store bitmaps in RAM, just send them straight to the GPU
    u32 *Data;
};

struct vertex
{
    vec3 Pos;
    vec2 UV;
    vec3 Normal;
};

struct lighting_values
{
    vec4 Ambient, Diffuse, Specular;
	r32 Alpha; // 'd'
	s32 SpecularExp;
	s32 Illum;
};

struct material
{
	asset_meta Meta;
	lighting_values Values;
	texture *DiffuseTexture;
};

enum primitive_type
{
    PrimType_Points,
    PrimType_Line,
    PrimType_LineStrip,
    PrimType_Triangles,
    PrimType_TriangleStrip,
};

struct mesh
{
	asset_meta Meta;
	primitive_type PrimType;
	vertex *Vertices;
	u16 VertexCount;
	u16 *Indices;
	u16 IndexCount;

	// TODO: Remove these and handle renderer-specific stuff generically somehow
	//       Maybe have the renderer/platform define a struct which this uses
	/*
	u32 VAOHandle;
	u32 VBufHandle;
	u32 IBufHandle;
	*/

	mesh_handles *Handles;
};

struct game_assets
{
    texture Textures[2048];
    u32 TextureCount;

    material Materials[512];
    u32 MaterialCount;

    mesh Meshes[512];
    u32 MeshCount;

    mesh *Ship;
    material *ShipMtl;
};

#endif

