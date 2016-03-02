#include "asset.h"

#include <fstream>
#include <string>
#include <vector>

// TODO: Remove these and replace with transient memory allocation
//       Could keep as platform debug functions
static void FreeFile(void *file)
{
    if(file)
    {
        //VirtualFree(file, 0, MEM_RELEASE);
        free(file);
    }
}

static void *ReadEntireFile(char *filename)
{
    void *result = 0;

    HANDLE file = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(file != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER size;
        if(GetFileSizeEx(file, &size))
        {
            Assert(size.QuadPart <= 0xFFFFFFFF);
            //result = VirtualAlloc(0, size.LowPart, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            result = malloc(size.LowPart);
            if(result)
            {
                DWORD bytesRead;
                if(ReadFile(file, result, size.LowPart, &bytesRead, 0) && (size.LowPart == bytesRead))
                {
                }
                else
                {
                    FreeFile(file);
                    result = 0;
                }
            }
        }

        CloseHandle(file);
    }

    return result;
}

#pragma pack(push, 1)
struct bitmap_header
{
    u16 fileType;
    u32 fileSize;
    u16 reserved1;
    u16 reserved2;
    u32 bitmapOffset;
    u32 size;
    s32 width;
    s32 height;
    u16 planes;
    u16 bitsPerPixel;
};
#pragma pack(pop)

static bitmap LoadBitmap(char *filename)
{
    bitmap result = {};
    void *file = ReadEntireFile(filename);

    if(file)
    {
        bitmap_header *header = (bitmap_header *)file;

        if((header->fileSize > 54) && (*(u8 *)file == 'B') && (*((u8 *)file + 1) == 'M'))
        {
            result.data = (u32 *)((u8 *)file + header->bitmapOffset);
            result.width = header->width;
            result.height = header->height;
        }
    }

    return result;
}

enum filter_type
{
    filter_type_nearest,
    filter_type_trilinear,
};

static GLuint LoadBitmapToTexture2D(char *filename, filter_type filterType = filter_type_trilinear)
{
    bitmap bmp = LoadBitmap(filename);

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    if(filterType == filter_type_trilinear)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        //glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bmp.width, bmp.height, 0, GL_BGR, GL_UNSIGNED_BYTE, bmp.data);

    return tex;
}

s32 countTokens(const WCHAR *str, WCHAR tok)
{
	s32 result = 0;

	while (*str++ != L'\0')
		if (*str == tok) result++;

	return result;
}

