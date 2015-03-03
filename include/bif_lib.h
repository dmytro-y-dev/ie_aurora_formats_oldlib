#ifndef _BIF_LIB_
#define _BIF_LIB_

#include "types_common.h"
#include "bits_funcs.h"
#include <vector>

using namespace std;
struct BifFileRes
{
	uint32 LocationFlags;
	uint32 Offset;
	uint32 Size;
	uint16 Type;
	uint16 Unknown;
	uint8 *Content;

	uint16 GetSourceIndex()
	{
		uint16 SourceIndex = 0;
		for (uint8 j = 0; j < 12; j++)
			SourceIndex = SetBit(SourceIndex, j, GetBit(LocationFlags, j + 20));
		return SourceIndex;
	}

	uint16 GetTilesetIndex()
	{
		uint16 SourceIndex = 0;
		for (uint8 j = 0; j < 6; j++)
			SourceIndex = SetBit(SourceIndex, j, GetBit(LocationFlags, j + 14));
		return SourceIndex;
	}

	uint16 GetNonTilesetIndex()
	{
		uint16 SourceIndex = 0;
		for (uint8 j = 0; j < 14; j++)
			SourceIndex = SetBit(SourceIndex, j, GetBit(LocationFlags, j));
		return SourceIndex;
	}

	void SetSourceIndex(const uint16 &Index)
	{
		for (uint8 j = 0; j < 12; j++)
			LocationFlags = SetBit(LocationFlags, j + 20, GetBit(Index, j));
	}

	void SetTilesetIndex(const uint16 &Index)
	{
		for (uint8 j = 0; j < 6; j++)
			LocationFlags = SetBit(LocationFlags, j + 14, GetBit(Index, j));
	}

	void SetNonTilesetIndex(const uint16 &Index)
	{
		for (uint8 j = 0; j < 14; j++)
			LocationFlags = SetBit(LocationFlags, j, GetBit(Index, j));
	}
};

struct BifTileRes
{
	uint32 LocationFlags;
	uint32 Offset;
	uint32 TilesCount;
	uint32 Size;
	uint16 Type;
	uint16 Unknown;
	uint8 *Content;

	uint16 GetSourceIndex()
	{
		uint16 SourceIndex = 0;
		for (uint8 j = 0; j < 12; j++)
			SourceIndex = SetBit(SourceIndex, j, GetBit(LocationFlags, j + 20));
		return SourceIndex;
	}

	uint16 GetTilesetIndex()
	{
		uint16 SourceIndex = 0;
		for (uint8 j = 0; j < 6; j++)
			SourceIndex = SetBit(SourceIndex, j, GetBit(LocationFlags, j + 14));
		return SourceIndex;
	}

	uint16 GetNonTilesetIndex()
	{
		uint16 SourceIndex = 0;
		for (uint8 j = 0; j < 14; j++)
			SourceIndex = SetBit(SourceIndex, j, GetBit(LocationFlags, j));
		return SourceIndex;
	}

	void SetSourceIndex(const uint16 &Index)
	{
		for (uint8 j = 0; j < 12; j++)
			LocationFlags = SetBit(LocationFlags, j + 20, GetBit(Index, j));
	}

	void SetTilesetIndex(const uint16 &Index)
	{
		for (uint8 j = 0; j < 6; j++)
			LocationFlags = SetBit(LocationFlags, j + 14, GetBit(Index, j));
	}

	void SetNonTilesetIndex(const uint16 &Index)
	{
		for (uint8 j = 0; j < 14; j++)
			LocationFlags = SetBit(LocationFlags, j, GetBit(Index, j));
	}
};

class BifFile
{
public:
	vector <BifFileRes> Files;
	vector <BifTileRes> Tiles;
public:
	~BifFile() { Clear(); }

	bool LoadFile(LPCSTR path);
	bool SaveFile(LPCSTR path);

	void Clear();
};

#endif // _KEY_LIB_