#include "tra_lib.h"
#include "ECL/strfuncs.h"
#include <stdio.h>

const uint32 MAX_ENTRY_SIZE = 33554432;

CTraList::CTraList() : CDynList()
{
	//
}

CTraList::~CTraList()
{
	Clear();
}

uint32 CTraList::GetNextID()
{
	uint32 uNextID = 0;
	for (uint32 i = 0; i < uItemsCount; i++)
	{
		GotoItm(i);
		if (((CTraItem *)this->Items->Content)->ID > uNextID)
			uNextID = ((CTraItem *)this->Items->Content)->ID;
	}
	return (uNextID + 1);
}

uint32 CTraList::Append(const CTraItem &Item)
{
	uint32 iResult = AppendItm();
	this->Items->Content = new CTraItem(Item);
	return iResult;
}

bool CTraList::Insert(const uint32 &Index, const CTraItem &Item)
{
	if (!InsertItm(Index)) return false;
	this->Items->Content = new CTraItem(Item);
	return true;
}

CTraItem * CTraList::GetEntry(const uint32 &Index)
{
	if (!GotoItm(Index)) return NULL;
	CTraItem *Item = new CTraItem(*((CTraItem *)this->Items->Content));
	return Item;
}

bool CTraList::GetIndexFromID(const uint32 &ID, uint32 &Index)
{
	for (uint32 i = 0; i < this->uItemsCount; i++)
	{
		bool bRes = GotoItm(i);
		if (((CTraItem *)this->Items->Content)->ID == ID)
		{
			Index = i;
			return true;
		}
	}
	return false;
}

bool CTraList::SetEntry(const uint32 &Index, const CTraItem &Item)
{
	if (!GotoItm(Index)) return false;

	delete (CTraItem *)this->Items->Content;
	this->Items->Content = new CTraItem(Item);

	return true;
}

uint32 CTraList::GetEntriesCount()
{
	return GetItmsCount();
}

bool CTraList::Remove(const uint32 &Index)
{
	if (!GotoItm(Index)) return false;
	delete (CTraItem *)this->Items->Content;
	return RemoveItm(Index);
}

void CTraList::Clear()
{
	for (uint32 i = 0; i < this->uItemsCount; i++)
	{
		GotoItm(i);
		delete (CTraItem *)this->Items->Content;
	}

	RemoveItms();
}


