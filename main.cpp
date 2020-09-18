#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.1415

void drawCircleText(HDC hdc, HWND hWnd, int x, int y);

char* strHello = "";
const char* strAuthor = "Made by Eugene";
char* strOutput;
const float MIN_RADIUS = 50;
float radius = MIN_RADIUS;

char strTemp[2];

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
			
			LOGFONT lf;
			lf.lfCharSet = DEFAULT_CHARSET;
			lf.lfPitchAndFamily = FIXED_PITCH;
			strcpy(lf.lfFaceName, "Courier New");
			lf.lfHeight = 20;
			lf.lfWidth = 10;
			lf.lfWeight = 40;
			lf.lfEscapement = 0;
			lf.lfItalic = 1;
			lf.lfUnderline = 0;
			lf.lfStrikeOut = 0; 
			
			HFONT hFont = CreateFontIndirect(&lf);
			SelectObject(ps.hdc, hFont);
			drawCircleText(ps.hdc, hWnd, rect.right / 2, rect.bottom / 2);
		//	TextOut(ps.hdc, 300, 300, (LPCSTR)strHello, 5);

			DeleteObject(hFont);
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
			strTemp[0] = wParam;
			strTemp[1] = 0;
			strcat(strOutput, strTemp);
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		}
	/*	case WM_MOUSEWHEEL:
		{
			short delta = HIWORD(wParam);
			if(delta > 0)
			{
				radius += 0.5;
			}
			else if(delta < 0)
			{
				radius -= 0.5;
			}
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		}*/
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
			break;
	}
	return 0;
}

char *readFile(const char* fileName)
{
	FILE *f;
	f = fopen(fileName, "r");
	fseek(f, 0, SEEK_END);
	int length = ftell(f);
	fseek(f, 0, SEEK_SET);
	char* buffer = (char*)malloc(length);
	fread(buffer, 1, length, f);
	fclose(f);
	return buffer;
}

void drawCircleText(HDC hdc, HWND hWnd, int x, int y)
{	
	float radius = 50;
	float angle = 3.1415 / 4;
	
	SetGraphicsMode(hdc, GM_ADVANCED);
	
	XFORM xform;
	
	xform.eM11 = cos(angle);
	xform.eM12 = sin(angle);
	xform.eM21 = -sin(angle);
	xform.eM22 = cos (angle);
	xform.eDx = 0;
	xform.eDy = 0;
	SetWorldTransform(hdc, &xform);
	
	RECT rect;
	GetClientRect(hWnd, &rect);
	LPtoDP(hdc, (LPPOINT)&rect, 2);
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
	
	strOutput = (char*)malloc(strlen(strHello));
	strcpy(strOutput, strHello);
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
