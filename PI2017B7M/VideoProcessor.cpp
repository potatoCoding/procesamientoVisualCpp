#include "VideoProcessor.h"

HRESULT CVideoProcessor::SampleCB(double SampleTime, IMediaSample *pSample) {
	return S_OK;
}
HRESULT CVideoProcessor::BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen) {
	//aqui procesaremos la recepcion de la muestra!!!
	VIDEOINFOHEADER* pVIH = (VIDEOINFOHEADER*)(m_MT.pbFormat);
	switch (pVIH->bmiHeader.biCompression)
	{
	case BI_RGB: {
		CIPImage* pFrame = CIPImage::createImage(pVIH->bmiHeader.biWidth, pVIH->bmiHeader.biHeight);
		for (int j = 0; j < pFrame->getSizeY(); j++)
		{
			memcpy(&(*pFrame)(0, j), pBuffer + 
				(pFrame->getSizeX()*(pFrame->getSizeY() - j - 1) * 4),
					pFrame->getSizeX()*4);
		}
		push(pFrame);
	}
		break;
	case MAKEFOURCC('Y', 'U', 'Y', '2')://YUV

		break;
	default:
		break;
	}
	return S_OK;
}

int max_Fps = 30;

void CVideoProcessor::push(CIPImage * pImage)
{
	EnterCriticalSection(&m_cs);
	if (m_ImageQueue.size() >= max_Fps) {
		CIPImage* pDrop = *m_ImageQueue.begin();
			m_ImageQueue.pop_front();
		CIPImage::destroyImage(pDrop);
	}
	m_ImageQueue.push_back(pImage);
	LeaveCriticalSection(&m_cs);
}

CIPImage * CVideoProcessor::Pull()
{
	CIPImage* pImage = NULL;
	EnterCriticalSection(&m_cs);
	if (m_ImageQueue.size() >= 1) {
		pImage = *m_ImageQueue.begin();
		m_ImageQueue.pop_front();
	}
	LeaveCriticalSection(&m_cs);
	return pImage;
}

CVideoProcessor::CVideoProcessor()
{
	InitializeCriticalSection(&m_cs);
}


CVideoProcessor::~CVideoProcessor()
{
}
