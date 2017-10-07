#include "IPImage.h"



CIPImage::PIXEL & CIPImage::operator()(int i, int j)
{
	static PIXEL dummy;
	if (i >= 0 && i < m_nSizex && j >= 0 && j < m_nSizey)
		return *(m_pBuffer + j*m_nSizex + i);
	else
		return dummy;
}

CIPImage::PIXEL Lerp(CIPImage::PIXEL& A, CIPImage::PIXEL& B, float u) {
	
	CIPImage::PIXEL C;// P=A+u*(B-A)
	C.r = (unsigned char)(A.r + u*((int)B.r - A.r));
	C.g = (unsigned char)(A.g + u*((int)B.g - A.g));
	C.b = (unsigned char)(A.b + u*((int)B.b - A.b));
	C.a = 0xff;
	return C;
}

CIPImage::PIXEL CIPImage::sample(float i, float j)
{
	int x = (int)i, y = (int)j;
	float p = i - x, q = j - y;
	PIXEL A, B , C, D;
	int a = max(0, min(m_nSizex - 1, x));
	int b = max(0, min(m_nSizex - 1, x + 1));
	int c = max(0, min(m_nSizey - 1, y));
	int d = max(0, min(m_nSizey - 1, y + 1));
	A = (*this)(a, c);
	B = (*this)(b, c);
	C = (*this)(a, d);
	D = (*this)(b, d);
	
	/*A = (*this)(x, y);
	B = (*this)(x+1, y);
	C = (*this)(x, y+1);
	D = (*this)(x+1, y+1);*/

	return Lerp(Lerp(A, B, p), Lerp(C, D, p), q);
}

CIPImage * CIPImage::createImage(int sx, int sy)
{
	CIPImage* pImage = new CIPImage();
	pImage->m_pBuffer = new PIXEL[sx*sy];
	if (!pImage->m_pBuffer) {
		delete pImage;
		return nullptr;
	}
	pImage->m_nSizex = sx;
	pImage->m_nSizey = sy;
	return pImage;
}

void CIPImage::destroyImage(CIPImage * pToDestroy)
{
	delete[] pToDestroy->m_pBuffer;
	delete pToDestroy;
}

CIPImage * CIPImage::CaptureDesktop()
{
	HDC hdc = CreateDC(L"DISPLAY", 0, 0, 0);
	int sx = GetDeviceCaps(hdc, VERTRES);
	int sy = GetDeviceCaps(hdc, VERTRES);
	CIPImage* pImage = createImage(sx, sy);
	HDC hdcMem = CreateCompatibleDC(hdc);
	HBITMAP hbmpMem = CreateCompatibleBitmap(hdc, sx, sy);
	SelectObject(hdcMem, hbmpMem);
	BitBlt(hdcMem, 0, 0, sx, sy, hdc, 0, 0, SRCCOPY);
	GetBitmapBits(hbmpMem, sx*sy * sizeof(PIXEL), pImage->m_pBuffer);
	DeleteDC(hdc);
	DeleteDC(hdcMem);
	DeleteObject(hbmpMem);
	return pImage;
}

