#ifndef ASSET_H
#define ASSET_H

struct bitmap
{
    u32 width;
    u32 height;
    u32 handle;
    u32 *data;
};

struct vertex
{
    vec3 pos;
    vec2 uv;
    vec3 normal;
};

struct lighting_values
{
    vec4 ambient, diffuse, specular;
	r32 alpha; // 'd'
	s32 specularExp;
	s32 illum;
	//s32 __pad; // 16-byte alignment
};

struct simple_material
{
	WCHAR filename[MAX_PATH];
	WCHAR name[MAX_PATH];
	lighting_values values;
	WCHAR textureName[MAX_PATH];
	//u32 diffuseTex;
	bitmap diffuseTexture;
};

struct simple_mesh
{
	vertex *vertices;
	u16 *indices;
	u16 numVertices;
	u16 numIndices;
	simple_material *material;
	u16 numMaterials;
	u32 vaoHandle;
	u32 vbufHandle;
	u32 ibufHandle;
};

#endif

