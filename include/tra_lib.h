#ifndef _TRA_LIB_H_
#define _TRA_LIB_H_

#include "ECL/types_common.h"
#include "ECL/dyn_list.h"

#include "ECL/strfuncs.h"
#include <stdio.h>

struct CTraItem
{
	uint32 ID;
	char *TextMale;
	char *TextFemale;
	char MaleSoundRes[9];
	char FemaleSoundRes[9];
	int ItemLink;

	CTraItem(const CTraItem &Item) 
	{
		for (uint8 i = 0; i < 9; i++)
		{
			MaleSoundRes[i] = Item.MaleSoundRes[i];
			FemaleSoundRes[i] = Item.FemaleSoundRes[i];
		}

		if (Item.TextMale)
		{
			uint32 uCount = strlen(Item.TextMale);
			TextMale = new char[uCount + 1];
			TextMale = strcpy(TextMale, Item.TextMale);
			TextMale[uCount] = '\0';
		} else TextMale = NULL;

		if (Item.TextFemale)
		{
			uint32 uCount = strlen(Item.TextFemale);
			TextFemale = new char[uCount + 1];
			TextFemale = strcpy(TextFemale, Item.TextFemale);
			TextFemale[uCount] = '\0';
		} else TextFemale = NULL;

		ID = Item.ID;
		ItemLink = Item.ItemLink;
	}

	CTraItem() 
	{
		for (uint8 i = 0; i < 9; i++)
		{
			MaleSoundRes[i] = '\0';
			FemaleSoundRes[i] = '\0';
		}

		TextFemale = NULL;
		TextMale = NULL;
		ID = 0;
		ItemLink = -1;
	}

	~CTraItem() 
	{
		if (TextMale) delete []TextMale;
		if (TextFemale) delete []TextFemale;
	}

	bool IsEntryEmpty()
	{
		if ((TextMale)&&(TextMale[0] != '\0')) return false;
		if ((TextFemale)&&(TextFemale[0] != '\0')) return false;
		if (MaleSoundRes[0] != '\0') return false;
		if (FemaleSoundRes[0] != '\0') return false;
		if (ItemLink != -1) return false;
		return true;
	}

	bool IsEntryEquale(const CTraItem &Item)
	{
		if ((TextMale == NULL)&&(TextMale != Item.TextMale)) return false;
		else
		if ((TextMale != NULL)&&(Item.TextMale != NULL))
		{
			if (!strEquale(TextMale, Item.TextMale)) { return false; }
		}
		else
		if ((TextMale == Item.TextMale)&&(TextMale != NULL)&&(Item.TextMale != NULL)) return false;

		if ((TextFemale == NULL)&&(TextFemale != Item.TextFemale)) return false;
		else
		if ((TextFemale != NULL)&&(Item.TextFemale != NULL))
		{
			if (!strEquale(TextFemale, Item.TextFemale)) { return false; }
		}
		else
		if ((TextFemale == Item.TextFemale)&&(TextFemale != NULL)&&(Item.TextFemale != NULL)) return false;

		if ((MaleSoundRes[0] == '\0')&&(MaleSoundRes[0] != Item.MaleSoundRes[0])) return false;
		else
		if ((MaleSoundRes[0] != '\0')&&(Item.MaleSoundRes[0] != '\0'))
		{
			if (!strEquale(MaleSoundRes, Item.MaleSoundRes)) { return false; }
		}
		else
		if ((strEquale(MaleSoundRes, Item.MaleSoundRes))&&
			(MaleSoundRes[0] != '\0')&&(Item.MaleSoundRes[0] != '\0')) return false;

		if ((FemaleSoundRes[0] == '\0')&&(FemaleSoundRes[0] != Item.FemaleSoundRes[0])) return false;
		else
		if ((FemaleSoundRes[0] != '\0')&&(Item.FemaleSoundRes[0] != '\0'))
		{
			if (!strEquale(FemaleSoundRes, Item.FemaleSoundRes)) { return false; }
		}
		else
		if ((strEquale(FemaleSoundRes, Item.FemaleSoundRes))&&
			(FemaleSoundRes[0] != '\0')&&(Item.FemaleSoundRes[0] != '\0')) return false;

		return true;
	}

	bool IsTextsEquale(const CTraItem &Item)
	{
		if ((TextMale == NULL)&&(TextMale != Item.TextMale)) return false;
		else
		if ((TextMale != NULL)&&(Item.TextMale != NULL))
		{
			if (!strEquale(TextMale, Item.TextMale)) { return false; }
		}
		else
		if ((TextMale == Item.TextFemale)&&(TextMale != NULL)&&(Item.TextMale != NULL)) return false;

		if ((TextFemale == NULL)&&(TextFemale != Item.TextFemale)) return false;
		else
		if ((TextFemale != NULL)&&(Item.TextFemale != NULL))
		{
			if (!strEquale(TextFemale, Item.TextFemale)) { return false; }
		}
		else
		if ((TextFemale == Item.TextFemale)&&(TextFemale != NULL)&&(Item.TextFemale != NULL)) return false;

		return true;
	}
};

class CTraList: private CDynList
{
public:
	CTraList();
	~CTraList();

	uint32 Append(const CTraItem &Item);
	bool Insert(const uint32 &Index, const CTraItem &Item);
	CTraItem * GetEntry(const uint32 &Index);
	bool GetIndexFromID(const uint32 &ID, uint32 &Index);
	bool SetEntry(const uint32 &Index, const CTraItem &Item);
	uint32 GetEntriesCount();
	bool Remove(const uint32 &Index);
	void Clear();

	uint32 GetNextID();

	bool LoadFile(const char *Filename);
	bool LoadFileWithoutIDs(const char *Filename);
	bool SaveFile(const char *Filename);
	bool SaveFileWithoutIDs(const char *Savename, const char *Filename);

	void SortEntries();
	bool IsEntryExists(const uint32 &ID);
protected:
	void WriteStr2File(FILE *fOut, CTraItem *itm);
};

#endif //_TRA_LIB_H