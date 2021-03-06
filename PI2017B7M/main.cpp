#include <Windows.h>
#include <commdlg.h>
#include "IPImage.h"
#include "Matrix3D.h"
#include "..\AtWareVC32Lib\AtWareVideoCapture.h"
#include "VideoProcessor.h"
IAtWareVideoCapture* g_pVC;
HWND g_hVideoWnd;
CVideoProcessor g_VP;
int edge = 1;
int edge_offset = 0;
/* Soft pixel shader c++ */
CIPImage::PIXEL Shader(int i, int j, CIPImage* Inputs[], int nArgs) {
	CIPImage::PIXEL Color = { 20,20,80,0 };
	return Color;
}

CIPImage::PIXEL Negative(int i, int j, CIPImage* Inputs[], int nArgs) {
	auto Color = (*Inputs)[0](i, j);
	CIPImage::PIXEL Dest = { (unsigned char)~Color.b, (unsigned char)~Color.g, (unsigned char)~Color.r, 0 };
	return Dest;
}

CIPImage::PIXEL HorizontalDerivate(int i, int j, CIPImage* Inputs[], int nArgs) {
	CIPImage::PIXEL Color, A, B;
	A = (*Inputs[0])((unsigned char)i - 1, (unsigned char)j);
	B = (*Inputs[0])((unsigned char)i + 1, (unsigned char)j);
	Color.r = max(min(255, 127 + (int)A.r - B.r), 0);
	Color.g = max(min(255, 127 + (int)A.g - B.g), 0);
	Color.b = max(min(255, 127 + (int)A.b - B.b), 0);
	return Color;
}

Matrix3D g_M = zero();
CIPImage::PIXEL InverseMapping(int i, int j, CIPImage* pInputs[], int nImputs) {
	vector3D source = { (float)i,(float)j,1,0 };
	vector3D dest = source * g_M;
	return pInputs[0]->sample(dest.x, dest.y);
}

int GetTrunkedColor(int color) {

	if (color>250)color = 250;
	else if (color>200)color = 200;
	else if (color>150)color = 150;
	else if (color>100)color = 100;
	else if (color>50)color = 50;
	else color = 0;

	return color;
}

float Kernel3x3[3][3] = { 
	/*{0,0.25,0}, 
	{-0.25,0,0.25},
	{0,-0.25,0} */
#define USE_A 1
#ifdef USE_A
	{ -1,-2,-1 },
	{ -1,0,1 },
	{ 1,2,1 }
#else
	{-1,-2,-1 },
	{0,0,0},
	{1,2,1}
#endif
};
float C = 127;

vector3D Mul(CIPImage::PIXEL p, float s) {
	return { p.b*s, p.g*s, p.r*s, 0 };
}

CIPImage::PIXEL Convole3x3(int i, int j, CIPImage* pInputs[], int nImputs) {
	vector3D S = { 0,0,0,0 };
	for (int y = -1; y < 2; y++)
	{
		for (int x = -1; x < 2; x++) 
		{
			vector3D C = Mul((*pInputs[0])(i+x, j+y), Kernel3x3[y+1][x+1]);
			S.x += C.x;
			S.y += C.y;
			S.z += C.z;
		}
	}
	CIPImage::PIXEL R;
	R.b = (char)(max(0, min(255, S.x+C)));
	R.g = (char)(max(0, min(255, S.y+C)));
	R.r = (char)(max(0, min(255, S.z+C)));
	R.a = 0xff;
	return R;
}