bool CTraList::LoadFile(const char *Filename)
{
	if (!Filename) return false;
	FILE *fIn = fopen(Filename, "rt");

	if (!fIn) return false;
	Clear();

	enum CurrentMode {CM_READ_CHUNK, CM_READ_STRING, CM_READ_SOUND, CM_READ_ID, CM_COMMENT, CM_LONG_COMMENT};
	enum CurrentPart {CM_READ_PARAM, CM_READ_VALUE};

	CurrentMode cmRead = CM_READ_CHUNK;
	CurrentPart cpRead = CM_READ_VALUE;

	char cBuf;

	char *sBuf = new char[MAX_ENTRY_SIZE];
	uint32 uCurCharIndex = 0;
	uint32 uCurIndex = 0, uStrIndex = 0, uSndIndex = 0;

	CTraItem *itm = new CTraItem();
	while (!feof(fIn))
	{
		fread(&cBuf, sizeof(char), 1, fIn);
		switch (cBuf)
		{
		case '\n':
			if (cmRead == CM_COMMENT) cmRead = CM_READ_CHUNK;
			else
			if ((cpRead == CM_READ_VALUE)&&((cmRead == CM_READ_STRING)||(cmRead == CM_READ_SOUND)))
			{
				sBuf[uCurCharIndex] = cBuf;
				sBuf[uCurCharIndex + 1] = '\0';
				uCurCharIndex++;
			}
			break;
		case '*':
			if (cmRead == CM_LONG_COMMENT)
			{
				fread(&cBuf, sizeof(char), 1, fIn);
				if (cBuf == '/') cmRead = CM_READ_CHUNK;
			}
			else
			if ((cpRead == CM_READ_VALUE)&&((cmRead == CM_READ_STRING)||(cmRead == CM_READ_SOUND)))
			{
				sBuf[uCurCharIndex] = cBuf;
				sBuf[uCurCharIndex + 1] = '\0';
				uCurCharIndex++;
			}
			break;
		case '/':
			if (cmRead == CM_READ_CHUNK)
			{
				fread(&cBuf, sizeof(char), 1, fIn);
				if (cBuf == '/') cmRead = CM_COMMENT;
				else if (cBuf == '*') cmRead = CM_LONG_COMMENT;
			}
			else
			if ((cpRead == CM_READ_VALUE)&&((cmRead == CM_READ_STRING)||(cmRead == CM_READ_SOUND)))
			{
				sBuf[uCurCharIndex] = cBuf;
				sBuf[uCurCharIndex + 1] = '\0';
				uCurCharIndex++;
			}
			break;
		case '@':
			if ((cmRead == CM_COMMENT)||(cmRead == CM_LONG_COMMENT)) break;
			if (uCurIndex == 0)
			{
				delete itm;
				itm = new CTraItem();
				fscanf(fIn, "%i", &itm->ID);

				uCurIndex++;
				uStrIndex = uCurIndex;
				uSndIndex = uCurIndex;
			}
			else
			if ((itm->TextMale == NULL)&&
				(itm->TextFemale == NULL)&&
				(itm->ItemLink == -1)&&
				(itm->FemaleSoundRes[0] == '\0')&&
				(itm->MaleSoundRes[0] == '\0'))
			{
				uint32 uBuf;
				fscanf(fIn, "%i", &uBuf);
				itm->ItemLink = uBuf;
			}
			else
			{
				Append(*itm);
				delete itm;

				itm = new CTraItem();
				fscanf(fIn, "%i", &itm->ID);

				uCurIndex++;
				uStrIndex = uCurIndex;
				uSndIndex = uCurIndex;
			}

			cpRead = CM_READ_PARAM;
			cmRead = CM_READ_CHUNK;
			break;
		case '~':
			if ((cmRead == CM_COMMENT)||(cmRead == CM_LONG_COMMENT)) break;
			if (cpRead == CM_READ_PARAM) 
			{
				fclose(fIn);
				sBuf[uCurCharIndex] = '\0';
				delete []sBuf;
				return false;
			}
			if (cmRead != CM_READ_STRING) cmRead = CM_READ_STRING;
			else
			{
				if (uStrIndex == uCurIndex)
				{
					sBuf[uCurCharIndex] = '\0';

					itm->TextMale = new char[uCurCharIndex + 1];
					itm->TextMale = strcpy(itm->TextMale, sBuf);
					itm->TextMale[uCurCharIndex] = '\0';

					delete []sBuf;
					sBuf = new char[MAX_ENTRY_SIZE];

					uCurCharIndex = 0;
					cmRead = CM_READ_CHUNK;

					uStrIndex++;
				}
				else
				{
					sBuf[uCurCharIndex] = '\0';

					itm->TextFemale = new char[uCurCharIndex + 1];
					itm->TextFemale = strcpy(itm->TextFemale, sBuf);
					itm->TextFemale[uCurCharIndex] = '\0';

					delete []sBuf;
					sBuf = new char[MAX_ENTRY_SIZE];

					uCurCharIndex = 0;
					cmRead = CM_READ_CHUNK;

					uStrIndex++;
				}
			}
			break;
		case '[':
			if ((cmRead == CM_COMMENT)||(cmRead == CM_LONG_COMMENT)) break;
			if (cpRead == CM_READ_PARAM)
			{
				fclose(fIn);
				sBuf[uCurCharIndex] = '\0';
				delete []sBuf;
				return false;
			}
			if (cmRead != CM_READ_STRING) cmRead = CM_READ_SOUND;
			else
			{
				sBuf[uCurCharIndex] = cBuf;
				sBuf[uCurCharIndex + 1] = '\0';
				uCurCharIndex++;
			}
			break;
		case ']':
			if ((cmRead == CM_COMMENT)||(cmRead == CM_LONG_COMMENT)) break;
			if (cpRead == CM_READ_PARAM)
			{
				fclose(fIn);
				sBuf[uCurCharIndex] = '\0';
				delete []sBuf;
				return false;
			}
			if (cmRead == CM_READ_SOUND)
			{
				if (uSndIndex == uCurIndex)
				{
					sBuf[uCurCharIndex] = '\0';
					strcpy(itm->MaleSoundRes, sBuf);
					delete []sBuf;

					sBuf = new char[MAX_ENTRY_SIZE];
					uCurCharIndex = 0;

					cmRead = CM_READ_CHUNK;
					uSndIndex++;
				}
				else
				{
					sBuf[uCurCharIndex] = '\0';
					strcpy(itm->FemaleSoundRes, sBuf);
					delete []sBuf;

					sBuf = new char[MAX_ENTRY_SIZE];
					uCurCharIndex = 0;

					cmRead = CM_READ_CHUNK;
					uSndIndex++;
				}
			}
			else
			{
				sBuf[uCurCharIndex] = cBuf;
				sBuf[uCurCharIndex + 1] = '\0';
				uCurCharIndex++;
			}
			break;
		case '=':
			if ((cmRead == CM_COMMENT)||(cmRead == CM_LONG_COMMENT)) break;
			if (cpRead != CM_READ_VALUE)
				cpRead = CM_READ_VALUE;
			else
			if ((cmRead == CM_READ_STRING)||(cmRead == CM_READ_SOUND))
			{
				sBuf[uCurCharIndex] = cBuf;
				sBuf[uCurCharIndex + 1] = '\0';
				uCurCharIndex++;
			}
			break;
		default:
			if ((cpRead == CM_READ_VALUE)&&((cmRead == CM_READ_STRING)||(cmRead == CM_READ_SOUND)))
			{
				sBuf[uCurCharIndex] = cBuf;
				sBuf[uCurCharIndex + 1] = '\0';
				uCurCharIndex++;
			}
			break;
		}
	}
	if (uCurIndex > 0) Append(*itm);
	delete itm;
	delete []sBuf;

	fclose(fIn);
	return true;
}

