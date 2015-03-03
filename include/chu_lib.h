#ifndef _CHU_LIB_
#define _CHU_LIB_

#include "types_common.h"
#include <vector>

using namespace std;
const char szDefaultSign[] = "CHUIV1  ";

struct CHUWindow
{
	char Name[9]; //* Этого не было в IESDP
	uint16 ID;
	uint16 Unknown1;
	uint16 X;
	uint16 Y;
	uint16 Width;
	uint16 Height;
	uint16 HasBackground; //На самом деле нерациональный флаг %)
	uint16 CtrlCount;
	char Background[9];
	uint16 FirstCtrlID;
	uint16 Unknown2;
};

//Все доступные ЭУ
struct CtrlButton //Кнопка [0]
{
	char Image[9];
	uint8 AnimationCycle;
	uint8 TextJustification;
	uint16 btnUnpressed;
	uint16 btnPressed;
	uint16 btnSelected;
	uint16 btnDisabled;
};

struct CtrlSlider //Текстовое поле с числом (spinbox) [2]
{
	char Background[9];
	char Button[9];
	uint16 CycleNumb;
	uint16 SliderUngrabbed;
	uint16 SliderGrabbed;
	uint16 ButtonX;
	uint16 ButtonY;
	uint16 ButtonJumpWidth;
	uint16 ButtonJumpCount;
	uint32 Unknown1;
	uint32 Unknown2;
};

struct CtrlTextEdit //Текстовое поле [3]
{
	char Background1[9];
	char Background2[9];
	char Background3[9];
	char Cursor[9];
	uint32 Unknown1;
	uint16 X;
	uint16 Y;
	uint32 Unknown2;
	char Font[9];
	char Unknown[35];
	uint16 MaxInputLen;
	uint32 Unknown3;
};

struct CHUControl
{
	uint32 Offset;
	uint32 Length;

	uint32 ID;
	uint16 X;
	uint16 Y;
	uint16 Width;
	uint16 Height;
	uint8 Type;
	uint8 Unknown;

	union
	{
		CtrlTextEdit TextEdit;
		CtrlSlider Slider;
		CtrlButton Button;
	};
};

class CHUFile
{
protected:
	uint32 uOffsetToCtrlTable;
	uint32 uOffsetToWndTable;
public:
	CHUFile() {}
	~CHUFile() {}

	bool LoadFile(LPCSTR path);
	bool SaveFile(LPCSTR path);

	uint32 GetOffsetToCtrlTable() { return uOffsetToCtrlTable; }
	uint32 GetOffsetToWndTable() { return uOffsetToWndTable; }
public:
	vector <CHUWindow> Windows;
	vector <CHUControl> Controls;
};

#endif