CIPImage::PIXEL Toon(int i, int j, CIPImage* pInputs[], int nImputs) {
	CIPImage::PIXEL Color = (*pInputs[0])(i, j);
	vector3D L = { 0,0,0,0 };
	vector3D N = { 0,0,0,0 };
	for (int y = -1; y < 2; y++)
	{
		for (int x = -1; x < 2; x++)
		{
			vector3D C = Mul((*pInputs[0])(i + x, j + y), Kernel3x3[y + 1][x + 1]);
			L.x += C.x;
			L.y += C.y;
			L.z += C.z;
			vector3D C1 = Mul((*pInputs[0])(i + x, j + y), 1);
			N.x += C1.x;
			N.y += C1.y;
			N.z += C1.z;
		}
	}
	CIPImage::PIXEL R;
	//float intensity = 1/((N.x*L.x)+(N.y*L.y)+(L.z*N.z));
	//float intensity = Dot((vector3D&)N, (vector3D&)L);
#define USE_B 1
#ifdef USE_B
	
	R.b = (char)(max(0, min(255, L.x + C)));
	R.g = (char)(max(0, min(255, L.y + C)));
	R.r = (char)(max(0, min(255, L.z + C)));
	R.a = 0xff;
	int prom = (R.b + R.g + R.r) / 3;
	if (prom > edge) {
		if (prom < edge + edge_offset) {
			R.b = (char)(max(0, min(255, L.x - N.x + C)));
			R.g = (char)(max(0, min(255, L.y - N.y + C)));
			R.r = (char)(max(0, min(255, L.z - N.z + C)));
			R.b = GetTrunkedColor((int)R.b);
			R.g = GetTrunkedColor((int)R.g);
			R.r = GetTrunkedColor((int)R.r);
		}
		else if (prom < edge + edge_offset +1) {
			R.b = Color.b;
			R.g = Color.g;
			R.r = Color.r;
		}
		else
		{
			R.b = GetTrunkedColor((int)R.b);
			R.g = GetTrunkedColor((int)R.g);
			R.r = GetTrunkedColor((int)R.r);

		}
	}
	else
	{
		R.b = Color.b;
		R.g = Color.g;
		R.r = Color.r;
	}
#else
	R.b = (char)(max(0, min(255, L.x - N.x + C)));
	R.g = (char)(max(0, min(255, L.y - N.y + C)));
	R.r = (char)(max(0, min(255, L.z - N.z + C)));
	R.a = 0xff;
	if ((R.b + R.g + R.r) / 3 > edge) {
		R.b = GetTrunkedColor((int)Color.b);
		R.g = GetTrunkedColor((int)Color.g);
		R.r = GetTrunkedColor((int)Color.r);
	}
	else
	{
		R.b = Color.b;
		R.g = Color.g;
		R.r = Color.r;
	}	
#endif
	return R;
}

