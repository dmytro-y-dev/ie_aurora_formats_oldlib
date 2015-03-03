#include "2da_lib.h"
#include <string.h>

C2DAFile::C2DAFile() : CDynList()
{
	Header = new _2DAEntry;
}

C2DAFile::~C2DAFile()
{
	Clear();
	if (Header)
	{
		for (uint32 i = 0; i < Header->uValuesCount; i++)
			if (Header->Values[i]) delete []Header->Values[i];
		if (Header->Values) delete []Header->Values;
		delete Header;
	}
}

bool C2DAFile::Load2DA(LPCSTR Filename)
{
	FILE *fIn = fopen(Filename, "rt");
	if (!fIn) return false;
	char sSign[9]; sSign[8] = '\0';
	fread(sSign, sizeof(char), 8, fIn);
	if (strcmp(sSign, "2DA V2.0")) return false;
	Clear();
	//Чтение заголовка
	{
		char strHeader[1024];
		bool bHeaderBegan = false, bNextWord = false;
		Header->uValuesCount = 0;
		for (uint16 i = 0; ;)
		{
			fread(&strHeader[i], sizeof(char), 1, fIn);
			if ((strHeader[i] != ' ')&&
				(strHeader[i] != '\t')&&
				(strHeader[i] != '\n'))
			{
				bHeaderBegan = true;
				bNextWord = true;
				i++;
			}
			else
			{
				if ((strHeader[i] == '\n')&&(bHeaderBegan))
				{
					if (strHeader[i - 1] != ' ') Header->uValuesCount++;
					strHeader[i] = '\0';
					break;
				}
				else
				if (bNextWord)
				{
					strHeader[i] = ' ';
					Header->uValuesCount++;
					bNextWord = false;
					i++;
				}
			}
		}
		Header->Values = new LPSTR[Header->uValuesCount];

		uint16 uOffset = 0;
		for (uint16 i = 0; i < Header->uValuesCount; i++)
		{
			uint8 uStrCount = 0;
			for (;;uStrCount++)
			{
				if ((strHeader[uStrCount + uOffset] == ' ')
					||(strHeader[uStrCount + uOffset] == '\0')) break;
			}
			Header->Values[i] = new CHAR[uStrCount + 1];
			for (uint8 j = 0; j < uStrCount; j++)
				Header->Values[i][j] = strHeader[j + uOffset];
			Header->Values[i][uStrCount] = '\0';
			uOffset += uStrCount + 1;
		}
	}
	//Чтение строк
	{
		uint i = 0;
		while (!feof(fIn))
		{
			_2DAEntry *entry = ReadEntry(fIn);
			if (entry)
			{
				AddEntry(*entry);
				delete entry;
			}
			else break;
			i++;
		}
	}
	fclose(fIn);
	return true;
}

_2DAEntry * C2DAFile::ReadEntry(FILE *fIn)
{
	_2DAEntry * entry = new _2DAEntry;
	fscanf(fIn, "%u", &entry->uIndex);
	entry->uValuesCount = Header->uValuesCount;
	entry->Values = new LPSTR[Header->uValuesCount];
	for (uint32 i = 0; i < Header->uValuesCount; i++)
	{
		if ((entry->Values[i] = ReadValue(fIn)) == NULL)
		{
			for (uint32 j = 1; j < i; j++)
				delete []entry->Values[j - 1];
			delete entry;
			return NULL;
		}
	}
	return entry;
}

LPSTR C2DAFile::ReadValue(FILE *fIn)
{
	CHAR strLBuf[1024];
	for (uint16 i = 0;;)
	{
		fread(&strLBuf[i], sizeof(char), 1, fIn);
		if (feof(fIn)) return NULL;
		if ((strLBuf[i] != ' ')&&
			(strLBuf[i] != '\t')&&
			(strLBuf[i] != '\n'))
		{
			i++;
		}
		else
		{
			if (i > 0)
			{
				strLBuf[i] = '\0';
				break;
			}
		}
	}
	uint16 uCount = strlen(strLBuf);
	LPSTR strTemp = new CHAR[uCount + 1];
	strcpy(strTemp, strLBuf);
	strTemp[uCount] = '\0';
	return strTemp;
}

uint32 GetNumbLen(uint32 numb)
{
	uint32 uNumbLen = 0;
	while (numb != 0)
	{
		numb /= 10;
		uNumbLen++;
	}

	if (uNumbLen == 0) return 1;
	else return uNumbLen;
}

