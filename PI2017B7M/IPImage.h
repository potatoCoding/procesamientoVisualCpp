#pragma once
#include <Windows.h>
class CIPImage
{
public:
	struct PIXEL
	{
		union 
		{
			struct 
			{
				 char b, g, r, a;
			};
			 long n;
		};
		
	};
protected:
	PIXEL* m_pBuffer;
	int m_nSizex;
	int m_nSizey;
public:
	int getSizeX() { return m_nSizex; };
	int getSizeY() { return m_nSizey; };
	PIXEL& operator()(int i, int j);
	PIXEL sample(float i, float j);
	static CIPImage* createImage(int sx, int sy);
	static void destroyImage(CIPImage* pToDestroy);
	static CIPImage* CaptureDesktop();
	static CIPImage* loadFromFile(char* pszFileName);
	static void saveToFile(char* pszFileName, CIPImage* pToSave);
	void draw(HDC hdc, int x, int y);
	void process(PIXEL (*pfn)(int, int, CIPImage*[],int nArgs), CIPImage* pInputs[], int nArgs);
protected:
	CIPImage();
	virtual ~CIPImage();
};