//Procedimiento de ventana adicional para el procedimiento de vista previa(Web Cam)
LRESULT WINAPI videoProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg)
	{
	case WM_CREATE:
		return 0;
		break;
	case WM_SIZE:
		if (g_pVC) {
			RECT rc;
			GetClientRect(hWnd, &rc);
			g_pVC->SetPreviewWindowPosition(&rc);
		}
		break;
	case WM_CLOSE:
		return 0;
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

//1- procedimiento ventana: tiene como objetivo procesar todos los eventos que el usuao y sistema generen.
//La iplementacion de estas repuesas define el comportamiento de la aplicacion
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static CIPImage* pInput;
	static float p = 0;
	static float px = 0, py = 0;
	static float mx = 0, my = 0;
	static float sx = 1.0f, sy = 1.0f;
	static float lastx, lasty;
	static bool bDragging = false;
	//despachar los diferentes mensajes de la ventana
	switch (msg)
	{
	case WM_TIMER:
		switch (wParam)
		{
			case 1:
				InvalidateRect(hWnd, NULL, 0);
			break;
		}
		break;
	case WM_LBUTTONDOWN:
		lastx = LOWORD(lParam);
		lasty = HIWORD(lParam);
			mx = LOWORD(lParam);
			my = HIWORD(lParam);
		bDragging = true;
 		break;
	case WM_MOUSEMOVE:
			mx = LOWORD(lParam);
			my = HIWORD(lParam);
		if (bDragging) {
			int x = LOWORD(lParam), y = HIWORD(lParam);
			int dx = x - lastx;
			int dy = y - lasty;
			px += dx;
			py += dy;
			lastx = x;
			lasty = y;
			InvalidateRect(hWnd, NULL, false);
		}
		break;
	case WM_LBUTTONUP:
			bDragging = false;
			break;
	case WM_KEYDOWN:
		//switch tecla pulsada
		switch (wParam)
		{
			case 'Q': p += 0.01f; InvalidateRect(hWnd, 0, 0); break;
			case 'E': p -= 0.01f; InvalidateRect(hWnd, 0, 0); break;
			case 'Z': sx += 0.01f; sy += 0.01f; InvalidateRect(hWnd, 0, 0); break;
			case 'X': sx -= 0.01f; sy -= 0.01f; InvalidateRect(hWnd, 0, 0); break;
			case 'C':
				if (pInput)
					CIPImage::destroyImage(pInput);
				pInput = CIPImage::CaptureDesktop();
				InvalidateRect(hWnd, 0, 0);//Repintar
			break;
			case 'V':
				if (g_pVC->EnumAndChooseCaptureDevice()) {
					AM_MEDIA_TYPE MT;
					memset(&MT, 0, sizeof(MT));
					MT.formattype = GUID_NULL;
					MT.majortype = MEDIATYPE_Video;
					MT.subtype = MEDIASUBTYPE_RGB32;
					g_pVC->SetMediaType(&MT);
					g_pVC->BuildStreamGraph();
					g_pVC->ShowPreviewWindow(true);
					g_pVC->GetMediaType(&g_VP.m_MT);
					g_pVC->SetCallBack(&g_VP,1);
					g_pVC->Start();
					RECT rc;
					GetClientRect(g_hVideoWnd, &rc);
					g_pVC->SetPreviewWindowPosition(&rc);
				}			
				break;
			case 'F':
			{
				OPENFILENAMEA Ofn;
				memset(&Ofn, 0, sizeof(Ofn));
				char szFileName[1024] = "";
				Ofn.lStructSize = sizeof(OPENFILENAME);
				Ofn.hwndOwner = hWnd;
				Ofn.lpstrFile = szFileName;
				Ofn.lpstrFilter = "Archivos DIB (*.bmp)\0*.bmp\0";
				Ofn.nFilterIndex = 0;
				Ofn.nMaxFile = 1023;
				if (GetOpenFileNameA(&Ofn)) {
					if (pInput) CIPImage::destroyImage(pInput);
					pInput = CIPImage::loadFromFile(szFileName);
					InvalidateRect(hWnd, 0, 0);//Repintar
				}
			}
				break;
			case 'S':
				CIPImage::saveToFile("C:\\Users\\Carlos\\Desktop\\imagen_Guardada.bmp", pInput);
				break;
			case'M':
				edge+=1;
			break;
			case 'N':
				edge-=1;
				break;
			case'P':
				edge_offset += 2;
				break;
			case 'O':
				edge_offset -= 2;
				break;;
		}
		break;
	case WM_PAINT: 
			{
#pragma region old code
				/*
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);
				int x = 1366;
				int y = 768;
				CIPImage* pImage = CIPImage::createImage(x, y);
				for (int i = 0; i < x; i++)
				{
					for (int j = 0; j < y; j++)
					{
						//(*pImage)(i, j).n = 0x000000ff;
						//estatica
						//(*pImage)(i, j).n = ((i*j)*(i*j))*rand() & 0xff | (0 & 0xff)*rand() << 8 | ((i*i + j*j)*rand() & 0xff) << 16;
						(*pImage)(i, j).n = ((i*j)*(i*j)) & 0xff | (0 & 0xff) << 8 | ((i*i + j*j) & 0xff) << 16;
					}
				}
				pImage->draw(hdc, 0, 0);
				CIPImage::destroyImage(pImage);
				EndPaint(hWnd, &ps);
				//InvalidateRect(hWnd, 0, 0);//Repintar
				*/
#pragma endregion
				//Cargar Nomral
				/*PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);
				if (pInput)
					pInput->draw(hdc, 0, 0);
				EndPaint(hWnd, &ps);*/
			
				//cargar con shader
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);
				RECT rc;
				GetClientRect(hWnd, &rc);
				CIPImage* pOutput = CIPImage::createImage(max(10, rc.right), max(10, rc.bottom));				
				pInput = g_VP.Pull();
				if (pInput) {
					g_M = Transpose(						
						
						Translate(-mx, -my)*
						Scaling(sx, sy)*
						Translate(mx, my)*
						Translate(-px, -py)*
						Translate((float)-(pInput->getSizeX() / 2), (float)-(pInput->getSizeY() / 2))*
						Transpose(Rotation(-p))*
						Translate((float)(pInput->getSizeX() / 2), (float)(pInput->getSizeY() / 2)));
					//pOutput->process(Negative, &pInput, 1);
					//pOutput->process(Convole3x3, &pInput, 1);	
					pOutput->process(Toon, &pInput, 1);
					CIPImage::destroyImage(pInput);
					pOutput->draw(hdc, 0, 0);
				}
				CIPImage::destroyImage(pOutput);
				EndPaint(hWnd, &ps);
			}
			return 0;
			break;
		case WM_CREATE:
			SetTimer(hWnd, 1, 20, NULL);
			return 0;
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
			break;
		case WM_CLOSE:
			if (IDYES == MessageBox(hWnd, L"¿Desea Salir?", L"Salir", MB_ICONQUESTION | MB_YESNO))
				DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
			break;
	}
}
/*2- Registro de clase de ventana:
tiene como objetivo informar a Windows cual es el procedimiento que utilizare para contestar a los eventos de mis ventanas. Asi el OS
podra invocar a WndProc tantas veces como eventos se despachen*/
ATOM RegistrarClaseVentana(HINSTANCE hInstance) {
	WNDCLASSEX wnc;
	memset(&wnc, 0, sizeof(wnc));
	wnc.cbSize = sizeof(WNDCLASSEX);//????
	wnc.hInstance = hInstance;
	wnc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wnc.lpfnWndProc = WndProc;
	wnc.lpszClassName = L"Mi Ventana";
	RegisterClassEx(&wnc);
	wnc.lpfnWndProc = videoProc;
	wnc.lpszClassName = L"VideoVentana";
	return RegisterClassEx(&wnc);
}
//3- creacion y muestra de la ventana
HWND crearVentana(HINSTANCE hInstance, int nCmdShow) {
	HWND hWnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW,L"Mi Ventana",L"Image Processor",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,0,0,hInstance,0);
	ShowWindow(hWnd, nCmdShow);
	hWnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, L"VideoVentana", L"Vista Previa", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, hInstance, 0);
	ShowWindow(hWnd, nCmdShow);
	g_hVideoWnd = hWnd;
	return hWnd;
}

