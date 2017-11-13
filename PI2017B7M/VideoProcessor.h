#pragma once
#include "..\AtWareVC32Lib\AtWareVideoCapture.h"
#include "IPImage.h"
#include <list>
class CVideoProcessor:public IAtWareSampleGrabberCB
{
	CRITICAL_SECTION m_cs;
	std::list<CIPImage*> m_ImageQueue;

protected:
	virtual HRESULT SampleCB(double SampleTime, IMediaSample *pSample);
	virtual HRESULT BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen);
public:
	void push(CIPImage* Image);
	CIPImage* Pull();
	AM_MEDIA_TYPE m_MT; //Formato actual de video
	CVideoProcessor();
	~CVideoProcessor();
};