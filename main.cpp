#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>

#define PI 3.1415

#define TABLE_DELIMITER '|'

#define MODE_CIRCLE 0
#define MODE_TABLE1 1
#define MODE_TABLE2 2

#define IDM_CIRCLE 0
#define IDM_TABLE1 1
#define IDM_TABLE2 2
#define IDM_SAVE 3
#define IDM_OPEN 4

int currentMode = MODE_CIRCLE;

void drawCircleText(HDC hdc, HWND hWnd, int x, int y);
void drawTextTable(HDC hdc, HWND hWnd, int rows, int cols);
void addMenu(HWND hWnd);
void onCommand(HWND hWnd, WPARAM wParam);
void WriteFile(const char* fileName);
void loadTable(const char* fileName);
void drawSecondTextTable(HDC hdc, HWND hWnd);
int getColsAmount();
char* showOpenTableDialog(HWND hWnd);

typedef std::vector<char*> TTableRow;
typedef std::vector<TTableRow> TTable;

TTable textTable;

char* strOutput;
float radius = 50;

const int MAX_LEN = 20; 
char strTemp[2];

int cols = 5;
int rows = 5;

char* showOpenTableDialog(HWND hWnd)
{
	OPENFILENAME ofn;
    char szFile[260];

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = szFile;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	
	
	if(GetOpenFileName(&ofn) == TRUE)
	{
		loadTable(szFile);
		return ofn.lpstrFile;
	}
	else
	{
		DWORD error = CommDlgExtendedError();
		char buffer[256];
		itoa(error, buffer, 16);
		MessageBox(hWnd, buffer, "Error", MB_OK | MB_ICONERROR);
		return NULL;
	}
}

void loadTable(const char* fileName)
{
	FILE *f;
	f = fopen(fileName, "r");
	if(!f)
		return;
	fseek(f, 0, SEEK_END);
	int length = ftell(f);
	char* buffer = (char*)malloc(length);
	fseek(f, 0, SEEK_SET);
	fread(buffer, 1, length, f);
	fclose(f);
	buffer[length] = 0;
	
	TTableRow* row = new TTableRow();
	char c;
	std::vector<char>* strTemp = new std::vector<char>();
	for(int  i = 0; i < length; i++)
	{
		c = buffer[i];
		if(c == TABLE_DELIMITER)
		{
			strTemp->push_back(0);
			row->push_back((char*)&strTemp->at(0));
			strTemp = new std::vector<char>();
		}
		else if(c == 10)
		{
			strTemp->push_back(0);
			row->push_back((char*)&strTemp->at(0));
			textTable.push_back(*row);
			row = new TTableRow();
			strTemp = new std::vector<char>();
		}
		else
		{
			strTemp->push_back(c);
		}
	}
	strTemp->push_back(0);
	row->push_back(&strTemp->at(0));
	textTable.push_back(*row);
}

int getColsAmount()
{
	int colsAmount = 0;
	TTable::iterator tableIterator = textTable.begin();
	while(tableIterator != textTable.end())
	{
		if(tableIterator->size() > colsAmount)
		{
			colsAmount = tableIterator->size();
		}
		tableIterator++;
	}
	return colsAmount;
}

void drawSecondTextTable(HDC hdc, HWND hWnd)
{
	int cols = getColsAmount();
	int rows = textTable.size();
	RECT rect;
	GetClientRect(hWnd, &rect);
	int width = rect.right / cols;
	int height = rect.bottom / rows;
	for(int row = 0; row < rows; row++)
	{
		for(int col = 0; col < cols; col++)
		{
			RECT cellRect;
			cellRect.left = col * width;
			cellRect.right = cellRect.left + width;
			cellRect.top = row * height;
			cellRect.bottom = cellRect.top + height;
			Rectangle(hdc, cellRect.left, cellRect.top, cellRect.right, cellRect.bottom);
			cellRect.top += height / 2;
			if(col <= textTable[row].size())
				DrawTextA(hdc, textTable[row][col], -1, &cellRect, DT_CENTER | DT_WORDBREAK);
		}
	}
}

void addMenu(HWND hWnd)
{
	HMENU hMenu = CreateMenu();
	HMENU hModeSubmenu = CreatePopupMenu();
	AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hModeSubmenu, "ChangeMode");
	AppendMenuA(hModeSubmenu, MF_STRING, IDM_CIRCLE, "Circle");
	AppendMenuA(hModeSubmenu, MF_STRING, IDM_TABLE1, "Table1");
	AppendMenuA(hModeSubmenu, MF_STRING, IDM_TABLE2, "Table2");
	AppendMenuA(hMenu, MF_STRING, IDM_SAVE, "Save");
	AppendMenuA(hMenu, MF_STRING, IDM_OPEN, "Open");
	SetMenu(hWnd, hMenu);
}

