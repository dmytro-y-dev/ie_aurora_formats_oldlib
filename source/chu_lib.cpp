#include "chu_lib.h"
#include "strfuncs.h"
#include <stdio.h>

bool CHUFile::LoadFile(LPCSTR path)
{
	FILE *fIn = fopen(path, "rb");
	if (!fIn) return false;
	char Sign[9]; Sign[8] = '\0';
	fread(Sign, sizeof(char), 8, fIn);

	if (!strEquale(Sign, szDefaultSign))
	{
		fclose(fIn);
		return false;
	}
	else
	{
		uint32 uWindowsCount;
		fread(&uWindowsCount, sizeof(uint32), 1, fIn);
		fread(&uOffsetToCtrlTable, sizeof(uint32), 1, fIn);
		fread(&uOffsetToWndTable, sizeof(uint32), 1, fIn);

		fseek(fIn, uOffsetToWndTable, 0);
		for (uint32 i = 0; i < uWindowsCount; i++)
		{
			CHUWindow wnd;
			fread(&wnd.Name, sizeof(char), 8, fIn);
			wnd.Name[8] = '\0';

			fread(&wnd.ID, sizeof(uint16), 1, fIn);
			fread(&wnd.Unknown1, sizeof(uint16), 1, fIn);
			fread(&wnd.X, sizeof(uint16), 1, fIn);
			fread(&wnd.Y, sizeof(uint16), 1, fIn);
			fread(&wnd.Width, sizeof(uint16), 1, fIn);
			fread(&wnd.Height, sizeof(uint16), 1, fIn);
			fread(&wnd.HasBackground, sizeof(uint16), 1, fIn);
			fread(&wnd.CtrlCount, sizeof(uint16), 1, fIn);

			fread(&wnd.Background, sizeof(char), 8, fIn);
			wnd.Background[8] = '\0';

			fread(&wnd.FirstCtrlID, sizeof(uint16), 1, fIn);
			fread(&wnd.Unknown2, sizeof(uint16), 1, fIn);
			Windows.push_back(wnd);
		}

		uint32 BaseOffset, Length; fread(&BaseOffset, sizeof(uint32), 1, fIn);
		uint32 uItemsCount = (BaseOffset - uOffsetToCtrlTable) / 8;
		fread(&Length, sizeof(uint32), 1, fIn);

		CHUControl ctrl; ctrl.Length = Length; ctrl.Offset = BaseOffset;
		Controls.push_back(ctrl);

		for (uint32 i = 1; i < uItemsCount; i++)
		{
			fread(&ctrl.Offset, sizeof(uint32), 1, fIn);
			fread(&ctrl.Length, sizeof(uint32), 1, fIn);
			Controls.push_back(ctrl);
		}

		for (uint32 i = 0; i < Controls.size(); i++)
		{
			fread(&Controls[i].ID, sizeof(uint32), 1, fIn);
			fread(&Controls[i].X, sizeof(uint16), 1, fIn);
			fread(&Controls[i].Y, sizeof(uint16), 1, fIn);
			fread(&Controls[i].Width, sizeof(uint16), 1, fIn);
			fread(&Controls[i].Height, sizeof(uint16), 1, fIn);
			fread(&Controls[i].Type, sizeof(uint8), 1, fIn);
			fread(&Controls[i].Unknown, sizeof(uint8), 1, fIn);

			if (Controls[i].Type == 0) //Button
			{
				fread(Controls[i].Button.Image, sizeof(char), 8, fIn);
				Controls[i].Button.Image[8] = '\0';

				fread(&Controls[i].Button.AnimationCycle, sizeof(uint8), 1, fIn);
				fread(&Controls[i].Button.TextJustification, sizeof(uint8), 1, fIn);
				fread(&Controls[i].Button.btnUnpressed, sizeof(uint16), 1, fIn);
				fread(&Controls[i].Button.btnPressed, sizeof(uint16), 1, fIn);
				fread(&Controls[i].Button.btnSelected, sizeof(uint16), 1, fIn);
				fread(&Controls[i].Button.btnDisabled, sizeof(uint16), 1, fIn);
			}
		}
	}

	fclose(fIn);
	return true;
}

bool CHUFile::SaveFile(LPCSTR path)
{
	return false;
}