#include <fstream>
using namespace std;
CIPImage * CIPImage::loadFromFile(char * pszFileName)
{
	fstream in;
	in.open(pszFileName, ios::in | ios::binary);
	if (!in.is_open())
		return nullptr;
	BITMAPFILEHEADER bfh;
	BITMAPINFOHEADER bih;
	memset(&bfh, 0, sizeof(bfh));
	memset(&bih, 0, sizeof(bih));
	in.read((char*)&bfh, sizeof(bfh));
	if (bfh.bfType != 'MB')
		return nullptr;
	in.read((char*)&bih, sizeof(bih));
	if (bih.biSize != sizeof(bih))
		return nullptr;	
	/*
	switch (bih.biSize)
	{
	case sizeof(BITMAPINFOHEADER) :
		break;
		case sizeof()
	default:
		break;
	}
	*/
	CIPImage* pImage = CIPImage::createImage(bih.biWidth, bih.biHeight);
	int rowLength = 4 * ((bih.biWidth*bih.biBitCount + 31) / 32);
	switch (bih.biBitCount)
	{
	case 1://1bpp tarea (paletizado)
		{
			RGBQUAD Paleta[2];
			int nColors = bih.biClrUsed ? bih.biClrUsed : 2;
			in.read((char*)&Paleta, nColors * sizeof(RGBQUAD));
			unsigned char* pLine = new unsigned char[rowLength];
			for (int j = bih.biHeight - 1; j >= 0; j--) {
				in.read((char*)pLine, rowLength);
				int c = 0;
				for (int i = 0; i < rowLength; i++) {
					PIXEL P;
					// se obtiene bit por bit (0x01) los de el char* (lLine[i]) que son los colores de los  pixeles a dibujar
					// siendo k el numero de desplazamiento por ciclo por lo que se lee 1 a 1 los bit de iz a derecha
					// (el contador c se explica en 24 bits)
					for (int k = 7; k >= 0; k--) {
						RGBQUAD& Color = Paleta[(pLine[i] >> k) & 0x01];
						P.r = Color.rgbRed;
						P.g = Color.rgbGreen;
						P.b = Color.rgbBlue;
						P.a = 0xff;
						(*pImage)(c, j) = P;
						c += 1;
					}
				}
			}
			delete[] pLine;
		}
		break;
	case 4://4bpp tarea (paletizado)
		{
			RGBQUAD Paleta[16];
			int nColors = bih.biClrUsed ? bih.biClrUsed : 16;
			in.read((char*)&Paleta, nColors * sizeof(RGBQUAD));
			unsigned char* pLine = new unsigned char[rowLength];
			for (int j = bih.biHeight - 1; j >= 0; j--) {
				in.read((char*)pLine, rowLength);
				int c = 0;
				for (int i = 0; i < rowLength - 1; i++) {
					PIXEL P;
					// se obtienen los 2 colores dentor de cada char* primero enmascarando todo el color desplazado 4 bits
					// el segundo es la paleta original pero solo usando los ultimos 4 bits (el contador c se explica en 24 bits)
					RGBQUAD& Color = Paleta[(pLine[i] >> 4) & 0xff];
					P.r = Color.rgbRed;
					P.g = Color.rgbGreen;
					P.b = Color.rgbBlue;
					P.a = 0xff;
					(*pImage)(c, j) = P;
					c += 1;
					RGBQUAD& Color2 = Paleta[(pLine[i]) & 0x0f];
					P.r = Color2.rgbRed;
					P.g = Color2.rgbGreen;
					P.b = Color2.rgbBlue;
				P.a = 0xff;
				(*pImage)(c, j) = P;
				c++;
			}
		}
		delete[] pLine;
		}
		break;	
	case 8://8bpp tarea (paletizado)
		{
			RGBQUAD paleta[256];
			int nColors = bih.biClrUsed ? bih.biClrUsed : 256;
			in.read((char*)&paleta, nColors * sizeof(RGBQUAD));
			unsigned char* pLine = new unsigned char[rowLength];
			for (int j = bih.biHeight - 1; j >= 0; j--)
			{
				in.read((char*)pLine, rowLength);
				for (int i = 0; i < bih.biWidth; i++)
				{
					PIXEL p;
					RGBQUAD& Color = paleta[pLine[i]];
					p.r = Color.rgbRed;
					p.g = Color.rgbGreen;
					p.b = Color.rgbBlue;
					p.a = 0xff;
					(*pImage)(i, j) = p;
				}			
			}
			delete[] pLine;
		}
		break;
	case 24://24bpp tarea (no paletizado)
		{
			// para cada color se usan 8 bits que se asignan de mandera directa a los colores y despues la imagen se pinta
			// como la imagen recorre 24 bits por pixel es necesario un contador (c) que lleve orden en el pintado para no dejar
			// lineas de pixel muertas
			unsigned char* pLine = new unsigned char[rowLength];
			for (int j = bih.biHeight - 1; j >= 0; j--) {
				in.read((char*)pLine, rowLength);
				int c = 0;
				for (int i = 0; i < rowLength; i += 3) {
					PIXEL P;
					P.r = pLine[i + 2];
					P.g = pLine[i + 1];
					P.b = pLine[i];
					P.a = 0xff;
					(*pImage)(c, j) = P;
					c++;
				}
			}
			delete[] pLine;
		}
		break;
	}

	return pImage;
}