bool CTraList::SaveFile(const char *Filename)
{
	FILE *fOut = fopen(Filename, "wt");
	if (!fOut) return false;

	uint32 uCurEntry = this->uItemsIndex;
	for (uint32 i = 0; i < this->uItemsCount; i++)
	{
		GotoItm(i);
		fprintf(fOut, "@%i = ", ((CTraItem *)this->Items->Content)->ID);

		WriteStr2File(fOut, (CTraItem *)this->Items->Content);

		fprintf(fOut, "\n");
	}

	fclose(fOut);
	GotoItm(uCurEntry);
	return true;
}

void CTraList::SortEntries()
{
	if (this->uItemsCount < 2) return;

	bool bNoChanges = false;
	uint32 j = 0;
	while (!bNoChanges)
	{
		bNoChanges = true;
		for (uint32 i = j; i < this->uItemsCount; i++)
		{
			if ((this->Items->GetNext() != NULL)&&
				(((CTraItem *)this->Items->Content)->ID >
					((CTraItem *)this->Items->GetNext()->Content)->ID))
			{
				bNoChanges = false;
				this->Items->SwapWithNext();
			}
			else
			if (this->Items->GetNext())
				this->Items = this->Items->GetNext();
		}
		if (bNoChanges) break;
		for (uint32 i = this->uItemsCount; i > j; i--)
		{
			if ((this->Items->GetPrev() != NULL)&&
				(((CTraItem *)this->Items->Content)->ID <
					((CTraItem *)this->Items->GetPrev()->Content)->ID))
			{
				this->Items->SwapWithPrev();
			}
			else
			if (this->Items->GetPrev())
				this->Items = this->Items->GetPrev();
		}
		j++;
	}
	this->uItemsIndex = this->uItemsCount - 1;
}

bool CTraList::IsEntryExists(const uint32 &ID)
{
	for (uint32 i = 0; i < uItemsCount; i++)
	{
		GotoItm(i);
		if (((CTraItem *)this->Items->Content)->ID == ID)
			return true;
	}
	return false;
}

