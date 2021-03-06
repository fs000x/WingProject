
#include "stdafx.h"



// http://www.codeproject.com/Articles/12093/Using-RichEditCtrl-to-Display-Formatted-Logs
int AppendToLogAndScroll(CRichEditCtrl* pCtrl, CString str, COLORREF color)
{
	long nVisible = 0;
	long nInsertionPoint = 0;
	CHARFORMAT cf;

	// Initialize character format structure
	cf.cbSize = sizeof(CHARFORMAT);
	cf.dwMask = CFM_COLOR;
	cf.dwEffects = 0; // To disable CFE_AUTOCOLOR

	cf.crTextColor = color;

	// Set insertion point to end of text
	nInsertionPoint = pCtrl->GetWindowTextLength();
	pCtrl->SetSel(nInsertionPoint, -1);

	// Set the character format
	pCtrl->SetSelectionCharFormat(cf);

	// Replace selection. Because we have nothing
	// selected, this will simply insert
	// the string at the current caret position.
	pCtrl->ReplaceSel(str);

	// Get number of currently visible lines or maximum number of visible lines
	// (We must call GetNumVisibleLines() before the first call to LineScroll()!)
	nVisible = GetNumVisibleLines(pCtrl);

	// Now this is the fix of CRichEditCtrl's abnormal behaviour when used
	// in an application not based on dialogs. Checking the focus prevents
	// us from scrolling when the CRichEditCtrl does so automatically,
	// even though ES_AUTOxSCROLL style is NOT set.
	if (pCtrl != pCtrl->GetFocus())
	{
		pCtrl->LineScroll(INT_MAX);
		pCtrl->LineScroll(1 - nVisible);
	}

	// 내용이 너무 많으면 지운다.
	const int maximumLine = 100;
	if (pCtrl->GetLineCount() > maximumLine)
	{
		long nFirstChar = pCtrl->CharFromPos(CPoint(0, 0));
		pCtrl->SetSel(0, nFirstChar);
		pCtrl->ReplaceSel(L"");
	}

	return 0;
}


//http://www.codeproject.com/Articles/12093/Using-RichEditCtrl-to-Display-Formatted-Logs
int GetNumVisibleLines(CRichEditCtrl* pCtrl)
{
	CRect rect;
	long nFirstChar, nLastChar;
	long nFirstLine, nLastLine;

	// Get client rect of rich edit control
	pCtrl->GetClientRect(rect);

	// Get character index close to upper left corner
	nFirstChar = pCtrl->CharFromPos(CPoint(0, 0));

	// Get character index close to lower right corner
	nLastChar = pCtrl->CharFromPos(CPoint(rect.right, rect.bottom));
	if (nLastChar < 0)
	{
		nLastChar = pCtrl->GetTextLength();
	}

	// Convert to lines
	nFirstLine = pCtrl->LineFromChar(nFirstChar);
	nLastLine = pCtrl->LineFromChar(nLastChar);

	return (nLastLine - nFirstLine);
}


// Naze32 CLI 명령 전송
void SendCommand(CSerial &serial, const unsigned char cmd)
{
	unsigned char packet[64];
	int checksum = 0;
	int idx = 0;
	packet[idx++] = '$';
	packet[idx++] = 'M';
	packet[idx++] = '<';
	packet[idx++] = 0;
	checksum ^= 0;
	packet[idx++] = cmd;
	checksum ^= cmd;
	packet[idx++] = checksum;
	serial.SendData((char*)packet, idx);
}


// 모터 정보를 받는다.
// Naze32 CLI 정보 수신
// return value : 0 정보 수신중
//						   n 수신된 정보 수
//						   -1 수신 완료, 실패
int RecvCommand(CSerial &serial, const unsigned char cmd, OUT unsigned char buffer[], const int maxLen)
{
	if (!serial.IsOpened())
		return 0;

	int state = 0;
	int len = 0;
	int readLen = 0;
	int msp = 0;
	int noDataCnt = 0;
	int checkSum = 0;
	while (1)
	{
		unsigned char c;
		if (serial.ReadData(&c, 1) <= 0)
		{
			Sleep(1);
			++noDataCnt;
			if (noDataCnt > 100)
				break; // exception
			continue;
		}

		switch (state)
		{
		case 0:
		{
			state = (c == '$') ? 1 : 0;
			//cout << c;
		}
		break;

		case 1:
		{
			state = (c == 'M') ? 2 : 0;
			//cout << c;
		}
		break;

		case 2:
		{
			state = (c == '>') ? 3 : 0;
			//cout << c;
		}
		break;

		case 3:
		{
			len = c;
			//cout << (int)c;
			checkSum ^= c;
			state = 4;
		}
		break;

		case 4:
		{
			msp = c;
			//cout << (int)c << " ";
			checkSum ^= c;
			state = 5;
		}
		break;

		case 5:
		{
			if (len > readLen)
			{
				checkSum ^= c;
				if (readLen < maxLen)
					buffer[readLen] = c;

				//cout << (int)c << " ";
			}
			else
			{
				if (checkSum == c)
				{
					if (msp != cmd)
						return 0;
					return readLen; // end;
				}
				else
				{
					if (msp != cmd)
						return 0;
					return -1; // end;
				}
			}

			++readLen;
		}
		break;

		default:
			break;
		}
	}

	return 0;
}

