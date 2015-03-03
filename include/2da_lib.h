#ifndef _2DA_LIB_H_
#define _2DA_LIB_H_

#include "./ECL/dyn_list.h"
#include <stdio.h>

#include <string.h>

typedef char *LPSTR;
typedef const char *LPCSTR;
typedef char CHAR;

struct _2DAEntry
{
	uint32 uIndex;
	uint8 uValuesCount;
	LPSTR *Values;
	_2DAEntry() : uIndex(0), uValuesCount(0), Values(NULL) {}
	_2DAEntry(const _2DAEntry &Entry) : uIndex(Entry.uIndex), uValuesCount(Entry.uValuesCount)
	{
		this->Values = new LPSTR[this->uValuesCount];
		uint32 uCount = 0;
		for (uint32 i = 0; i < Entry.uValuesCount; i++)
		{
			uCount = strlen(Entry.Values[i]);
			this->Values[i] = new CHAR[uCount + 1];
			this->Values[i] = strcpy(this->Values[i], Entry.Values[i]);
			this->Values[i][uCount] = '\0';
		}
	}
};

class C2DAFile: public CDynList
{
public:
	C2DAFile();
	~C2DAFile();

	bool Load2DA(LPCSTR Filename);
	bool Save2DA(LPCSTR Filename);

	uint32 AddEntry(const _2DAEntry &Item);
	_2DAEntry * GetEntry(const uint32 &Index);
	uint32 GetEntriesCount();
	bool RemoveEntry(const uint32 &Index);
	void Clear();

	void Sort();
public:
	_2DAEntry *Header;
protected:
	_2DAEntry * ReadEntry(FILE *fIn);
	LPSTR ReadValue(FILE *fIn);
};

#endif //_2DA_LIB_H_