bool C2DAFile::Save2DA(LPCSTR Filename)
{
	FILE *fOut = fopen(Filename, "wt");
	if (!fOut) return false;

	_2DAEntry *entry;
	uint32 uMaxParamsLen[256], uMaxIndexLen = 0;
	for (uint32 i = 0; i < Header->uValuesCount; i++) uMaxParamsLen[i] = strlen(Header->Values[i]);

	for (uint32 i = 0; i < uItemsCount; i++)
	{
		uint32 uParamLen;

		entry = GetEntry(i);
		for (uint8 j = 0; j < entry->uValuesCount; j++)
		{
			uParamLen = strlen(entry->Values[j]);
			if (uParamLen > uMaxParamsLen[j]) uMaxParamsLen[j] = uParamLen;
		}
		if (entry->uIndex > uMaxIndexLen) uMaxIndexLen = entry->uIndex;
	}

	uMaxIndexLen = GetNumbLen(uMaxIndexLen);

	//Запись заголовка
	char sSign[] = "2DA V2.0\n\n";
	fwrite(sSign, sizeof(char), 10, fOut);
	fprintf(fOut, " ");
	for (uint32 z = 0; z < uMaxIndexLen; z++) fprintf(fOut, " ");

	for (uint8 i = 0; i < Header->uValuesCount; i++)
	{
		fprintf(fOut, "%s", Header->Values[i]);
		if (i != (Header->uValuesCount - 1))
		{
			for (uint32 z = strlen(Header->Values[i]); z < uMaxParamsLen[i]; z++) fprintf(fOut, " ");
			fprintf(fOut, " ");
		}
	}
	fprintf(fOut, "\n\n");

	//Запись элементов
	for (uint32 i = 0; i < uItemsCount; i++)
	{
		entry = GetEntry(i);
		fprintf(fOut, "%i ", entry->uIndex);
		uint32 uSpacesMinus = 0;
		for (uint32 z = GetNumbLen(entry->uIndex), uSpacesMinus = uMaxIndexLen - GetNumbLen(entry->uIndex);
					z < uMaxIndexLen; z++) fprintf(fOut, " ");

		uint32 uParamLen;
		for (uint8 j = 0; j < entry->uValuesCount; j++)
		{
			fprintf(fOut, "%s", entry->Values[j]);
			if (j != (entry->uValuesCount - 1))
			{
				for (uint32 z = strlen(entry->Values[j]) + uSpacesMinus;
					z < uMaxParamsLen[j]; z++) fprintf(fOut, " ");
				fprintf(fOut, " ");
			}
		}
		fprintf(fOut, "\n");
	}
	fclose(fOut);
	return true;
}

uint32 C2DAFile::AddEntry(const _2DAEntry &Item)
{
	uint32 uResult = AppendItm();
	this->Items->Content = new _2DAEntry(Item);
	return uResult;
}

_2DAEntry * C2DAFile::GetEntry(const uint32 &Index)
{
	if (!GotoItm(Index)) return NULL;
	return (_2DAEntry *)this->Items->Content;
}

uint32 C2DAFile::GetEntriesCount()
{
	return GetItmsCount();
}

bool C2DAFile::RemoveEntry(const uint32 &Index)
{
	if (!GotoItm(Index)) return false;
	_2DAEntry *Temp = (_2DAEntry *)this->Items->Content;
	for (uint32 i = 0; i < Temp->uValuesCount; i++)
		if (Temp->Values[i]) delete []Temp->Values[i];
	if (Temp->Values) delete []Temp->Values;
	delete (_2DAEntry *)this->Items->Content;
	return RemoveItm(Index);
}

void C2DAFile::Clear()
{
	while (RemoveEntry(0));
}

void C2DAFile::Sort()
{
	uint32 iCount = this->uItemsCount;
	if (iCount < 2) return;
	bool bNoChanges = false; bool bFirst = true;
	uint32 j = 0; GotoItm(0);
	while (!bNoChanges)
	{
		bNoChanges = true;
		for (uint32 i = j; i < iCount; i++)
		{
			if ((this->Items->GetNext() != NULL)&&
				(((_2DAEntry *)this->Items->Content)->uIndex >
					((_2DAEntry *)this->Items->GetNext()->Content)->uIndex))
			{
				bNoChanges = false;
				this->Items->SwapWithNext();
				uItemsIndex = iCount - 2;
			}
			else
			if (this->Items->GetNext())
				this->Items = this->Items->GetNext();
		}
		if ((bNoChanges)&&(!bFirst)) break;
		for (uint32 i = iCount; i > j; i--)
		{
			if ((this->Items->GetPrev() != NULL)&&
				(((_2DAEntry *)this->Items->Content)->uIndex <
					((_2DAEntry *)this->Items->GetPrev()->Content)->uIndex))
			{
				this->Items->SwapWithPrev();
				bNoChanges = false;
				uItemsIndex = j + 1;
			}
			else
			if (this->Items->GetPrev())
				this->Items = this->Items->GetPrev();
		}
		if (bNoChanges) break;
		j++; bFirst = false;
	}
	GotoItm(0);
	return;
}