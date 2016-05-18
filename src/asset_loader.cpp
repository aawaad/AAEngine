#include "asset_loader.h"

static u32 FindTextureSlot(game_assets *Assets, WCHAR *Filename)
{
    Assert(Assets->TextureCount < ArrayCount(Assets->Textures));
    u32 Hash = ComputeHash(Filename, ArrayCount(Assets->Textures));

    while(Assets->Textures[Hash] && wcscmp(Assets->Textures[Hash]->Meta.Filename, Filename))
    {
        Hash = ++Hash % ArrayCount(Assets->Textures);
    }

    return Hash;
}

static texture *LookupTextureHash(game_assets *Assets, WCHAR *Filename)
{
    texture *Result = nullptr;

    u32 Hash = FindTextureSlot(Assets, Filename);

    if(Assets->Textures[Hash])
    {
        Assert(wcscmp(Filename, Assets->Textures[Hash]->Meta.Filename) == 0);
        Result = Assets->Textures[Hash];
    }

    return Result;
}

static texture *LoadTextureFromBMP(memory_region *Region, game_assets *Assets, WCHAR *Filename,
                                  texture_type Type,
                                  texture_filter_type MinFilter,
                                  texture_filter_type MagFilter,
                                  texture_addr_mode WrapU,
                                  texture_addr_mode WrapV,
                                  b32 Lock)
{
    WCHAR Fullpath[MAX_PATH];
    Platform.FullPathFromRelativeWchar(Fullpath, Filename, MAX_PATH);

    texture *Result = nullptr;

    if(Result = LookupTextureHash(Assets, Fullpath))
    {
        ++Result->Meta.References;
        return Result;
    }

    void *File = Platform.ReadEntireFile(Fullpath);

    if(File)
    {
        bitmap_header *Header = (bitmap_header *)File;

        if((Header->FileSize >= 54) && (*(u8 *)File == 'B') && (*((u8 *)File + 1) == 'M'))
        {
            Result = AllocStruct(Region, texture);
            Result->Meta.Filename = (WCHAR *)AllocSize(Region, sizeof(WCHAR) * wcslen(Fullpath) + 2);
            wcscpy(Result->Meta.Filename, Fullpath);

            Result->Type = Type;
            Result->MinFilterType = MinFilter;
            Result->MagFilterType = MagFilter;
            Result->WrapModeU = WrapU;
            Result->WrapModeV = WrapV;
            Result->Width = Header->Width;
            Result->Height = Header->Height;

            u32 *Data = (u32 *)((u8 *)File + Header->BitmapOffset);
            //Result->Data = (u32 *)((u8 *)File + Header->BitmapOffset);
            Result->Handle = RendererCreateTexture(Result, Data);

            Result->Meta.State = Lock ? AssetState_Locked : AssetState_Loaded; // heh
            Result->Meta.References = 1;

            u32 Slot = FindTextureSlot(Assets, Filename);
            Assets->Textures[Slot] = Result;
            ++Assets->TextureCount;
        }
        Platform.FreeFile(File);
    }

    return Result;
}

static texture *LoadTexture2D(memory_region *Region, game_assets *Assets, WCHAR *Filename, b32 Lock)
{
    texture *Result = nullptr;

    Result = LoadTextureFromBMP(Region, Assets, Filename, TextureType_2D,
                                TextureFilterType_Linear,
                                TextureFilterType_Linear,
                                TextureMode_Wrap,
                                TextureMode_Wrap,
                                Lock);
    return Result;
}

// TODO: Cleanup this code duplication
static u32 FindMeshSlot(game_assets *Assets, WCHAR *Filename)
{
    Assert(Assets->TextureCount < ArrayCount(Assets->Meshes));
    u32 Hash = ComputeHash(Filename, ArrayCount(Assets->Meshes));

    while(Assets->Meshes[Hash] && wcscmp(Assets->Meshes[Hash]->Meta.Filename, Filename))
    {
        Hash = ++Hash % ArrayCount(Assets->Meshes);
    }

    return Hash;
}

static mesh *LookupMeshHash(game_assets *Assets, WCHAR *Filename)
{
    mesh *Result = nullptr;
    u32 Hash = ComputeHash(Filename, ArrayCount(Assets->Meshes));

    if(Assets->Meshes[Hash])
    {
        Assert(wcscmp(Filename, Assets->Meshes[Hash]->Meta.Filename) == 0);
        Result = Assets->Meshes[Hash];
    }

    return Result;
}

