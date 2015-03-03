#ifndef _TLK_V3_LIB_
#define _TLK_V3_LIB_

#include "ECL/types_common.h"
#include "ECL/dyn_list.h"

#include "ECL/strfuncs.h"

#ifndef _TLK_LANGS_DEFINED_
#define _TLK_LANGS_DEFINED_

#define TLK_LANG_ENGLISH				0
#define TLK_LANG_FRENCH					1
#define TLK_LANG_GERMAN					2
#define TLK_LANG_ITALIAN				3
#define TLK_LANG_SPANISH				4
#define TLK_LANG_POLISH					5
#define TLK_LANG_KOREAN					128
#define TLK_LANG_CHINESE_TRADITIONAL	129
#define TLK_LANG_CHINESE_SIMPLIFIED		130
#define TLK_LANG_JAPANESE				131

#endif

struct CTLKItemV3
{
	uint32 Flags;
	char SoundResRef[17];
	uint32 VolumeVariance;
	uint32 PitchVariance;
	uint32 OffsetToString;
	uint32 StringSize;
	float SoundLength;
	char *Text;

	CTLKItemV3(const CTLKItemV3 &Item) 
	{
		for (uint8 i = 0; i < 17; i++)
			SoundResRef[i] = Item.SoundResRef[i];

		if (Item.Text)
		{
			uint32 uCount = strlen(Item.Text);
			Text = new char[uCount + 1];
			Text = strcpy(Text, Item.Text);
			Text[uCount] = '\0';
		} else Text = NULL;

		Flags = Item.Flags;
		VolumeVariance = Item.VolumeVariance;
		PitchVariance = Item.PitchVariance;
		OffsetToString = Item.OffsetToString;
		StringSize = Item.StringSize;
		SoundLength = Item.SoundLength;
	}

	CTLKItemV3() 
	{
		for (uint8 i = 0; i < 17; i++)
			SoundResRef[i] = '\0';

		Text = NULL;
		Flags = 0;
		VolumeVariance = 0;
		PitchVariance = 0;
		OffsetToString = 0;
		StringSize = 0;
		SoundLength = 0.0;
	}

	~CTLKItemV3() 
	{
		if (Text) delete []Text;
	}
};

const uint8 TLKHeaderV3_Size = 20;
const uint8 TLKEntryV3_Size = 40;

class CTLKFileV3 : private CDynList
{
public:
	uint32 LanguageID;
public:
	CTLKFileV3();
	~CTLKFileV3();

	uint32 Append(const CTLKItemV3 &Item);
	bool Insert(const uint32 &Index, const CTLKItemV3 &Item);
	CTLKItemV3 * GetEntry(const uint32 &Index);
	bool SetEntry(const uint32 &Index, const CTLKItemV3 &Item);
	uint32 GetEntriesCount();
	bool Remove(const uint32 &Index);
	void Clear();

	bool IsReadOnly();
	uint32 GetContentOffset();

	bool LoadFile(const char * Filename);
	bool SaveFile(const char * Filename);
};

#endif //_TLK_V3_LIB_