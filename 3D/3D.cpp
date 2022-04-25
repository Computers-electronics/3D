#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <process.h> 
#include <mmsystem.h>
#include "Resource.h"
#pragma comment(lib, "winmm.lib")
#pragma warning(disable : 4996)
#define HIBA_00 TEXT("Error:Program initialisation process.")
HINSTANCE hInstGlob;
int SajatiCmdShow;
char szClassName[] = "WindowsApp";
HWND Form1;
HINSTANCE hInst;
LRESULT CALLBACK WndProc0(HWND, UINT, WPARAM, LPARAM);
const char* str = "obj.obj";
RECT rc;
HDC hdcMem;
HBITMAP hbmMem, hbmOld;
HBRUSH hbrBkGnd;
HDC hdc;
PAINTSTRUCT ps;
int rajzolas_folyamatban = 0;
#define MAX_OBJ_NUM 10000000
#define VERTEX_NUMBER 1000
int beolvpont_index[MAX_OBJ_NUM];//beolvasott vertexek számát tárolja, alakzatonként, sorfolytonosan
float beolv_pontok[MAX_OBJ_NUM][3];//beolvasott vertexeket tárolja, alakzatonként, sorfolytonosan
float nyers_csucspontok[MAX_OBJ_NUM][3];//objektum cache: ugyanaz, mint a be-olv_pontok, de már a 3D-hez igazitva. Minden renderelés ebbõl indul.
int nyers_csucspontok_index[MAX_OBJ_NUM];
int nyers_csucspontok_tipus[MAX_OBJ_NUM];
int nyers_csucspontok_offset[MAX_OBJ_NUM];
int nyers_csucspontok_szinek[MAX_OBJ_NUM];
float forgatott_csucspontok[MAX_OBJ_NUM][3];// a nyers_negyzetek elforgatott koordiná-tái
float lekepezett_csucspontok[MAX_OBJ_NUM][3];// a kirajzolando negyzetek kepernyoko-ordinátái
float zorder[MAX_OBJ_NUM][2];// mélységi rendezésre használt segédtomb
int nezopont = -1445;
int raw_vertex_counter;
float persp_szog;
float fszog_x;
float fszog_y;
float fszog_z;
float fszog_x2;
float fszog_y2;
float fszog_z2;
float Math_PI = 3.14159265358979323846;
int global_offset = 0;
int nyers_csucspontok_szinek_length;
int nyers_csucspontok_index_length;
int nyers_csucspontok_tipus_length;
int nyers_csucspontok_offset_length;
int nyers_csucspontok_length;
int beolv_pontok_length;
int lekepezett_csucspontok_length;
int zorder_length;
int forgatott_csucspontok_length;
int beolvpont_index_length;
int stage_stageWidth = 1920;
int stage_stageHeight = 1050;
int joystick_mode = 0; //0-forgatas,1-lepkedes
int jelzo;
int szamol1_length;
int kezdet;
int vege;
long int szamlalo_vert, szamlalo_poly, szamlalo_zorder, szamlalo_fps;//statisztikák
float fps_stat;
char tempstr[200];
void init(void);
void cleanup_matrices(void);
void kep_frissit(void);
void trans_matrix_create(void);
void perspektivikus_projekcio(void);
void parhuzamos_projekcio(void);
void rajzolas(void);
void kepernyo_clear(void);
void z_ordering(void);
void rendezes_shell(void);
void forgatas();
void draw_3d_objects(int tipus);
void data_load(void);
void nagyitas(void);
void kicsinyites(void);
void DB_prepare_puffer(void);
void DB_start_drawing(void);
void DB_end_drawing(void);
FILE* fajl;
unsigned char sor1[1024], sor2[1024], sor3[1024], sor4[1024];
int s1, s2, elemszam, maxsorszelesseg = 250;
float tomb_vertices[300000][3];
int tomb_faces[300000][500];
int tomb_vertices_length = 0, tomb_faces_length = 0;
int getelementcount(unsigned char csv_szoveg[]);
void getelement(unsigned char csv_szoveg[], unsigned int hanyadik, unsigned char csv_szoveg2[]);
void obj_loader(void);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	static TCHAR szAppName[] = TEXT("StdWinClassName");
	MSG msg;
	WNDCLASS wndclass0;
	SajatiCmdShow = iCmdShow;
	hInstGlob = hInstance;
	hInst = hInstance;
	wndclass0.style = CS_HREDRAW | CS_VREDRAW;
	wndclass0.lpfnWndProc = WndProc0;
	wndclass0.cbClsExtra = 0;
	wndclass0.cbWndExtra = 0;
	wndclass0.hInstance = hInstance;
	wndclass0.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass0.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass0.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass0.lpszMenuName = NULL;
	wndclass0.lpszClassName = TEXT("WIN0");

	if (!RegisterClass(&wndclass0))
	{
		MessageBox(NULL, HIBA_00, TEXT("Program Start"), MB_ICONERROR); return 0;
	}

	Form1 = CreateWindow(TEXT("WIN0"),
		TEXT("Form1"),
		(WS_OVERLAPPED | WS_SYSMENU | WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX),
		-10,
		-10,
		1920,
		1050,
		NULL,
		NULL,
		hInstance,
		NULL);

	//************************
	//dupla pufferelés inicializálása
	//************************
	DB_prepare_puffer();

	ShowWindow(Form1, 3);
	//SetWindowTextA(Form1, (LPCSTR)hInstance);
	UpdateWindow(Form1);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}