static mesh *LoadMesh(memory_region *Region, game_assets *Assets, WCHAR *Filename, b32 FlipUV,
                      primitive_type PrimType, b32 Lock)
{
    WCHAR Fullpath[MAX_PATH];
    Platform.FullPathFromRelativeWchar(Fullpath, Filename, MAX_PATH);

    mesh *Result;
    if(Result = LookupMeshHash(Assets, Fullpath))
    {
        ++Result->Meta.References;
        return Result;
    }

	std::wifstream FStream;
	std::wstring Line;
	std::vector<vec3> Vertices(0);
	std::vector<vec2> UVs(0);
	std::vector<vec3> Normals(0);
	std::vector<u16> Indices(0);
	std::vector<u16> UVIndices(0);
	std::vector<u16> NormalIndices(0);

	// Get just the path
	WCHAR Path[MAX_PATH];
	wcscpy(Path, Filename);
	WCHAR *LastSlash = wcsrchr(Path, L'\\');
	if (LastSlash)
		*(LastSlash + 1) = L'\0';
	else
    {
        LastSlash = wcsrchr(Path, L'/');
        if (LastSlash)
            *(LastSlash +1) = L'\0';
        else
		    *Path = L'\0';
    }

	FStream.open(Filename);

    if(!FStream.good())
    {
        return nullptr;
    }

	Result = (mesh *)AllocStruct(Region, mesh);
	ZeroMemory(Result, sizeof(mesh));

    Result->Meta.References = 1;
    Result->Meta.Filename = (WCHAR *)AllocSize(Region, sizeof(WCHAR) * wcslen(Fullpath) + 2);
    wcscpy(Result->Meta.Filename, Fullpath);
    Result->Meta.State = Lock ? AssetState_Locked : AssetState_Loaded; 
    Result->PrimType = PrimType;

	WCHAR First[8];
	r32 V1, V2, V3;

	// try to load the main OBJ file
	while(std::getline(FStream, Line))
	{
		WCHAR *wcLine = new WCHAR[Line.size() + 1];
		std::copy(Line.begin(), Line.end(), wcLine);
		wcLine[Line.size()] = '\0';
		WCHAR *Part = wcstok(wcLine, L" ");

        if(wcslen(wcLine) == 0) { continue; }

		if (wcscmp(Part, L"v") == 0)
		{
			swscanf(Line.c_str(), L"%s%f%f%f", First, &V1, &V2, &V3);

			Vertices.push_back({ V1, V2, V3 });
		}
		else if (wcscmp(Part, L"vt") == 0)
		{
			swscanf(Line.c_str(), L"%s%f%f", First, &V1, &V2);

			UVs.push_back({ V1, V2 });
		}
		else if (wcscmp(Part, L"vn") == 0)
		{
			swscanf(Line.c_str(), L"%s%f%f%f", First, &V1, &V2, &V3);

			Normals.push_back({ V1, V2, V3 });
		}
		else if (wcscmp(Part, L"f") == 0)
		{
			s32 Slashes = CountTokens(Line.c_str(), L'/');
			
			if (Slashes == 0) // v
			{
				while ((Part = wcstok(NULL, L" ")) != NULL)
				{
					Indices.push_back((u16)_wtoi(Part) - 1);
				}
			}
			else if (Slashes == 3) // v/vt
			{
				while ((Part = wcstok(NULL, L" /")) != NULL)
				{
					Indices.push_back((u16)_wtoi(Part) - 1);
					Part = wcstok(NULL, L" /");
					UVIndices.push_back((u16)_wtoi(Part) - 1);
				}
			}
			else if (Slashes == 6) // v/vt/vn, v//vn
			{
				while ((Part = wcstok(NULL, L" /")) != NULL)
				{
					Indices.push_back((u16)_wtoi(Part) - 1);
					Part = wcstok(NULL, L" /");
					UVIndices.push_back((u16)_wtoi(Part) - 1);
					Part = wcstok(NULL, L" /");
					NormalIndices.push_back((u16)_wtoi(Part) - 1);
				}
			}
		}
		else if (wcscmp(Part, L"mtllib") == 0)
		{
			Part = wcstok(NULL, L" ");
		}
		else if (wcscmp(Part, L"usemtl") == 0)
		{
			Part = wcstok(NULL, L" ");
		}

		delete wcLine;
	}

	// Parse the indices
	// Append the new vertex and its index only if it's unique
	// Otherwise just append the old index to the list of indices
	u16 *idxTable = new u16[Indices.size()];
	std::vector<vertex> tempVertices(0);
	std::vector<u16> tempIndices(0);
	u16 currentIdx = 0;
	for (s32 i = 0; i < Indices.size(); ++i)
	{
		// Check for duplicates
		s32 prevIdx = -1;
		for (s32 j = 0; j < i; j++)
		{
			if (Indices[j] == Indices[i]
				&& UVIndices[j] == UVIndices[i]
				&& NormalIndices[j] == NormalIndices[i])
			{
				prevIdx = j;
				break;
			}
		}

		// Add vertex and index if the new set of indices is unique
		if (prevIdx == -1)
		{
			idxTable[i] = currentIdx;
			vertex V = {};

			V.Pos = Vertices[Indices[i]];
			if (UVIndices[i] >= 0)
				V.UV = UVs[UVIndices[i]];

			tempVertices.push_back(V);
			tempIndices.push_back(currentIdx);
			currentIdx++;
		}
		else
			tempIndices.push_back(idxTable[prevIdx]);
	}
	delete idxTable;

	// Fill the result vertex and index arrays from the temp arrays
	Result->VertexCount = (u16)tempVertices.size();
	Result->Vertices = (vertex *)AllocSize(Region, sizeof(vertex) * Result->VertexCount); //new vertex[Result->VertexCount];
	for (s32 i = 0; i < Result->VertexCount; ++i)
	{
		Result->Vertices[i] = tempVertices[i];

        if(FlipUV)
        {
            Result->Vertices[i].UV.y = -Result->Vertices[i].UV.y;
        }
	}

	Result->IndexCount = (u16)tempIndices.size();
	Result->Indices = (u16 *)AllocSize(Region, sizeof(u16) * Result->IndexCount); //new u16[Result->IndexCount];
	for (s32 i = 0; i < Result->IndexCount; ++i)
		Result->Indices[i] = tempIndices[i];

	FStream.close();

    Result->Handles = RendererCreateBuffers(Result);

    u32 Slot = FindMeshSlot(Assets, Result->Meta.Filename);
    Assets->Meshes[Slot] = Result;
    ++Assets->MeshCount;

	return Result;
}

