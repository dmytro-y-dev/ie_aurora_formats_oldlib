#include "key_lib.h"
#include "strfuncs.h"

const char szKEYDefaultSign[] = "KEY V1  ";

void KeyFile::Clear()
{
	for (uint32 i = 0; i < Arhives.size(); i++)
		delete []Arhives[i].Name;
	Arhives.clear();
}

bool KeyFile::LoadFile(LPCSTR path)
{
	FILE *fIn = fopen(path, "rb");
	if (!fIn) return false;

	char Sign[8];
	fread(Sign, sizeof(char), 8, fIn);
	if (!strSubEquale(Sign, 0, szKEYDefaultSign, 0, 8))
	{
		fclose(fIn);
		return false;
	}

	Clear();
	uint32 uBifsCount, uResCount, uOffsetBif, uOffsetRes;
	fread(&uBifsCount, sizeof(uint32), 1, fIn);
	fread(&uResCount, sizeof(uint32), 1, fIn);
	fread(&uOffsetBif, sizeof(uint32), 1, fIn);
	fread(&uOffsetRes, sizeof(uint32), 1, fIn);

	for (uint32 i = 0; i < uBifsCount; i++)
	{
		fseek(fIn, uOffsetBif + 12 * i, 0);

		KeyArhive bif; uint16 NameLength;
		fread(&bif.Size, sizeof(uint32), 1, fIn);
		fread(&bif.Offset, sizeof(uint32), 1, fIn);
		fread(&NameLength, sizeof(uint16), 1, fIn);
		fread(&bif.LocationFlags, sizeof(uint16), 1, fIn);

		fseek(fIn, bif.Offset, 0);

		bif.Name = new char[NameLength];
		fread(bif.Name, sizeof(char), NameLength, fIn);
		Arhives.push_back(bif);
	}

	fseek(fIn, uOffsetRes, 0);
	for (uint32 i = 0; i < uResCount; i++)
	{
		KeyResource res;
		for (uint32 j = 0; j < 9; j++) res.Name[j] = '\0';

		fread(res.Name, sizeof(char), 8, fIn);
		fread(&res.Type, sizeof(uint16), 1, fIn);
		fread(&res.LocationFlags, sizeof(uint32), 1, fIn);
		Arhives[res.GetSourceIndex()].Resources.push_back(res);
	}

	fclose(fIn);
	return true;
}

bool KeyFile::SaveFile(LPCSTR path)
{
	FILE *fOut = fopen(path, "wb");
	if (!fOut) return false;
	fwrite(szKEYDefaultSign, sizeof(char), 8 , fOut);

	uint32 bufData = Arhives.size();
	fwrite(&bufData, sizeof(uint32), 1 , fOut);

	uint32 ResourcesCount = 0;
	for (uint32 i = 0; i < Arhives.size(); i++)
		ResourcesCount += Arhives[i].Resources.size();

	fwrite(&ResourcesCount, sizeof(uint32), 1 , fOut);
	bufData = 24; fwrite(&bufData, sizeof(uint32), 1, fOut);

	uint32 uBifNamesLen = 0;
	for (uint32 i = 0; i < Arhives.size(); i++)
		uBifNamesLen += strlen(Arhives[i].Name) + 1;

	bufData += Arhives.size() * 12 + uBifNamesLen;
	fwrite(&bufData, sizeof(uint32), 1, fOut);

	uBifNamesLen = 24 + Arhives.size() * 12;
	for (uint32 i = 0; i < Arhives.size(); i++)
	{
		fwrite(&Arhives[i].Size, sizeof(uint32), 1, fOut);
		fwrite(&uBifNamesLen, sizeof(uint32), 1, fOut);

		uint16 uLen = strlen(Arhives[i].Name) + 1;
		uBifNamesLen += uLen;
		fwrite(&uLen, sizeof(uint16), 1, fOut);
		fwrite(&Arhives[i].LocationFlags, sizeof(uint16), 1, fOut);
	}

	for (uint32 i = 0; i < Arhives.size(); i++)
		fwrite(Arhives[i].Name, sizeof(char), strlen(Arhives[i].Name) + 1, fOut);

	for (uint32 i = 0; i < Arhives.size(); i++)
	{
		for (uint32 j = 0; j < Arhives[i].Resources.size(); j++)
		{
			fwrite(Arhives[i].Resources[j].Name, sizeof(char), 8, fOut);
			fwrite(&Arhives[i].Resources[j].Type, sizeof(uint16), 1, fOut);
			fwrite(&Arhives[i].Resources[j].LocationFlags, sizeof(uint32), 1, fOut);
        }
	}
	fclose(fOut);
	return true;
}