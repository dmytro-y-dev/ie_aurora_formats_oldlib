#include "bif_lib.h"
#include "strfuncs.h"

const char szBIFDefaultSign[] = "BIFFV1  ";

void BifFile::Clear()
{
	for (uint32 i = 0; i < Files.size(); i++)
		delete []Files[i].Content;

	for (uint32 i = 0; i < Tiles.size(); i++)
		delete []Tiles[i].Content;

	Files.clear();
	Tiles.clear();
}

bool BifFile::LoadFile(LPCSTR path)
{
	FILE *fIn = fopen(path, "rb");
	if (!fIn) return false;

	char Sign[8];
	fread(Sign, sizeof(char), 8, fIn);
	if (!strSubEquale(Sign, 0, szBIFDefaultSign, 0, 8))
	{
		fclose(fIn);
		return false;
	}

	Clear();
	uint32 uFilesCount, uTilesCount, uOffset;
	fread(&uFilesCount, sizeof(uint32), 1, fIn);
	fread(&uTilesCount, sizeof(uint32), 1, fIn);
	fread(&uOffset, sizeof(uint32), 1, fIn);

	for (uint32 i = 0; i < uFilesCount; i++)
	{
		fseek(fIn, uOffset + i * 16, 0);
		BifFileRes file;
		fread(&file.LocationFlags, sizeof(uint32), 1, fIn);
		fread(&file.Offset, sizeof(uint32), 1, fIn);
		fread(&file.Size, sizeof(uint32), 1, fIn);
		fread(&file.Type, sizeof(uint16), 1, fIn);
		fread(&file.Unknown, sizeof(uint16), 1, fIn);

		fseek(fIn, file.Offset, 0);

		file.Content = new uint8[file.Size];
		fread(file.Content, sizeof(uint8), file.Size, fIn);

		Files.push_back(file);
	}

	uOffset = uOffset + uFilesCount * 16;
	for (uint32 i = 0; i < uTilesCount; i++)
	{
		fseek(fIn, uOffset + i * 20, 0);

		BifTileRes file;
		fread(&file.LocationFlags, sizeof(uint32), 1, fIn);
		fread(&file.Offset, sizeof(uint32), 1, fIn);
		fread(&file.TilesCount, sizeof(uint32), 1, fIn);
		fread(&file.Size, sizeof(uint32), 1, fIn);
		fread(&file.Type, sizeof(uint16), 1, fIn);
		fread(&file.Unknown, sizeof(uint16), 1, fIn);

		fseek(fIn, file.Offset, 0);
		file.Content = new uint8[file.Size * file.TilesCount];
		fread(file.Content, sizeof(uint8), file.Size * file.TilesCount, fIn);

		Tiles.push_back(file);
	}
	fclose(fIn);
	return true;
}

bool BifFile::SaveFile(LPCSTR path)
{
	FILE *fOut = fopen(path, "wb");
	if (!fOut) return false;
	fwrite(szBIFDefaultSign, sizeof(char), 8 , fOut);

	uint32 uFilesCount = Files.size(), uTilesCount = Tiles.size(), uOffset = 20;
	fwrite(&uFilesCount, sizeof(uint32), 1, fOut);
	fwrite(&uTilesCount, sizeof(uint32), 1, fOut);
	fwrite(&uOffset, sizeof(uint32), 1, fOut);

	uint32 uPrevLen = 20 + uFilesCount * 16 + uTilesCount * 20;
	for (uint32 i = 0; i < uFilesCount; i++)
	{
		Files[i].Offset = uPrevLen;
		fwrite(&Files[i].LocationFlags, sizeof(uint32), 1, fOut);
		fwrite(&Files[i].Offset, sizeof(uint32), 1, fOut);
		fwrite(&Files[i].Size, sizeof(uint32), 1, fOut);
		fwrite(&Files[i].Type, sizeof(uint16), 1, fOut);
		fwrite(&Files[i].Unknown, sizeof(uint16), 1, fOut);
		uPrevLen += Files[i].Size;
	}

	for (uint32 i = 0; i < uTilesCount; i++)
	{
		Tiles[i].Offset = uPrevLen;
		fwrite(&Tiles[i].LocationFlags, sizeof(uint32), 1, fOut);
		fwrite(&Tiles[i].Offset, sizeof(uint32), 1, fOut);
		fwrite(&Tiles[i].TilesCount, sizeof(uint32), 1, fOut);
		fwrite(&Tiles[i].Size, sizeof(uint32), 1, fOut);
		fwrite(&Tiles[i].Type, sizeof(uint16), 1, fOut);
		fwrite(&Tiles[i].Unknown, sizeof(uint16), 1, fOut);
		uPrevLen += Tiles[i].Size * Tiles[i].TilesCount;
	}

	for (uint32 i = 0; i < uFilesCount; i++) fwrite(Files[i].Content, sizeof(uint8), Files[i].Size, fOut);
	for (uint32 i = 0; i < uTilesCount; i++) fwrite(Tiles[i].Content, sizeof(uint8), Tiles[i].Size * Tiles[i].TilesCount, fOut);

	fclose(fOut);
	return true;
}