bool CTraList::LoadFileWithoutIDs(const char *Filename)
{
	if (!Filename) return false;
	FILE *fIn = fopen(Filename, "rt");

	if (!fIn) return false;
	Clear();

	enum CurrentMode {CM_READ_CHUNK, CM_READ_STRING};
	CurrentMode cmRead = CM_READ_CHUNK;

	char cBuf;

	char *sBuf = new char[MAX_ENTRY_SIZE];
	uint32 uCurCharIndex = 0;
	uint32 uCurIndex = 0;

	CTraItem *itm = new CTraItem();
	while (true)
	{
		fread(&cBuf, sizeof(char), 1, fIn);
		if (feof(fIn)) break;
		switch (cBuf)
		{		
		case '~':
			if (cmRead != CM_READ_STRING) cmRead = CM_READ_STRING;
			else
			{
				sBuf[uCurCharIndex] = '\0';

				itm->TextMale = new char[uCurCharIndex + 1];
				itm->TextMale = strcpy(itm->TextMale, sBuf);
				itm->TextMale[uCurCharIndex] = '\0';

				delete []sBuf;
				sBuf = new char[MAX_ENTRY_SIZE];
				cmRead = CM_READ_CHUNK;

				bool bFound = false;
				uCurCharIndex = 0;
				do
				{
					fread(&cBuf, sizeof(char), 1, fIn);
					if (feof(fIn)) break;
					if (cBuf == '[')
					{
						fread(&cBuf, sizeof(char), 1, fIn);
						while ((cBuf != ']')&&(uCurCharIndex < 8))
						{
							if (feof(fIn)) break;
							itm->MaleSoundRes[uCurCharIndex] = cBuf;
							uCurCharIndex++;
							fread(&cBuf, sizeof(char), 1, fIn);
						}
						itm->MaleSoundRes[uCurCharIndex] = '\0';
						bFound = true;
					}
					else
					if (cBuf == '~')
					{
						cmRead = CM_READ_STRING;
						bFound = true;
					}

				} while ((!bFound)&&((cBuf == ' ')||
						(cBuf == '\t')||
						(cBuf == '\n')));

				if (itm->IsEntryEmpty())
				{
					delete itm;
					itm = new CTraItem();
					itm->ID = uCurIndex;
					break;
				}

				bFound = false;
				uCurCharIndex = 0;
				for (uint32 j = 0; j < uItemsCount; j++)
				{
					if (!GotoItm(j)) break;
					if (itm->IsEntryEquale(*((CTraItem *)this->Items->Content)))
					{
						delete itm;
						itm = new CTraItem();
						itm->ID = uCurIndex;
						bFound = true;
						break;
					}
				}

				if (!bFound)
				{
					Append(*itm);
					delete itm;

					uCurIndex++;
					itm = new CTraItem();
					itm->ID = uCurIndex;
				}
			}
			break;
		default:
			if (cmRead == CM_READ_STRING)
			{
				sBuf[uCurCharIndex] = cBuf;
				sBuf[uCurCharIndex + 1] = '\0';
				uCurCharIndex++;
			}
			break;
		}
	}

	if (!itm->IsEntryEmpty())
	{
		bool bFound = false;
		for (uint32 j = 0; j < uItemsCount; j++)
		{
			if (!GotoItm(j)) break;
			if (itm->IsEntryEquale(*((CTraItem *)this->Items->Content)))
			{
				bFound = true;
				break;
			}
		}

		if (!bFound) Append(*itm);
		delete itm;
	}
	else delete itm;

	delete []sBuf;

	fclose(fIn);
	return true;
}