// TODO: Cleanup this code duplication
static u32 FindMaterialSlot(game_assets *Assets, WCHAR *Filename)
{
    Assert(Assets->TextureCount < ArrayCount(Assets->Materials));
    u32 Hash = ComputeHash(Filename, ArrayCount(Assets->Materials));

    while(Assets->Materials[Hash] && wcscmp(Assets->Materials[Hash]->Meta.Filename, Filename))
    {
        Hash = ++Hash % ArrayCount(Assets->Materials);
    }

    return Hash;
}

static material *LookupMaterialHash(game_assets *Assets, WCHAR *Filename)
{
    material *Result = nullptr;
    u32 Hash = ComputeHash(Filename, ArrayCount(Assets->Materials));

    if(Assets->Materials[Hash])
    {
        Assert(wcscmp(Filename, Assets->Materials[Hash]->Meta.Filename) == 0);
        Result = Assets->Materials[Hash];
    }

    return Result;
}

static material *LoadMaterial(memory_region *Region, game_assets *Assets, WCHAR *Filename, b32 Lock) 
{
    WCHAR Fullpath[MAX_PATH];
    Platform.FullPathFromRelativeWchar(Fullpath, Filename, MAX_PATH);

    material *Result;
    if(Result = LookupMaterialHash(Assets, Fullpath))
    {
        ++Result->Meta.References;
        return Result;
    }

	// Get just the path
    WCHAR Path[MAX_PATH];
	wcscpy(Path, Fullpath);
	WCHAR *LastSlash = wcsrchr(Path, L'\\');
	if (LastSlash)
		*(LastSlash + 1) = L'\0';
	else
    {
        LastSlash = wcsrchr(Path, L'/');
        if (LastSlash)
            *(LastSlash +1) = L'\0';
        else
		    *Path = L'\0';
    }

	std::wifstream FStream;
	std::wstring Line;
    FStream.open(Fullpath);
    WCHAR First[8];

    if(!FStream.good())
    {
        return nullptr;
    }

    Result = (material *)AllocStruct(Region, material);
    Result->Meta.Filename = (WCHAR *)AllocSize(Region, sizeof(WCHAR) * wcslen(Fullpath) + 2);
    wcscpy(Result->Meta.Filename, Fullpath);
    Result->Meta.References = 1;
    Result->Meta.State = Lock ? AssetState_Locked : AssetState_Loaded;

    while (std::getline(FStream, Line))
    {
        WCHAR *wcLine = new WCHAR[Line.size() + 1];
        std::copy(Line.begin(), Line.end(), wcLine);
        wcLine[Line.size()] = '\0';
        WCHAR *Part = wcstok(wcLine, L" ");

        if (Part == nullptr)
            continue;

        if (wcscmp(Part, L"Ka") == 0)
        {
            swscanf(Line.c_str(), L"%s%f%f%f", First,
                    &Result->Values.Ambient.x,
                    &Result->Values.Ambient.y,
                    &Result->Values.Ambient.z);
        }
        if (wcscmp(Part, L"Kd") == 0)
        {
            swscanf(Line.c_str(), L"%s%f%f%f", First,
                    &Result->Values.Diffuse.x,
                    &Result->Values.Diffuse.y,
                    &Result->Values.Diffuse.z);
        }
        if (wcscmp(Part, L"Ks") == 0)
        {
            swscanf(Line.c_str(), L"%s%f%f%f", First,
                    &Result->Values.Specular.x,
                    &Result->Values.Specular.y,
                    &Result->Values.Specular.z);
        }
        if (wcscmp(Part, L"illum") == 0)
        {
            swscanf(Line.c_str(), L"%s%d", First, &Result->Values.Illum);
        }
        if (wcscmp(Part, L"Ns") == 0)
        {
            swscanf(Line.c_str(), L"%s%d", First, &Result->Values.SpecularExp);
        }
        if (wcscmp(Part, L"map_Kd") == 0)
        {
            Part = wcstok(NULL, L" ");
            wcscpy(Fullpath, Part);
        }
    }

    FStream.close();

    u32 Slot = FindMaterialSlot(Assets, Filename);
    Assets->Materials[Slot] = Result;
    ++Assets->MaterialCount;

    // Load the texture and store it with the material
    if (wcscmp(Fullpath, L"") != 0)
    {
        wcscat(Path, Fullpath);
        Result->DiffuseTexture = LoadTextureFromBMP(Region, Assets, Path,
                                                            TextureType_2D,
                                                            TextureFilterType_Linear,
                                                            TextureFilterType_Linear);
    }

    return Result;
}