void onCommand(HWND hWnd, WPARAM wParam)
{
	switch (wParam)
	{
		case IDM_CIRCLE:
			currentMode = MODE_CIRCLE;
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		case IDM_TABLE1:
			currentMode = MODE_TABLE1;
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		case IDM_TABLE2:
			currentMode = MODE_TABLE2;
			loadTable("table.txt");
		//	MessageBox(hWnd, "Table2 not implemented yet", "Sorry", MB_OK | MB_ICONINFORMATION);
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		case IDM_SAVE:
			WriteFile("Output.txt");
			MessageBox(hWnd, "Saved", "OK", MB_OK | MB_ICONINFORMATION);
			break;
		case IDM_OPEN:
		{
			char* fileName;
			fileName = showOpenTableDialog(hWnd);
			if(fileName)
				MessageBox(hWnd, fileName, "OK", MB_OK | MB_ICONINFORMATION);
			else
				MessageBox(hWnd, "Something wrong", "Not ok", MB_OK | MB_ICONERROR);
			break;
		}
		
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	switch(message)
	{
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}
		case WM_PAINT:
		{
			BeginPaint(hWnd, &ps);
			RECT rect;
			GetClientRect(hWnd, &rect);
			
			if(currentMode == MODE_TABLE1)
			{
				drawTextTable(ps.hdc, hWnd, rows, cols);
			}
			else if (currentMode == MODE_CIRCLE)
			{
				drawCircleText(ps.hdc, hWnd, rect.right / 2, rect.bottom / 2);	
			}
			else if(currentMode == MODE_TABLE2)
			{
				drawSecondTextTable(GetDC(hWnd), hWnd);
			}
			EndPaint(hWnd, &ps);
			break;
		}
		case WM_SIZE:
		{
			RECT rect;
			GetClientRect(hWnd, &rect);
			InvalidateRect(hWnd, &rect, TRUE);
			break;
		}
		case WM_CHAR:
		{
			if(wParam == VK_BACK)
			{
				strOutput[strlen(strOutput) - 1] = 0;
				InvalidateRect(hWnd, NULL, TRUE);
				break;
			}
			if(strlen(strOutput) >= MAX_LEN)
				break;
			strTemp[0] = wParam;
			strTemp[1] = 0;
			strcat(strOutput, strTemp);
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		}
		case WM_COMMAND:
		{
			onCommand(hWnd, wParam);
			break;
		}
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
			break;
	}
	return 0;
}

HFONT getCircleFont()
{
	LOGFONT lf;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfPitchAndFamily = FIXED_PITCH;
	strcpy(lf.lfFaceName, "Courier New");
	lf.lfHeight = 20;
	lf.lfWidth = 10;
	lf.lfWeight = 400;
	lf.lfEscapement = 0;
	lf.lfItalic = 1;
	lf.lfUnderline = 0;
	lf.lfStrikeOut = 0; 
				
	HFONT hFont = CreateFontIndirect(&lf);
	return hFont;
}

void drawCircleText(HDC hdc, HWND hWnd, int x, int y)
{	

	HFONT hFont = getCircleFont();
	SelectObject(hdc, hFont);
	
	float angle = 3.1415 / 4;
	
	
	XFORM xform;
	
	xform.eM11 = cos(angle);
	xform.eM12 = sin(angle);
	xform.eM21 = -sin(angle);
	xform.eM22 = cos (angle);
	xform.eDx = 0;
	xform.eDy = 0;
	SetGraphicsMode(hdc, GM_ADVANCED);
	SetWorldTransform(hdc, &xform);
	
	RECT rect;
	GetClientRect(hWnd, &rect);
	int amount = strlen(strOutput);
	float deltaAngle = 2 * PI / amount;
	for(int  i = 0; i < amount; i++)
	{
		angle = i * deltaAngle;
		rect.left = -10;
		rect.top = -10;
		rect.right = 10;
		rect.bottom = 10;
		xform.eM11 = cos(angle + PI / 2);
		xform.eM12 = sin(angle + PI / 2);
		xform.eM21 = -sin(angle + PI / 2);
		xform.eM22 = cos (angle + PI / 2);
		xform.eDx = cos(angle) * radius + x;
		xform.eDy = sin(angle) * radius + y;
		SetWorldTransform(hdc, &xform);
		DrawText(hdc, strOutput + i, 1, &rect, DT_CENTER);
	}
	
	DeleteObject(hFont);
}

//m - rows, n - cols
void drawTextTable(HDC hdc, HWND hWnd, int rows, int cols)
{
	RECT rect;
	GetClientRect(hWnd, &rect);
	
	int width = rect.right / cols;
	int height = rect.bottom / rows;
	
	LOGFONT lf;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfPitchAndFamily = FIXED_PITCH;
	strcpy(lf.lfFaceName, "Times New Roman");
	lf.lfHeight = height * 0.5;
	lf.lfWidth = width * 0.5;
	lf.lfWeight = 400;
	lf.lfEscapement = 0;
	lf.lfItalic = 0;
	lf.lfUnderline = 0;
	lf.lfStrikeOut = 0;
	
	HFONT hFont = CreateFontIndirect(&lf);
	SelectObject(hdc, hFont);
	
	int length = strlen(strOutput);
	for(int i = 0; i < rows; i++)
		for(int j = 0; j < cols; j++)
		{
			int index = i * cols + j;
			if(index < length)
				TextOut(hdc, j * width, i * height, strOutput + index, 1);
		}
	
	
	DeleteObject(hFont); 
}

void WriteFile(const char* fileName)
{
	FILE *f;
	f = fopen(fileName, "w");
	fwrite(strOutput, 1, strlen(strOutput), f);
	fclose(f);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow)
{
WNDCLASSEX wc; 

	memset(&wc,0,sizeof(wc));
	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.lpfnWndProc	 = WndProc; 
	wc.hInstance	 = hInstance;
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszClassName = "WindowClass";
	wc.hIcon		 = LoadIcon(NULL, IDI_APPLICATION); 
	wc.hIconSm		 = LoadIcon(NULL, IDI_APPLICATION); 

	if(!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Window Registration Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	HWND hWnd; 
	hWnd = CreateWindowEx(WS_EX_CLIENTEDGE,"WindowClass","Laba2_OSiSP", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 
		CW_USEDEFAULT, 
		640, 
		480, 
		NULL,NULL,hInstance,NULL);

	if(hWnd == NULL) { 
		MessageBox(NULL, "Window Creation Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}
	
	addMenu(hWnd);
	
	strOutput = (char*)malloc(2);
	strcpy(strOutput, " ");
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	
	MSG msg;
	while(GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	return msg.wParam;
}
