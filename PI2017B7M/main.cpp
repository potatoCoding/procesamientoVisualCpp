#include <Windows.h>
#include <commdlg.h>
#include "IPImage.h"
#include "Matrix3D.h"

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
				if (pInput) {
					g_M = Transpose(						
						
						Translate(-mx, -my)*
						Scaling(sx, sy)*
						Translate(mx, my)*
						Translate(-px, -py)*
						Translate((float)-(pInput->getSizeX() / 2), (float)-(pInput->getSizeY() / 2))*
						Transpose(Rotation(-p))*
						Translate((float)(pInput->getSizeX() / 2), (float)(pInput->getSizeY() / 2)));
					pOutput->process(InverseMapping, &pInput, 1);	
				}
				//pOutput->process(Negative, &pInput, 1);
				pOutput->draw(hdc, 0, 0);
				CIPImage::destroyImage(pOutput);
				EndPaint(hWnd, &ps);
			}
			return 0;
			break;
		case WM_CREATE:
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
	return RegisterClassEx(&wnc);
}
//3- creacion y muestra de la ventana
HWND crearVentana(HINSTANCE hInstance, int nCmdShow) {
	HWND hWnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW,L"Mi Ventana",L"Mi primer ventana en C++",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,0,0,hInstance,0);
	ShowWindow(hWnd, nCmdShow);
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

	RegistrarClaseVentana(hInstance);
	crearVentana(hInstance, nCmdShow);
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