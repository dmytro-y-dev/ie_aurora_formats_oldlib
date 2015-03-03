#include "tlk_lib_v1.h"

#include "ECL/bits_funcs.h"
#include <stdio.h>

CTLKFileV1::CTLKFileV1() : CDynList()
{
	this->LanguageID = 0;
}

CTLKFileV1::~CTLKFileV1()
{
	Clear();
}

uint32 CTLKFileV1::Append(const CTLKItemV1 &Item)
{
	uint32 iResult = AppendItm();
	this->Items->Content = new CTLKItemV1(Item);
	return iResult;
}

bool CTLKFileV1::Insert(const uint32 &Index, const CTLKItemV1 &Item)
{
	if (!InsertItm(Index)) return false;
	this->Items->Content = new CTLKItemV1(Item);
	return true;
}

CTLKItemV1 * CTLKFileV1::GetEntry(const uint32 &Index)
{
	if (!GotoItm(Index)) return false;
	CTLKItemV1 *Item = new CTLKItemV1(*((CTLKItemV1 *)this->Items->Content));
	return Item;
}

bool CTLKFileV1::SetEntry(const uint32 &Index, const CTLKItemV1 &Item)
{
	if (!GotoItm(Index)) return false;

	delete (CTLKItemV1 *)this->Items->Content;
	this->Items->Content = new CTLKItemV1(Item);

	return true;
}

uint32 CTLKFileV1::GetEntriesCount()
{
	return GetItmsCount();
}

bool CTLKFileV1::Remove(const uint32 &Index)
{
	if (!GotoItm(Index)) return false;
	delete (CTLKItemV1 *)this->Items->Content;
	return RemoveItm(Index);
}

void CTLKFileV1::Clear()
{
	for (uint32 i = 0; i < this->uItemsCount; i++)
	{
		GotoItm(i);
		delete (CTLKItemV1 *)this->Items->Content;
	}
	RemoveItms();
}

uint32 CTLKFileV1::GetContentOffset()
{
	return TLKEntryV1_Size * this->uItemsCount + TLKHeaderV1_Size;
}

bool CTLKFileV1::LoadFile(const char * Filename)
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
	
	if (strEquale(sSign, "V1  "))
	{
		Clear();

		uint32 uStringsCount = 0;
		uint32 uTextOffset = 0;
		uint32 uLanguageID = 0;

		fread(&uLanguageID, sizeof(uint16), 1, fIn);
		fread(&uStringsCount, sizeof(uint32), 1, fIn);
		fread(&uTextOffset, sizeof(uint32), 1, fIn);

		CTLKItemV1 *tlkItems = new CTLKItemV1[uStringsCount];
		for (uint32 i = 0; i < uStringsCount; i++)
		{
			fread(&tlkItems[i].Flags, sizeof(uint16), 1, fIn);
			fread(tlkItems[i].SoundResRef, sizeof(char), 8, fIn);
			fread(&tlkItems[i].VolumeVariance, sizeof(uint32), 1, fIn);
			fread(&tlkItems[i].PitchVariance, sizeof(uint32), 1, fIn);
			fread(&tlkItems[i].OffsetToString, sizeof(uint32), 1, fIn);
			fread(&tlkItems[i].StringSize, sizeof(uint32), 1, fIn);
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

bool CTLKFileV1::SaveFile(const char * Filename)
{
	FILE * fOut = fopen(Filename, "wb");
	if (!fOut) return false;

	char Sign[] = "TLK V1  ";
	fwrite(Sign, sizeof(char), 8, fOut);

	fwrite(&this->LanguageID, sizeof(uint16), 1, fOut);
	fwrite(&this->uItemsCount, sizeof(uint32), 1, fOut);

	uint32 uContentOffset = GetContentOffset();
	fwrite(&uContentOffset, sizeof(uint32), 1, fOut);

	const uint32 uNullValue = 0;
	uint32 bufValue = 0;
	for (uint32 i = 0; i < this->uItemsCount; i++)
	{
		GotoItm(i);

		uint16 uFlags = ((CTLKItemV1 *)this->Items->Content)->Flags;
		if (uFlags == 0)
		{
			if (((CTLKItemV1 *)this->Items->Content)->Text)
				uFlags = SetBit(uFlags, 0, true);
			if (((CTLKItemV1 *)this->Items->Content)->SoundResRef[0] != '\0')
				uFlags = SetBit(uFlags, 1, true);
		}
			
		fwrite(&uFlags, sizeof(uint16), 1, fOut);
		fwrite(&((CTLKItemV1 *)this->Items->Content)->SoundResRef, sizeof(char), 8, fOut);
		fwrite(&((CTLKItemV1 *)this->Items->Content)->VolumeVariance, sizeof(uint32), 1, fOut);
		fwrite(&((CTLKItemV1 *)this->Items->Content)->PitchVariance, sizeof(uint32), 1, fOut);

		if (((CTLKItemV1 *)this->Items->Content)->Text)
		{
			uint32 uLen = strlen(((CTLKItemV1 *)this->Items->Content)->Text);
			fwrite(&bufValue, sizeof(uint32), 1, fOut);
			fwrite(&uLen, sizeof(uint32), 1, fOut);
			bufValue += uLen;
		}
		else
		{
			fwrite(&uNullValue, sizeof(uint32), 1, fOut);
			fwrite(&uNullValue, sizeof(uint32), 1, fOut);
		}
	}

	for (uint32 i = 0; i < this->uItemsCount; i++)
	{
		GotoItm(i);

		if (((CTLKItemV1 *)this->Items->Content)->Text)
			fwrite(((CTLKItemV1 *)this->Items->Content)->Text, sizeof(char),
				strlen(((CTLKItemV1 *)this->Items->Content)->Text), fOut);
	}

	fclose(fOut);
	return true;
}