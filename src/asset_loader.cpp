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

static texture *LoadTextureFromBMP(memory_region *Region, WCHAR *Filename,
                                  texture_type Type = TextureType_2D,
                                  texture_filter_type MinFilter = TextureFilterType_Nearest,
                                  texture_filter_type MagFilter = TextureFilterType_Nearest,
                                  texture_addr_mode WrapU = TextureMode_Wrap,
                                  texture_addr_mode WrapV = TextureMode_Wrap,
                                  b32 Lock = false)
{
    texture *Result = nullptr;

    WCHAR Fullpath[MAX_PATH];
    Platform.FullPathFromRelativeWchar(Fullpath, Filename, MAX_PATH);

    void *File = Platform.ReadEntireFile(Fullpath);

    if(File)
    {
        bitmap_header *Header = (bitmap_header *)File;

        if((Header->FileSize >= 54) && (*(u8 *)File == 'B') && (*((u8 *)File + 1) == 'M'))
        {
            Result = AllocStruct(Region, texture);
            Result->Meta.Filename = (WCHAR *)AllocSize(Region, sizeof(WCHAR) * wcslen(Fullpath));
            wcscpy(Result->Meta.Filename, Fullpath);

            Result->Type = Type;
            Result->MinFilterType = MinFilter;
            Result->MagFilterType = MagFilter;
            Result->WrapModeU = WrapU;
            Result->WrapModeV = WrapV;
            Result->Width = Header->Width;
            Result->Height = Header->Height;

            //u32 *Data = (u32 *)((u8 *)File + Header->BitmapOffset);
            Result->Data = (u32 *)((u8 *)File + Header->BitmapOffset);
            Result->Handle = RendererCreateTexture(Result, Result->Data);

            Result->Meta.State = Lock ? AssetState_Locked : AssetState_Loaded; // heh
            Result->Meta.References = 1;
        }
    }

    return Result;
}

static mesh *LoadMesh(memory_region *Region, WCHAR *Filename, b32 FlipUV = false,
                      primitive_type PrimType = PrimType_Triangles, b32 Lock = false)
{
	std::wifstream FStream;
	std::wstring Line;
	std::vector<vec3> Vertices(0);
	std::vector<vec2> UVs(0);
	std::vector<vec3> Normals(0);
	std::vector<u16> Indices(0);
	std::vector<u16> UVIndices(0);
	std::vector<u16> NormalIndices(0);

    WCHAR Fullpath[MAX_PATH];
    Platform.FullPathFromRelativeWchar(Fullpath, Filename, MAX_PATH);

	mesh *Result = (mesh *)AllocStruct(Region, mesh);
	ZeroMemory(Result, sizeof(mesh));

    Result->Meta.Filename = (WCHAR *)AllocSize(Region, sizeof(WCHAR) * wcslen(Fullpath));
    wcscpy(Result->Meta.Filename, Fullpath);
    Result->Meta.References = 1;
    Result->Meta.State = Lock ? AssetState_Locked : AssetState_Loaded; 
    Result->PrimType = PrimType;

	// Get just the path
	//WCHAR Path[MAX_PATH];
	wcscpy(Fullpath, Filename);
	WCHAR *LastSlash = wcsrchr(Fullpath, L'\\');
	if (LastSlash)
		*(LastSlash + 1) = L'\0';
	else
    {
        LastSlash = wcsrchr(Fullpath, L'/');
        if (LastSlash)
            *(LastSlash +1) = L'\0';
        else
		    *Fullpath = L'\0';
    }

	FStream.open(Filename);

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
			//wcscpy(Result->Material->Metadata.Filename, Part);
		}
		else if (wcscmp(Part, L"usemtl") == 0)
		{
			Part = wcstok(NULL, L" ");
			//wcscpy(Result->Material->name, Part);
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
			//V.UV.x = 1.0f - V.UV.x;
			//V.UV.y = 1.0f - V.UV.y;
			if (NormalIndices[i] >= 0)
				V.Normal = Normals[NormalIndices[i]];

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
	Result->Vertices = new vertex[Result->VertexCount];
	/*
	Result->Pos = new vec3[Result->VertexCount];
	Result->Normal = new vec3[Result->VertexCount];
	Result->UV = new vec2[Result->VertexCount];
	*/
	for (s32 i = 0; i < Result->VertexCount; ++i)
	{
		Result->Vertices[i] = tempVertices[i];
		/*
		Result->Pos[i] = tempVertices[i].Pos;
		Result->Normal[i] = tempVertices[i].Normal;
		Result->UV[i] = tempVertices[i].UV;
		*/

        if(FlipUV)
        {
            Result->Vertices[i].UV.y = -Result->Vertices[i].UV.y;
    		//Result->uv[i].y = -Result->uv[i].y;
        }
	}

	Result->IndexCount = (u16)tempIndices.size();
	Result->Indices = new u16[Result->IndexCount];
	for (s32 i = 0; i < Result->IndexCount; ++i)
		Result->Indices[i] = tempIndices[i];

	FStream.close();

    Result->Handles = RendererCreateBuffers(Result);

	return Result;
}

static material *LoadMaterial(memory_region *Region, WCHAR *Filename, b32 Lock = false) 
{
    material *Result = (material *)AllocStruct(Region, material);

    WCHAR Fullpath[MAX_PATH];
    Platform.FullPathFromRelativeWchar(Fullpath, Filename, MAX_PATH);

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

    Result->Meta.Filename = (WCHAR *)AllocSize(Region, sizeof(WCHAR) * wcslen(Fullpath));
    wcscpy(Result->Meta.Filename, Fullpath);
    Result->Meta.References = 1;
    Result->Meta.State = Lock ? AssetState_Locked : AssetState_Loaded;

	std::wifstream FStream;
	std::wstring Line;
    FStream.open(Fullpath);
    WCHAR First[8];

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

    // Load the texture and store it with the material
    if (wcscmp(Fullpath, L"") != 0)
    {
        wcscat(Path, Fullpath);
        Result->DiffuseTexture = LoadTextureFromBMP(Region, Path,
                                                            TextureType_2D,
                                                            TextureFilterType_Linear,
                                                            TextureFilterType_Linear);
    }

    return Result;
}