static void UnloadTexture(memory_region *Region, game_assets *Assets, texture *Texture)
{
    if(Texture->Meta.References <= 1)
    {
        u32 Slot = FindTextureSlot(Assets, Texture->Meta.Filename);
        if(Assets->Textures[Slot] == Texture)
        {
            Assets->Textures[Slot] = nullptr;
        }
        DeallocSize(Region, Texture->Meta.Filename, sizeof(WCHAR) * wcslen(Texture->Meta.Filename));
        DeallocSize(Region, Texture, sizeof(texture));
    }
}

static void UnloadMesh(memory_region *Region, game_assets *Assets, mesh *Mesh)
{
    if(Mesh->Meta.References <= 1)
    {
        u32 Slot = FindMeshSlot(Assets, Mesh->Meta.Filename);
        if(Assets->Meshes[Slot] == Mesh)
        {
            Assets->Meshes[Slot] = nullptr;
        }
        DeallocSize(Region, Mesh->Meta.Filename, sizeof(WCHAR) * wcslen(Mesh->Meta.Filename));
        DeallocSize(Region, Mesh, sizeof(mesh));
    }
}

static void UnloadMaterial(memory_region *Region, game_assets *Assets, material *Material)
{
    if(Material->Meta.References <= 1)
    {
        if(Material->DiffuseTexture)
        {
            UnloadTexture(Region, Assets, Material->DiffuseTexture);
        }

        u32 Slot = FindMaterialSlot(Assets, Material->Meta.Filename);
        if(Assets->Materials[Slot] == Material)
        {
            Assets->Materials[Slot] = nullptr;
        }

        DeallocSize(Region, Material->Meta.Filename, sizeof(WCHAR) * wcslen(Material->Meta.Filename));
        DeallocSize(Region, Material, sizeof(material));
    }
    else
    {
        --Material->Meta.References;
    }
}

