#include "tlk_lib_v3.h"

#include "ECL/bits_funcs.h"
#include <stdio.h>

CTLKFileV3::CTLKFileV3() : CDynList()
{
	this->LanguageID = 0;
}

CTLKFileV3::~CTLKFileV3()
{
	Clear();
}

uint32 CTLKFileV3::Append(const CTLKItemV3 &Item)
{
	uint32 iResult = AppendItm();
	this->Items->Content = new CTLKItemV3(Item);
	return iResult;
}

bool CTLKFileV3::Insert(const uint32 &Index, const CTLKItemV3 &Item)
{
	if (!InsertItm(Index)) return false;
	this->Items->Content = new CTLKItemV3(Item);
	return true;
}

CTLKItemV3 * CTLKFileV3::GetEntry(const uint32 &Index)
{
	if (!GotoItm(Index)) return NULL;
	CTLKItemV3 *Item = new CTLKItemV3(*((CTLKItemV3 *)this->Items->Content));
	return Item;
}

bool CTLKFileV3::SetEntry(const uint32 &Index, const CTLKItemV3 &Item)
{
	if (!GotoItm(Index)) return false;

	delete (CTLKItemV3 *)this->Items->Content;
	this->Items->Content = new CTLKItemV3(Item);

	return true;
}

uint32 CTLKFileV3::GetEntriesCount()
{
	return GetItmsCount();
}

bool CTLKFileV3::Remove(const uint32 &Index)
{
	if (!GotoItm(Index)) return false;
	delete (CTLKItemV3 *)this->Items->Content;
	return RemoveItm(Index);
}

void CTLKFileV3::Clear()
{
	for (uint32 i = 0; i < this->uItemsCount; i++)
	{
		GotoItm(i);
		delete (CTLKItemV3 *)this->Items->Content;
	}
	RemoveItms();
}

uint32 CTLKFileV3::GetContentOffset()
{
	return TLKEntryV3_Size * this->uItemsCount + TLKHeaderV3_Size;
}

bool CTLKFileV3::LoadFile(const char * Filename)
{
	FILE *fIn = fopen(Filename, "rb");
	if (!fIn) return false;
	char sSign[5]; sSign[4] = '\0';
	fread(sSign, sizeof(char), 4, fIn);
	if (!strEquale(sSign, "TLK "))
	{
		fclose(fIn);
		return false;
	}
	fread(sSign, sizeof(char), 4, fIn);
	
	if (strEquale(sSign, "V3.0"))
	{
		Clear();

		uint32 uStringsCount = 0;
		uint32 uTextOffset = 0;
		uint32 uLanguageID = 0;

		fread(&uLanguageID, sizeof(uint32), 1, fIn);
		fread(&uStringsCount, sizeof(uint32), 1, fIn);
		fread(&uTextOffset, sizeof(uint32), 1, fIn);

		CTLKItemV3 *tlkItems = new CTLKItemV3[uStringsCount];
		for (uint32 i = 0; i < uStringsCount; i++)
		{
			fread(&tlkItems[i].Flags, sizeof(uint32), 1, fIn);
			fread(&tlkItems[i].SoundResRef, sizeof(char), 16, fIn);
			fread(&tlkItems[i].VolumeVariance, sizeof(uint32), 1, fIn);
			fread(&tlkItems[i].PitchVariance, sizeof(uint32), 1, fIn);
			fread(&tlkItems[i].OffsetToString, sizeof(uint32), 1, fIn);
			fread(&tlkItems[i].StringSize, sizeof(uint32), 1, fIn);
			fread(&tlkItems[i].SoundLength, sizeof(float), 1, fIn);
		}

		for (uint32 i = 0; i < uStringsCount; i++)
		{
			if (tlkItems[i].StringSize > 0)
			{
				tlkItems[i].Text = new char[tlkItems[i].StringSize + 1];
				fseek(fIn, uTextOffset + tlkItems[i].OffsetToString, 0);
				fread(tlkItems[i].Text, sizeof(char), tlkItems[i].StringSize, fIn);
				tlkItems[i].Text[tlkItems[i].StringSize] = '\0';
			}
			else tlkItems[i].Text = NULL;

			Append(tlkItems[i]);
		}

		delete []tlkItems;
		fclose(fIn);

		return true;
	}
	else fclose(fIn);

	return false;
}

bool CTLKFileV3::SaveFile(const char * Filename)
{
	FILE * fOut = fopen(Filename, "wb");
	if (!fOut) return false;

	char Sign[] = "TLK V3.0";
	fwrite(Sign, sizeof(char), 8, fOut);

	fwrite(&this->LanguageID, sizeof(uint32), 1, fOut);
	fwrite(&this->uItemsCount, sizeof(uint32), 1, fOut);

	uint32 uContentOffset = GetContentOffset();
	fwrite(&uContentOffset, sizeof(uint32), 1, fOut);

	const uint32 uNullValue = 0;
	uint32 bufValue = 0;
	for (uint32 i = 0; i < this->uItemsCount; i++)
	{
		GotoItm(i);

		uint32 uFlags = ((CTLKItemV3 *)this->Items->Content)->Flags;
		if (uFlags == 0)
		{
			if (((CTLKItemV3 *)this->Items->Content)->Text)
				uFlags = SetBit(uFlags, 0, true);
			if (((CTLKItemV3 *)this->Items->Content)->SoundResRef[0] != '\0')
				uFlags = SetBit(uFlags, 1, true);
			if (((CTLKItemV3 *)this->Items->Content)->SoundLength != 0.0)
				uFlags = SetBit(uFlags, 2, true);
		}
			
		fwrite(&uFlags, sizeof(uint32), 1, fOut);
		fwrite(&((CTLKItemV3 *)this->Items->Content)->SoundResRef, sizeof(char), 16, fOut);
		fwrite(&((CTLKItemV3 *)this->Items->Content)->VolumeVariance, sizeof(uint32), 1, fOut);
		fwrite(&((CTLKItemV3 *)this->Items->Content)->PitchVariance, sizeof(uint32), 1, fOut);

		if (((CTLKItemV3 *)this->Items->Content)->Text)
		{
			uint32 uLen = strlen(((CTLKItemV3 *)this->Items->Content)->Text);
			fwrite(&bufValue, sizeof(uint32), 1, fOut);
			fwrite(&uLen, sizeof(uint32), 1, fOut);
			bufValue += uLen;
		}
		else
		{
			fwrite(&uNullValue, sizeof(uint32), 1, fOut);
			fwrite(&uNullValue, sizeof(uint32), 1, fOut);
		}

		fwrite(&((CTLKItemV3 *)this->Items->Content)->SoundLength, sizeof(float), 1, fOut);
	}

	for (uint32 i = 0; i < this->uItemsCount; i++)
	{
		GotoItm(i);

		if (((CTLKItemV3 *)this->Items->Content)->Text)
			fwrite(((CTLKItemV3 *)this->Items->Content)->Text, sizeof(char),
				strlen(((CTLKItemV3 *)this->Items->Content)->Text), fOut);
	}

	fclose(fOut);
	return true;
}