#include <fstream>
using namespace std;
void CIPImage::saveToFile(char * pszFileName, CIPImage * pToSave)
{
	//variables
	BITMAPFILEHEADER bfh;
	BITMAPINFOHEADER bih;
	fstream out;
	//abrir y comprobar de abierto archivo a guardar
	out.open(pszFileName, ios::out | ios::binary);
	if (!out.is_open())
		return;
	//asignar memoria
	memset(&bfh, 0, sizeof(bfh));
	memset(&bih, 0, sizeof(bih));
	//llenar file header
	bfh.bfType = 'MB';	
	bfh.bfReserved1 = 0;
	bfh.bfReserved2 = 0;
	//bfh.bfOffBits = 0x36;	
	//bfh.bfSize = sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER) + sizeof(pToSave->m_pBuffer);	
	bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bfh.bfSize = ((pToSave->m_nSizex)*(pToSave->m_nSizey) * 24) / 8 + bfh.bfOffBits;
	//llenar info header	
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biWidth = pToSave->m_nSizex;
	bih.biHeight = pToSave->m_nSizey;
	bih.biPlanes = 1;
	bih.biBitCount = 24;
	bih.biCompression = BI_RGB;
	unsigned int rowLength = (pToSave->m_nSizex + 3) & ~3;
	//unsigned int rowLength = 4 * ((bih.biWidth*bih.biBitCount + 31) / 32);
	bih.biSizeImage = (rowLength * 24 * pToSave->m_nSizey) / 8;
	//bih.biSizeImage = 0;
	//bih.biXPelsPerMeter = 3780;
	//bih.biYPelsPerMeter = 3780;
	bih.biXPelsPerMeter = 0x0ec4;
	bih.biYPelsPerMeter = 0x0ec4;
	bih.biClrUsed = 0;
	bih.biClrImportant = 0;
	//enmascarado
	unsigned int RedMask = 0x00FF0000;
	unsigned int GreenMask = 0x0000FF00;
	unsigned int BlueMask = 0x000000FF;
	unsigned char* pLine = new unsigned char[rowLength];

	out.write((char*)&bfh, sizeof(BITMAPFILEHEADER));
	out.write((char*)&bih, sizeof(BITMAPINFOHEADER));	
	for (int j = bih.biHeight - 1; j >= 0; j--) {
		int c = 0;
		for (int i = 0; i < rowLength; i += 4) {
			/*unsigned char rToSave = (*pToSave)(i + 2, j).r & RedMask;
			unsigned char gToSave = (*pToSave)(i + 1, j).g & GreenMask;
			unsigned char bToSave = (*pToSave)(i, j).b & BlueMask;
			out.write((char*)&rToSave, sizeof(PIXEL));
			out.write((char*)&gToSave, sizeof(PIXEL));
			out.write((char*)&bToSave, sizeof(PIXEL));*/
			out.write((char*)&((*pToSave)(i - 2, j).r), sizeof(PIXEL));
			out.write((char*)&((*pToSave)(i - 1, j).g), sizeof(PIXEL));
			out.write((char*)&((*pToSave)(i, j).b), sizeof(PIXEL));
			c++;
		}
	}
	//Guardar
	/*out.write((char*)&bfh, sizeof(BITMAPFILEHEADER));
	out.write((char*)&bih, sizeof(BITMAPINFOHEADER));
	out.write((char*)pToSave->m_pBuffer, bih.biSizeImage);*/

	out.close();
}
void CIPImage::draw(HDC hdc, int x, int y)
{
	/*1- Crear una memoria en espacio de kernel con el formato de salida (hdc)*/
	HBITMAP hMem = CreateCompatibleBitmap(hdc, m_nSizex, m_nSizey);
	HDC hdcMem = CreateCompatibleDC(hdc);
	/*2- trasnferir memoria de app a memoria de kernel*/
	SetBitmapBits(hMem, m_nSizex*m_nSizey*sizeof(PIXEL), m_pBuffer);
	/*3- transferir datos en memoria de kernel a memoria de dispositivo(GPU)*/
	SelectObject(hdcMem, hMem);
	BitBlt(hdc,x,y,m_nSizex, m_nSizey,hdcMem,0,0,SRCCOPY);
	//Limpiar
	DeleteObject(hMem);
	DeleteDC(hdcMem);
}

//funcion que procesa todos los pixeles con otra funcion especificada en tiempo de ejecucion
void CIPImage::process(PIXEL(*pfn)(int, int, CIPImage *[], int nArgs), CIPImage * pInputs[], int nArgs)
{
	for (int j = 0; j < m_nSizey; j++)
		for (int i = 0; i < m_nSizex; i++)
			(*this)(i, j) = pfn(i, j, pInputs, nArgs);
}

CIPImage::CIPImage()
{
	m_pBuffer = nullptr;
	m_nSizex = 0;
	m_nSizey = 0;
}


CIPImage::~CIPImage()
{
}