bool CTraList::SaveFileWithoutIDs(const char *Savename, const char *Filename)
{
	if (!Filename) return false;
	if (!Savename) return false;

	CTraList *lstOrig = new CTraList();
	if (!lstOrig->LoadFileWithoutIDs(Filename)) return false;
	if (lstOrig->GetEntriesCount() != this->GetEntriesCount())
	{
		delete lstOrig;
		return false;
	}

	FILE *fIn = fopen(Filename, "rt");
	if (!fIn)
	{
		delete lstOrig;
		return false;
	}

	FILE *fOut = fopen(Savename, "wt");
	if (!fOut)
	{
		fclose(fIn);
		delete lstOrig;
		return false;
	}

	enum CurrentMode {CM_READ_CHUNK, CM_READ_STRING};
	CurrentMode cmRead = CM_READ_CHUNK;

	char cBuf;

	char *sBuf = new char[MAX_ENTRY_SIZE];
	uint32 uCurCharIndex = 0;
	uint32 uCurIndex = 0;

	CTraItem *itm = new CTraItem();
	while (true)
	{
		fread(&cBuf, sizeof(char), 1, fIn);
		if (feof(fIn)) break;
		switch (cBuf)
		{		
		case '~':
			if (cmRead != CM_READ_STRING) cmRead = CM_READ_STRING;
			else
			{
				sBuf[uCurCharIndex] = '\0';

				itm->TextMale = new char[uCurCharIndex + 1];
				itm->TextMale = strcpy(itm->TextMale, sBuf);
				itm->TextMale[uCurCharIndex] = '\0';

				delete []sBuf;
				sBuf = new char[MAX_ENTRY_SIZE];
				cmRead = CM_READ_CHUNK;

				bool bFound = false;
				uCurCharIndex = 0;
				do
				{
					fread(&cBuf, sizeof(char), 1, fIn);
					if (feof(fIn)) bFound = true;
					else
					if (cBuf == '[')
					{
						fread(&cBuf, sizeof(char), 1, fIn);
						uint32 uIndexC = 0;
						while ((cBuf != ']')&&(uIndexC < 8))
						{
							if (feof(fIn)) break;
							itm->MaleSoundRes[uIndexC] = cBuf;
							uIndexC++;
							fread(&cBuf, sizeof(char), 1, fIn);
						}
						itm->MaleSoundRes[uIndexC] = '\0';
						bFound = true;
					}
					else
					if (cBuf == '~')
					{
						cmRead = CM_READ_STRING;
						bFound = true;
					}

					sBuf[uCurCharIndex] = cBuf;
					uCurCharIndex++;
				} while ((!bFound)&&((cBuf == ' ')||
						(cBuf == '\t')||
						(cBuf == '\n')));

				uCurCharIndex--;
				sBuf[uCurCharIndex] = '\0';
				if (itm->IsEntryEmpty())
				{
					WriteStr2File(fOut, itm);
					fwrite(sBuf, sizeof(char), uCurCharIndex, fOut);
					fwrite(&cBuf, sizeof(char), 1, fOut);
					delete itm;
					itm = new CTraItem();
					itm->ID = uCurIndex;

					delete []sBuf;
					sBuf = new char[MAX_ENTRY_SIZE];
					uCurCharIndex = 0;
					break;
				}

				bFound = false;
				for (uint32 j = 0; j < uItemsCount; j++)
				{
					CTraItem *itmBuf = lstOrig->GetEntry(j);

					if (itm->IsEntryEquale(*itmBuf))
					{
						delete itm;
						itm = new CTraItem();
						itm->ID = uCurIndex;

						cmRead = CM_READ_CHUNK;

						delete itmBuf;
						itmBuf = GetEntry(j);

						WriteStr2File(fOut, itmBuf);
						fwrite(sBuf, sizeof(char), uCurCharIndex, fOut);
						if ((cBuf != '~')&&
							(cBuf != ']')) fwrite(&cBuf, sizeof(char), 1, fOut);

						delete itmBuf;
						bFound = true;
						break;
					}

					delete itmBuf;
				}

				if (!bFound)
				{
					WriteStr2File(fOut, itm);
					fwrite(sBuf, sizeof(char), uCurCharIndex, fOut);
					if ((cBuf != '~')&&
						(cBuf != ']')) fwrite(&cBuf, sizeof(char), 1, fOut);
					delete itm;

					uCurIndex++;
					itm = new CTraItem();
					itm->ID = uCurIndex;
				}

				delete []sBuf;
				sBuf = new char[MAX_ENTRY_SIZE];
				uCurCharIndex = 0;
			}
			break;
		default:
			if (cmRead == CM_READ_STRING)
			{
				sBuf[uCurCharIndex] = cBuf;
				sBuf[uCurCharIndex + 1] = '\0';
				uCurCharIndex++;
			}
			else fwrite(&cBuf, sizeof(char), 1, fOut);
			break;
		}
	}
	
	delete itm;
	delete []sBuf;

	delete lstOrig;
	fclose(fOut);
	fclose(fIn);
	return true;
}

void CTraList::WriteStr2File(FILE *fOut, CTraItem *itm)
{
	bool bMaleString = false, bMaleSound = false, bFemaleString = false;

	if (itm->ItemLink == -1)
	{
		if (itm->TextMale)
		{
			fprintf(fOut, "~%s~", itm->TextMale);
			bMaleString = true;
		}
		else fprintf(fOut, "~~");

		if (itm->MaleSoundRes[0] != '\0')
		{
			fprintf(fOut, " [%s]", itm->MaleSoundRes);
			bMaleSound = true;
		}

		if (itm->TextFemale)
		{
			if ((itm->FemaleSoundRes[0] != '\0')&&(!bMaleSound))
				fprintf(fOut, " []");
			fprintf(fOut, " ~%s~", itm->TextFemale);
			bFemaleString = true;
		}

		if (itm->FemaleSoundRes[0] != '\0')
		{
			if ((!bMaleString)&&(!bMaleSound)&&(!bFemaleString))
				fprintf(fOut, " [] ~~");
			fprintf(fOut, " [%s]", itm->FemaleSoundRes);
		}
		else
		if ((itm->TextFemale)&&(bMaleSound))
			fprintf(fOut, " []");
	}
	else fprintf(fOut, "@%i", itm->ItemLink);
}