static simple_mesh *LoadMesh(WCHAR *filename, b32 flipUV = false)
{
	std::wifstream fstream;
	std::wstring line;
	std::vector<vec3> vertices(0);
	std::vector<vec2> UVs(0);
	std::vector<vec3> normals(0);
	std::vector<u16> indices(0);
	std::vector<u16> UVIndices(0);
	std::vector<u16> normalIndices(0);

	// Get just the path
	WCHAR path[MAX_PATH];
	wcscpy(path, filename);
	WCHAR *lastSlash = wcsrchr(path, L'\\');
	if (lastSlash)
		*(lastSlash + 1) = L'\0';
	else
    {
        lastSlash = wcsrchr(path, L'/');
        if (lastSlash)
            *(lastSlash +1) = L'\0';
        else
		    *path = L'\0';
    }
	
	fstream.open(filename);

	WCHAR first[8];
	r32 v1, v2, v3;

	simple_mesh *result = new simple_mesh;
	ZeroMemory(result, sizeof(simple_mesh));
	result->material = new simple_material;
	ZeroMemory(result->material, sizeof(simple_material));

	// try to load the main OBJ file
	while(std::getline(fstream, line))
	{
		WCHAR *wcline = new WCHAR[line.size() + 1];
		std::copy(line.begin(), line.end(), wcline);
		wcline[line.size()] = '\0';
		WCHAR *part = wcstok(wcline, L" ");

		if (wcscmp(part, L"v") == 0)
		{
			swscanf(line.c_str(), L"%s%f%f%f", first, &v1, &v2, &v3);

			vertices.push_back({ v1, v2, v3 });
		}
		else if (wcscmp(part, L"vt") == 0)
		{
			swscanf(line.c_str(), L"%s%f%f", first, &v1, &v2);

			UVs.push_back({ v1, v2 });
		}
		else if (wcscmp(part, L"vn") == 0)
		{
			swscanf(line.c_str(), L"%s%f%f%f", first, &v1, &v2, &v3);

			normals.push_back({ v1, v2, v3 });
		}
		else if (wcscmp(part, L"f") == 0)
		{
			s32 slashes = countTokens(line.c_str(), L'/');
			
			if (slashes == 0) // v
			{
				while ((part = wcstok(NULL, L" ")) != NULL)
				{
					indices.push_back((u16)_wtoi(part) - 1);
				}
			}
			else if (slashes == 3) // v/vt
			{
				while ((part = wcstok(NULL, L" /")) != NULL)
				{
					indices.push_back((u16)_wtoi(part) - 1);
					part = wcstok(NULL, L" /");
					UVIndices.push_back((u16)_wtoi(part) - 1);
				}
			}
			else if (slashes == 6) // v/vt/vn, v//vn
			{
				while ((part = wcstok(NULL, L" /")) != NULL)
				{
					indices.push_back((u16)_wtoi(part) - 1);
					part = wcstok(NULL, L" /");
					UVIndices.push_back((u16)_wtoi(part) - 1);
					part = wcstok(NULL, L" /");
					normalIndices.push_back((u16)_wtoi(part) - 1);
				}
			}
		}
		else if (wcscmp(part, L"mtllib") == 0)
		{
			part = wcstok(NULL, L" ");
			wcscpy(result->material->filename, part);
		}
		else if (wcscmp(part, L"usemtl") == 0)
		{
			part = wcstok(NULL, L" ");
			wcscpy(result->material->name, part);
		}

		delete wcline;
	}

	// Parse the indices
	// Append the new vertex and its index only if it's unique
	// Otherwise just append the old index to the list of indices
	u16 *idxTable = new u16[indices.size()];
	std::vector<vertex> tempVertices(0);
	std::vector<u16> tempIndices(0);
	u16 currentIdx = 0;
	for (s32 i = 0; i < indices.size(); ++i)
	{
		// Check for duplicates
		s32 prevIdx = -1;
		for (s32 j = 0; j < i; j++)
		{
			if (indices[j] == indices[i]
				&& UVIndices[j] == UVIndices[i]
				&& normalIndices[j] == normalIndices[i])
			{
				prevIdx = j;
				break;
			}
		}

		// Add vertex and index if the new set of indices is unique
		if (prevIdx == -1)
		{
			idxTable[i] = currentIdx;
			vertex v = {};

			v.pos = vertices[indices[i]];
			if (UVIndices[i] >= 0)
				v.uv = UVs[UVIndices[i]];
			//v.uv.x = 1.0f - v.uv.x;
			//v.uv.y = 1.0f - v.uv.y;
			if (normalIndices[i] >= 0)
				v.normal = normals[normalIndices[i]];

			tempVertices.push_back(v);
			tempIndices.push_back(currentIdx);
			currentIdx++;
		}
		else
			tempIndices.push_back(idxTable[prevIdx]);
	}
	delete idxTable;

	// Fill the result vertex and index arrays from the temp arrays
	result->numVertices = (u16)tempVertices.size();
	result->vertices = new vertex[result->numVertices];
	/*
	result->pos = new vec3[result->numVertices];
	result->normal = new vec3[result->numVertices];
	result->uv = new vec2[result->numVertices];
	*/
	for (s32 i = 0; i < result->numVertices; ++i)
	{
		result->vertices[i] = tempVertices[i];
		/*
		result->pos[i] = tempVertices[i].pos;
		result->normal[i] = tempVertices[i].normal;
		result->uv[i] = tempVertices[i].uv;
		*/

        if(flipUV)
        {
            result->vertices[i].uv.y = -result->vertices[i].uv.y;
    		//result->uv[i].y = -result->uv[i].y;
        }
	}

	result->numIndices = (u16)tempIndices.size();
	result->indices = new u16[result->numIndices];
	for (s32 i = 0; i < result->numIndices; ++i)
		result->indices[i] = tempIndices[i];

	fstream.close();
	
	// If we found a material filename, try to load it
	if (result->material->filename)
	{
		WCHAR fullpath[MAX_PATH];
		wcscpy(fullpath, path);
		wcscat(fullpath, result->material->filename);

		fstream.open(fullpath);
		
		bool foundMaterial = false;

		while (std::getline(fstream, line))
		{
			WCHAR *wcline = new WCHAR[line.size() + 1];
			std::copy(line.begin(), line.end(), wcline);
			wcline[line.size()] = '\0';
			WCHAR *part = wcstok(wcline, L" ");

			if (part == nullptr)
				continue;

			if (foundMaterial)
			{
				if (wcscmp(part, L"Ka") == 0)
				{
					swscanf(line.c_str(), L"%s%f%f%f", first,
						&result->material->values.ambient.x,
						&result->material->values.ambient.y,
						&result->material->values.ambient.z);
				}
				if (wcscmp(part, L"Kd") == 0)
				{
					swscanf(line.c_str(), L"%s%f%f%f", first,
						&result->material->values.diffuse.x,
						&result->material->values.diffuse.y,
						&result->material->values.diffuse.z);
				}
				if (wcscmp(part, L"Ks") == 0)
				{
					swscanf(line.c_str(), L"%s%f%f%f", first,
						&result->material->values.specular.x,
						&result->material->values.specular.y,
						&result->material->values.specular.z);
				}
				if (wcscmp(part, L"illum") == 0)
				{
					swscanf(line.c_str(), L"%s%d", first, &result->material->values.illum);
				}
				if (wcscmp(part, L"Ns") == 0)
				{
					swscanf(line.c_str(), L"%s%d", first, &result->material->values.specularExp);
				}
				if (wcscmp(part, L"map_Kd") == 0)
				{
					part = wcstok(NULL, L" ");
					wcscpy(result->material->textureName, part);
				}
			}

			// Find the relevant material declaration for this mesh
			if (wcscmp(part, L"newmtl") == 0)
			{
				part = wcstok(NULL, L" ");
				if (wcscmp(part, result->material->name) == 0)
				{
					foundMaterial = true;
				}
			}
		}

		fstream.close();

		// Load the texture and store it with the material
		if (wcscmp(result->material->textureName, L"") != 0)
		{
			wcscpy(fullpath, path);
			wcscat(fullpath, result->material->textureName);
			char cpath[MAX_PATH];
			wcstombs(cpath, fullpath, MAX_PATH);
			//result->material->diffuseTex = LoadBitmapToTexture2D(cpath, filter_type_trilinear);
			result->material->diffuseTexture = LoadBitmap(cpath);
		}
	}

	return result;
}