LRESULT CALLBACK fSzog(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			kep_frissit();
			EndDialog(hwnd, LOWORD(wParam));
			return (INT_PTR)TRUE;

		}
		if (LOWORD(wParam) == 3)
		{
			fszog_x2 = HIWORD(wParam);
			return (INT_PTR)TRUE;
		}
		if (LOWORD(wParam) == 4)
		{
			fszog_y2 = HIWORD(wParam);
			return (INT_PTR)TRUE;
		}
		if (LOWORD(wParam) == 6)
		{
			fszog_z2++;
			return (INT_PTR)TRUE;
		}
		if (LOWORD(wParam) == 7)
		{
			fszog_z2--;
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
LRESULT CALLBACK WndProc0(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	MSG msg;
	HDC hdc_lokal;
	PAINTSTRUCT ps_lokal;
	unsigned int xPos, yPos, xPos2, yPos2, fwButtons;

	switch (message) {
	case WM_CREATE:
		//************************
		//kezdõ beállítások és a gamepad figyelésének aktiválása
		//************************
		init();
		if ((joyGetNumDevs()) > 0) joySetCapture(hwnd, JOYSTICKID1, NULL, FALSE);
		break;
	case WM_ERASEBKGND:
		return (LRESULT)1;
	case MM_JOY1MOVE:
		//************************
		//gamepad mûveletek feldolgozása
		//************************
		fwButtons = wParam;
		xPos = LOWORD(lParam);
		yPos = HIWORD(lParam);
		if (joystick_mode == 0)
		{
			if (xPos == 65535) fszog_y2 += 5.0;
			else if (xPos == 0) fszog_y2 -= 5.0;
			if (yPos == 65535) fszog_x2 += 5.0;
			else if (yPos == 0) fszog_x2 -= 5.0;
			if (fwButtons == 128) fszog_z2 += 5.0;
			else if (fwButtons == 64) fszog_z2 -= 5.0;
			if (fszog_y2 > 360) fszog_y2 = 0;
			else if (fszog_y2 < 0) fszog_y2 = 355;
			if (fszog_x2 > 360) fszog_x2 = 0;
			else if (fszog_x2 < 0) fszog_x2 = 355;
			if (fszog_z2 > 360) fszog_z2 = 0;
			else if (fszog_z2 < 0) fszog_z2 = 355;
		}

		if (fwButtons == 128) fszog_z2 += 5.0;
		else if (fwButtons == 64) fszog_z2 -= 5.0;
		else if (fwButtons == 2) { nagyitas(); }
		else if (fwButtons == 4) { kicsinyites(); }
		else if (fwButtons == 1) { nezopont += 500; }
		else if (fwButtons == 8) { nezopont -= 500; }
		else if (fwButtons == 16) { ; }
		else if (fwButtons == 32) { ; }
		kep_frissit();
		break;
	case WM_PAINT:
		hdc_lokal = BeginPaint(hwnd, &ps_lokal);
		EndPaint(hwnd, &ps_lokal);
		kep_frissit();
		break;
	case WM_RBUTTONUP:
		kicsinyites();
		break;
	case WM_LBUTTONUP:
		nagyitas();
		break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(540);
		break;
	case IDM_ABOUT:
		DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, About);
		break;
	case WM_COMMAND:
		int wmId = LOWORD(wParam);
		switch (wmId)
		{
		case IDM_FORGATAS:
			DialogBox(hInst, MAKEINTRESOURCE(101), hwnd, fSzog);
			break;
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, About);
			break;
		default:
			return DefWindowProc(hwnd, message, wParam, lParam);
		}
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}
void init(void)
{
	persp_szog = Math_PI / 180;
	fszog_x = 0 * Math_PI / 180; fszog_x2 = 150;
	fszog_y = 0 * Math_PI / 180; fszog_y2 = 150;
	fszog_z = 0 * Math_PI / 180; fszog_z2 = 0;
	data_load();
}
void data_load(void)
{
	if (rajzolas_folyamatban == 1) return;
	cleanup_matrices();
	obj_loader();
	trans_matrix_create();
}
void trans_matrix_create(void)
{
	int i;

	forgatott_csucspontok_length = 0;
	for (i = 0; i < nyers_csucspontok_length; ++i)
	{
		forgatott_csucspontok[i][0] = nyers_csucspontok[i][0];
		forgatott_csucspontok[i][1] = nyers_csucspontok[i][1];
		forgatott_csucspontok[i][2] = nyers_csucspontok[i][2];
		++forgatott_csucspontok_length;
	}
}
void cleanup_matrices(void)
{
	nyers_csucspontok_offset_length = 0;
	global_offset = 0;
	nyers_csucspontok_szinek_length = nyers_csucspontok_tipus_length = nyers_csucspontok_index_length = beolv_pontok_length = nyers_csucspontok_length = beolvpont_index_length = raw_vertex_counter = 0;
}
void kep_frissit(void)
{
	if (rajzolas_folyamatban == 1) return;
	rajzolas_folyamatban = 1;
	char tempstr[255], tempstr2[255];
	strcpy(tempstr2, "Vertices: ");
	itoa(szamlalo_vert, tempstr, 10); strcat(tempstr2, tempstr); strcat(tempstr2, " Poly-gons: ");
	itoa(szamlalo_poly, tempstr, 10); strcat(tempstr2, tempstr); strcat(tempstr2, " Z ordered: ");
	itoa(szamlalo_zorder, tempstr, 10); strcat(tempstr2, tempstr);

	kezdet = GetTickCount();

	fszog_x = fszog_x2 * Math_PI / 180;
	fszog_y = fszog_y2 * Math_PI / 180;
	fszog_z = fszog_z2 * Math_PI / 180;
	forgatas();

	parhuzamos_projekcio();
	perspektivikus_projekcio();
	rajzolas();

	vege = GetTickCount();
	if ((vege - kezdet) == 0) ++vege;
	fps_stat = 1000 / (vege - kezdet); strcat(tempstr2, " FPS: "); itoa(fps_stat, tempstr, 10); strcat(tempstr2, tempstr);
	SetWindowTextA(Form1, tempstr2);
	rajzolas_folyamatban = 0;
}
void perspektivikus_projekcio(void)
{
	int i;
	float sx = stage_stageWidth / 2;
	float sy = stage_stageHeight / 2;
	float sultra = stage_stageHeight / 2;

	lekepezett_csucspontok_length = 0;

	for (i = 0; i < nyers_csucspontok_length; ++i)
	{
		sultra = nezopont / (nezopont - forgatott_csucspontok[i][2]);
		lekepezett_csucspontok[i][0] = (forgatott_csucspontok[i][0] * (sultra)) + sx;
		lekepezett_csucspontok[i][1] = (forgatott_csucspontok[i][1] * (sultra)) + stage_stageHeight / 3;
		lekepezett_csucspontok[i][2] = forgatott_csucspontok[i][2];
		++lekepezett_csucspontok_length;
	}
}
void parhuzamos_projekcio(void)
{
	int i;
	float dx;
	float dy;

	lekepezett_csucspontok_length = 0;
	for (i = 0; i < nyers_csucspontok_length; ++i)
	{
		dx = forgatott_csucspontok[i][0] * cos(persp_szog) + forgatott_csucspontok[i][1] * cos(persp_szog);
		dy = forgatott_csucspontok[i][0] * sin(persp_szog) - forgatott_csucspontok[i][1] * sin(persp_szog) - forgatott_csucspontok[i][2];
		lekepezett_csucspontok[i][0] = dx + stage_stageWidth / 3;
		lekepezett_csucspontok[i][1] = dy + stage_stageHeight / 2;
		lekepezett_csucspontok[i][2] = forgatott_csucspontok[i][2];
	}
}
void rajzolas(void)
{
	DB_start_drawing();
	kepernyo_clear();

	z_ordering();

	draw_3d_objects(1);//epuletek

	DB_end_drawing();
}
void kepernyo_clear(void)
{
	HBRUSH hBrush;
	RECT rect;
	HDC hdc;
	hdc = hdcMem;
	hBrush = CreateSolidBrush(RGB(255, 255, 255));
	SelectObject(hdc, hBrush);
	SetRect(&rect, 0, 0, 1500, 900);
	FillRect(hdc, &rect, hBrush);
	DeleteObject(hBrush);
}
void z_ordering(void)
{
	int i;
	int j = 0;
	int s, x_minusz_hatar = -100, y_minusz_hatar = -100, x_max_hatar = stage_stageWidth + 100, y_max_hatar = stage_stageHeight + 100;
	int kilog;
	float tavolsag;

	szamlalo_zorder = 0;
	zorder_length = 0;
	for (i = 0; i < nyers_csucspontok_index_length; ++i)
	{
		tavolsag = 999999;
		kilog = 0;
		for (s = 0; s < nyers_csucspontok_index[i]; j = nyers_csucspontok_offset[i] + s, ++s)
		{
			if (lekepezett_csucspontok[j][0] < x_minusz_hatar || lekepezett_csucspontok[j][0] > x_max_hatar) kilog = 1;
			if (lekepezett_csucspontok[j][1] < y_minusz_hatar || lekepezett_csucspontok[j][1] > y_max_hatar) kilog = 1;
			if (lekepezett_csucspontok[j][2] < tavolsag)
			{
				tavolsag = lekepezett_csucspontok[j][2];
			}
		}
		if (kilog != 0) continue;//levagas, grafikai hiba elkerulese
		if (tavolsag < nezopont) continue;//levagas, grafikai hiba elkerulese

		++szamlalo_zorder;
		zorder[zorder_length][0] = tavolsag;
		zorder[zorder_length][1] = i;
		++zorder_length;
	}
	rendezes_shell();
}
void rendezes_shell(void)
{
	int i;
	int j;
	int t;
	int k;
	float csere0;
	float csere1;

	for (k = zorder_length / 2; k > 0; k = k / 2)
		for (i = k; i < zorder_length; ++i)
			for (j = i - k; (j >= 0) && (zorder[j][0] > zorder[j + k][0]); j = j - k)
			{
				csere0 = zorder[j][0];
				csere1 = zorder[j][1];
				zorder[j][0] = zorder[j + k][0];
				zorder[j][1] = zorder[j + k][1];
				zorder[j + k][0] = csere0;
				zorder[j + k][1] = csere1;
			}
}
void forgatas()
{
	int i;
	float szog_sin = sin(fszog_x);
	float szog_cos = cos(fszog_x);
	float t0, t2, t1;

	for (i = 0; i < nyers_csucspontok_length; ++i)
	{
		forgatott_csucspontok[i][0] = nyers_csucspontok[i][0];
		forgatott_csucspontok[i][1] = (nyers_csucspontok[i][1] * szog_cos) - (nyers_csucspontok[i][2] * szog_sin);
		forgatott_csucspontok[i][2] = nyers_csucspontok[i][1] * szog_sin + nyers_csucspontok[i][2] * szog_cos;
	}

	szog_sin = sin(fszog_y);
	szog_cos = cos(fszog_y);

	for (i = 0; i < forgatott_csucspontok_length; ++i)
	{
		t0 = forgatott_csucspontok[i][0];
		t2 = forgatott_csucspontok[i][2];
		forgatott_csucspontok[i][0] = t0 * szog_cos + t2 * szog_sin;
		forgatott_csucspontok[i][2] = t0 * szog_sin - t2 * szog_cos;
	}

	szog_sin = sin(fszog_z);
	szog_cos = cos(fszog_z);

	for (i = 0; i < forgatott_csucspontok_length; ++i)
	{
		t0 = forgatott_csucspontok[i][0];
		t1 = forgatott_csucspontok[i][1];
		forgatott_csucspontok[i][0] = t0 * szog_cos - t1 * szog_sin;
		forgatott_csucspontok[i][1] = t0 * szog_sin + t1 * szog_cos;
	}
}
void draw_3d_objects(int tipus)
{
	int sorszam;
	int i;
	int j;
	int k;
	int akt_index;

	HBRUSH hBrush;
	RECT rect;
	HDC hdc;
	PAINTSTRUCT  ps;
	HPEN hPen1;
	POINT vektor_parancsok[60000];
	int vektor_poz = 0;

	hdc = hdcMem;
	hPen1 = CreatePen(PS_SOLID, 1, RGB(155, 155, 155));
	hBrush = CreateSolidBrush(RGB(0, 255, 255));

	SelectObject(hdc, hPen1);
	SelectObject(hdc, hBrush);

	for (i = zorder_length - 1; i > -1; --i)
	{
		sorszam = zorder[i][1];//poligon sorszama
		akt_index = nyers_csucspontok_offset[sorszam];
		j = akt_index;

		if (tipus == 1 && nyers_csucspontok_tipus[sorszam] == tipus)
		{
			vektor_poz = 0;

			for (j = akt_index, k = 0; k < nyers_csucspontok_index[sorszam]; ++j, ++k)
			{
				vektor_parancsok[vektor_poz].x = lekepezett_csucspontok[j][0];
				vektor_parancsok[vektor_poz++].y = lekepezett_csucspontok[j][1];
			}
			Polygon(hdc, vektor_parancsok, vektor_poz);
		}
	}

	DeleteObject(hBrush);
	DeleteObject(hPen1);
}
int getelementcount(unsigned char csv_szoveg[])
{
	int s1, s2;
	for (s1 = s2 = 0; s1 < strlen((const char*)csv_szoveg); ++s1)
	{
		if (csv_szoveg[s1] == 10) break;
		else if (csv_szoveg[s1] == 32) ++s2;
	}
	return s2;
}
void getelement(unsigned char csv_szoveg[], unsigned int hanyadik, unsigned char csv_szoveg2[])
{
	int s1, s2, s3, s4 = 0;
	for (s1 = 0, s2 = 0; s1 < strlen((const char*)csv_szoveg); ++s1)
	{
		if (csv_szoveg[s1] == 32)
		{
			++s2;
			if (s2 == hanyadik)
			{
				for (s3 = s1 + 1; s3 < strlen((const char*)csv_szoveg); ++s3)
				{
					if (csv_szoveg[s3] == 32 || csv_szoveg[s3] == 10)
					{
						csv_szoveg2[s4] = 0;
						return;
					}
					else csv_szoveg2[s4++] = csv_szoveg[s3];
				}
			}
		}
	}
}
void obj_loader(void)
{
	int i, j;
	float adat1, adat2, adat3, adat4;
	fajl = fopen(str, "rt"); if (fajl == NULL) { printf("file io error"); return; }

	szamlalo_vert = szamlalo_poly = 0;

	while (!feof(fajl))
	{
		fgets((char*)sor1, maxsorszelesseg, fajl);

		if (sor1[0] == 118 && sor1[1] == 32) //*** 'v '
		{
			getelement(sor1, 1, sor2); adat1 = atof((const char*)sor2);
			getelement(sor1, 2, sor2); adat2 = atof((const char*)sor2);
			getelement(sor1, 3, sor2); adat3 = atof((const char*)sor2);
			tomb_vertices[tomb_vertices_length][0] = adat1;
			tomb_vertices[tomb_vertices_length][1] = adat2;
			tomb_vertices[tomb_vertices_length++][2] = adat3;
		}
		else if (sor1[0] == 102 && sor1[1] == 32) //*** 'f '
		{
			elemszam = getelementcount(sor1);
			++szamlalo_poly;

			tomb_faces[tomb_faces_length][0] = elemszam;
			for (i = 1; i < elemszam + 1; ++i)
			{
				getelement(sor1, i, sor2); adat1 = atof((const char*)sor2);
				tomb_faces[tomb_faces_length][i] = adat1 - 1;
			}
			++tomb_faces_length;
		}
	}
	fclose(fajl);

	for (i = 0; i < tomb_faces_length; ++i)
	{
		for (j = 1; j < tomb_faces[i][0] + 1; ++j)
		{
			nyers_csucspontok[nyers_csucspontok_length][0] = tomb_vertices[tomb_faces[i][j]][0];
			nyers_csucspontok[nyers_csucspontok_length][1] = tomb_vertices[tomb_faces[i][j]][1];
			nyers_csucspontok[nyers_csucspontok_length++][2] = tomb_vertices[tomb_faces[i][j]][2];
		}
		nyers_csucspontok_index[nyers_csucspontok_index_length++] = tomb_faces[i][0];
		nyers_csucspontok_tipus[nyers_csucspontok_tipus_length++] = 1;
		nyers_csucspontok_offset[nyers_csucspontok_offset_length++] = global_offset;
		global_offset += tomb_faces[i][0];
		nyers_csucspontok_szinek[nyers_csucspontok_szinek_length++] = RGB(200, 222, 200);
		szamlalo_vert += tomb_faces[i][0];
	}
}
void nagyitas(void)
{
	int i;

	for (i = 0; i < nyers_csucspontok_length; ++i)
	{
		nyers_csucspontok[i][0] *= 2;
		nyers_csucspontok[i][1] *= 2;
		nyers_csucspontok[i][2] *= 2;
	}
	kep_frissit();
}
void kicsinyites(void)
{
	int i;

	for (i = 0; i < nyers_csucspontok_length; ++i)
	{
		nyers_csucspontok[i][0] /= 2;
		nyers_csucspontok[i][1] /= 2;
		nyers_csucspontok[i][2] /= 2;
	}
	kep_frissit();
}
void DB_prepare_puffer(void)
{
	GetClientRect(Form1, &rc);
	hdc = GetDC(Form1);
	hbmMem = CreateCompatibleBitmap(hdc, rc.right - rc.left, rc.bottom - rc.top);
}
void DB_start_drawing(void)
{
	GetClientRect(Form1, &rc);
	hdc = GetDC(Form1);
	BeginPaint(Form1, &ps);
	hdcMem = CreateCompatibleDC(hdc);
	hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);
	hbrBkGnd = CreateSolidBrush(RGB(255, 255, 255));
	FillRect(hdcMem, &rc, hbrBkGnd);
	DeleteObject(hbrBkGnd);
}
void DB_end_drawing(void)
{
	BitBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hdcMem, 0, 0, SRCCOPY);
	SelectObject(hdcMem, hbmOld);
	DeleteDC(hdcMem);
	EndPaint(Form1, &ps);
}