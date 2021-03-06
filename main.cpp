#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <cmath>

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

const int INITIAL_HEIGHT = 480;
const int INITIAL_WIDTH = 640;

#define WINDOW_MIN_WIDTH  300;
#define WINDOW_MIN_HEIGHT  450;

const int FONTS_AMOUNT = 4;
LPCSTR fontsNames[FONTS_AMOUNT] = {"Times New Roman", "Courier New", "Broadway", "CommercialScript BT"};

int currentMode = MODE_CIRCLE;

void drawCircleText(HDC hdc, HWND hWnd, int x, int y);
void drawTextTable(HDC hdc, HWND hWnd, int rows, int cols);
void addMenu(HWND hWnd);
void onCommand(HWND hWnd, WPARAM wParam);
void WriteFile(const char* fileName);
void loadTable(const char* fileName);
void drawSecondTextTable(HDC hdc, HWND hWnd);
int getColsAmount();
int getMaxElementLength();

typedef std::vector<char*> TTableRow;
typedef std::vector<TTableRow> TTable;

std::vector<int> cellHeights;

TTable textTable;

char* strOutput;
const float INITIAL_RADIUS = 100;
float radius = INITIAL_RADIUS;

const int MAX_LEN = 20; 
char strTemp[2];

int cols = 5;
int rows = 5;

int tableCellWidth;
int widthCellInChars = 11;

int getMaxElementLength(TTableRow row)
{
	int max = 0;
	for(int i = 0; i < row.size(); i++)
	{
		int length = strlen(row[i]);
		if(length > max)
			max = length;
	}	
	return max;
}

void getCellHeights(HDC hdc)
{
	cellHeights.clear();
	TEXTMETRIC tm;
	GetTextMetrics(hdc, &tm);
	int charHeight = tm.tmHeight + tm.tmExternalLeading + tm.tmInternalLeading;
	int charWidth = tm.tmAveCharWidth + tm.tmOverhang;
	widthCellInChars = tableCellWidth / charWidth;
	if(tableCellWidth % charWidth)
		widthCellInChars++;
	for(int i = 0; i < textTable.size(); i++)
	{
		int maxElementLength = getMaxElementLength(textTable[i]);
		int charRowsAmount =  maxElementLength/ widthCellInChars;
	//	if(maxElementLength % widthCellInChars)
			charRowsAmount++;
		cellHeights.push_back(charHeight * charRowsAmount);
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
	
	for(int i = 0; i < textTable.size(); i++)
	{
		for(int j = 0; j < textTable[i].size(); j++)
		{
			free(textTable[i][j]);
		}
	}
	textTable.clear();
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

HFONT getSecondTableFont(HWND hWnd, int fontNum, int cols, int rows)
{
	RECT rect;
	GetClientRect(hWnd, &rect);
	LOGFONT lf;
	lf.lfCharSet = DEFAULT_CHARSET;
	strcpy(lf.lfFaceName, fontsNames[fontNum]);
	lf.lfHeight = 50 * rect.bottom / INITIAL_HEIGHT / rows;
	lf.lfWidth = 50 * rect.right / INITIAL_WIDTH / cols;
	lf.lfWeight = 300;
	lf.lfEscapement = 0;
	lf.lfItalic = 0;
	lf.lfUnderline = 0;
	lf.lfStrikeOut = 0; 

	HFONT hFont = CreateFontIndirect(&lf);
	return hFont;
}

void drawSecondTextTable(HDC hdc, HWND hWnd)
{
	int cols = getColsAmount();
	int rows = textTable.size();
	HFONT hFont = getSecondTableFont(hWnd, 1, cols, rows);
	SelectObject(hdc, hFont);
	RECT rect;
	int tablePenWidth = 2; 
	GetClientRect(hWnd, &rect);
	int width = rect.right / cols;
	int height;
	tableCellWidth = width;
	getCellHeights(hdc);
	RECT cellRect;
	cellRect.top = 0;
	for(int row = 0; row < rows; row++)
	{

		height = cellHeights[row];
		cellRect.bottom = cellRect.top + height;
		for(int col = 0; col < cols; col++)
		{
			HFONT hFont = getSecondTableFont(hWnd, (row * cols + col) % FONTS_AMOUNT, cols, rows);
			SelectObject(hdc, hFont);
			cellRect.left = col * width;
			cellRect.right = cellRect.left + width;
			Rectangle(hdc, cellRect.left, cellRect.top, cellRect.right, cellRect.bottom);
		/*	char buffer[260];
			itoa(height, buffer, 10);
			MessageBoxA(hWnd, buffer, "OK", MB_OK);*/
		//	cellRect.top += height / 2;
			if(col <= textTable[row].size())
			{
				RECT temp = cellRect;
				cellRect.left += tablePenWidth;
				cellRect.right -= tablePenWidth;
				cellRect.bottom -= tablePenWidth;
				cellRect.top += tablePenWidth;
				DrawTextA(hdc, textTable[row][col], -1, &cellRect, DT_CENTER | DT_WORDBREAK);
				cellRect = temp;
			}
			DeleteObject(hFont);
		}
		cellRect.top += height;
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
	}
}

VOID OnTryGetResizeInfo(LPARAM lParam)
{
    LPMINMAXINFO lpMMI;
    lpMMI = (LPMINMAXINFO)lParam;
    lpMMI->ptMinTrackSize.x = WINDOW_MIN_WIDTH;
    lpMMI->ptMinTrackSize.y = WINDOW_MIN_HEIGHT;
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
		case WM_GETMINMAXINFO: 
			OnTryGetResizeInfo(lParam);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
			break;
	}
	return 0;
}

HFONT getCircleFont(HWND hWnd)
{
	RECT rect;
	GetClientRect(hWnd, &rect);
	LOGFONT lf;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfPitchAndFamily = FIXED_PITCH;
	strcpy(lf.lfFaceName, "Courier New");
	lf.lfHeight = 20 * rect.bottom / INITIAL_HEIGHT;
	lf.lfWidth = 15 * rect.right / INITIAL_WIDTH;
	lf.lfWeight = 400;
	lf.lfEscapement = 0;
	lf.lfItalic = 0;
	lf.lfUnderline = 0;
	lf.lfStrikeOut = 0; 

	HFONT hFont = CreateFontIndirect(&lf);
	return hFont;
}

void drawCircleText(HDC hdc, HWND hWnd, int x, int y)
{	

	HFONT hFont = getCircleFont(hWnd);
	SelectObject(hdc, hFont);
	
	float angle = 3.1415 / 4;
	
	RECT rect;
	GetClientRect(hWnd, &rect);
	if(rect.bottom / INITIAL_HEIGHT > rect.right / INITIAL_WIDTH)
	{
		radius = INITIAL_RADIUS * rect.bottom / INITIAL_HEIGHT;
	}
	else
	{
		radius = INITIAL_RADIUS * rect.right / INITIAL_WIDTH;
	}
	
	XFORM xform;
	
	xform.eM11 = cos(angle);
	xform.eM12 = sin(angle);
	xform.eM21 = -sin(angle);
	xform.eM22 = cos (angle);
	xform.eDx = 0;
	xform.eDy = 0;
	SetGraphicsMode(hdc, GM_ADVANCED);
	SetWorldTransform(hdc, &xform);
	
	int amount = strlen(strOutput);
	float deltaAngle = 2 * PI / MAX_LEN;
	for(int  i = 0; i < amount; i++)
	{
		angle = i * deltaAngle;
		rect.left = -20;
		rect.top = -20;
		rect.right = 20;
		rect.bottom = 20;
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
		INITIAL_WIDTH, 
		INITIAL_HEIGHT, 
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