/*4- se le delega a win main la responsabilidad de:
registar clase ventana
*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpzCmdLine, int nCmdShow) {
#pragma region Old Code
	/*CIPImage I;
	I(300, 200).n = 90;
	int res = MessageBox(NULL, L"Hola Mundo", L"Mi APP W32 en C++", MB_OKCANCEL);
	switch (res)
	{
		case IDOK:
			MessageBox(NULL, L"OK", L"OK", MB_OK|MB_ICONINFORMATION);
			break;
		case IDCANCEL:
			MessageBox(NULL, L"Cancel", L"Cancel", MB_OK|MB_ICONERROR);
		break;
		default:
			break;
	}*/
	/*HDC hdcDesktop = CreateDC(L"DISPLAY", 0, 0, 0);
	int x = 1366;
	int y = 768;
	CIPImage* pImage = CIPImage::createImage(x, y);
	for (int i = 0; i < x; i++)
	{
		for (int j = 0; j < y; j++)
		{
			//(*pImage)(i, j).n = 0x000000ff;
			(*pImage)(i, j).n = ((i*j)*(i*j))&0xff | (0&0xff)<<8 | ((i*i+j*j)&0xff)<<16;
		}
	}
	int i = 10;
	while (i > 0)
	{
		pImage->draw(hdcDesktop, 0, 0);
		Sleep(1000);
		i--;
	}
	CIPImage::destroyImage(pImage);*/
#pragma endregion
	vector3D V = { 1,2,3,0 };//w debe ser siempre 0
	Matrix3D I = Transpose(Identity());
	vector3D P = V*I;

	printf("");
	wprintf(L"");
	RegistrarClaseVentana(hInstance);
	HWND hWnd = crearVentana(hInstance, nCmdShow);
	g_pVC = CreateAtWareVideoCapture();
	g_pVC->Initialize(hWnd);
	/*bucle de mensajes = bomba de mensajes
	recibir y confirmar de recibido todos los mensajes que llegen a este proceso. 
	El bucle de mensaje, lee cada mensaje y decide si despacha o no mensaje al procedimiento venatana correspondiente*/
	MSG msg;
	while (GetMessage(&msg,0,0,0))
	{
		DispatchMessage(&msg);
	}
	
	return NULL;
}