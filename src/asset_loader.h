#ifndef ASSET_LOADER_H
#define ASSET_LOADER_H

#include "asset.h"

#include <fstream>
#include <string>
#include <vector>

#pragma pack(push, 1)
struct bitmap_header
{
    u16 FileType;
    u32 FileSize;
    u16 Reserved1;
    u16 Reserved2;
    u32 BitmapOffset;
    u32 Size;
    s32 Width;
    s32 Height;
    u16 Planes;
    u16 BitsPerPixel;
};
#pragma pack(pop)

s32 CountTokens(const WCHAR *Str, WCHAR Tok)
{
	s32 Result = 0;

	while (*Str++ != L'\0')
		if (*Str == Tok) Result++;

	return Result;
}

// NOTE: djb2
u32 ComputeHash(WCHAR *Str, u32 Max)
{
    u32 Hash = 5381;
    s32 c;

    while (c = *Str++)
        Hash = ((Hash << 5) + Hash) + c;

    return (Hash & (Max - 1));
}

static u32 FindTextureSlot(game_assets *Assets, WCHAR *Filename);
static texture *LookupTextureHash(game_assets *Assets, WCHAR *Filename);

static texture *LoadTextureFromBMP(memory_region *Region, game_assets *Assets, WCHAR *Filename,
                                  texture_type Type = TextureType_2D,
                                  texture_filter_type MinFilter = TextureFilterType_Nearest,
                                  texture_filter_type MagFilter = TextureFilterType_Nearest,
                                  texture_addr_mode WrapU = TextureMode_Wrap,
                                  texture_addr_mode WrapV = TextureMode_Wrap,
                                  b32 Lock = false);
static texture *LoadTexture2D(memory_region *Region, game_assets *Assets, WCHAR *Filename, b32 Lock = false);

// TODO: Cleanup this code duplication
static u32 FindMeshSlot(game_assets *Assets, WCHAR *Filename);
static mesh *LookupMeshHash(game_assets *Assets, WCHAR *Filename);

static mesh *LoadMesh(memory_region *Region, game_assets *Assets, WCHAR *Filename, b32 FlipUV = false,
                      primitive_type PrimType = PrimType_Triangles, b32 Lock = false);

// TODO: Cleanup this code duplication
static u32 FindMaterialSlot(game_assets *Assets, WCHAR *Filename);
static material *LookupMaterialHash(game_assets *Assets, WCHAR *Filename);

static material *LoadMaterial(memory_region *Region, game_assets *Assets, WCHAR *Filename, b32 Lock = false);

static void UnloadTexture(memory_region *Region, game_assets *Assets, texture *Texture);
static void UnloadMesh(memory_region *Region, game_assets *Assets, mesh *Mesh);
static void UnloadMaterial(memory_region *Region, game_assets *Assets, material *Material);

#endif

