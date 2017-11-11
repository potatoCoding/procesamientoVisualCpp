#pragma once
#include <DShow.h>
#ifndef SAFE_RELEASE 
	#define SAFE_RELEASE(x) if((x)){(x)->Release(); (x)=NULL;}
#endif

__interface IAtWareSampleGrabberCB
{
public:
	//No usar tiene bugs
	virtual HRESULT SampleCB( double SampleTime, IMediaSample *pSample )    =0;
	//Usar esta
	virtual HRESULT BufferCB( double SampleTime, BYTE *pBuffer, long BufferLen ) = 0;
};
__interface IAtWareVideoCapture
{
	virtual bool Initialize(HWND hWnd)=0;
	virtual bool EnumAndChooseCaptureDevice(void)=0;
	virtual bool EnumCaptureDevices(IEnumMoniker** ppOutEnum)=0;
	virtual bool OpenCaptureDevice(IMoniker* pmnkCaptureDevice)=0;
	virtual void ShowConfigureCaptureDeviceDialog(void)=0;
	virtual bool ShowVFWLegacyDialogs()=0;
	virtual void GetCaptureDeviceControllers(IAMVideoProcAmp** ppOutIAMVideoProcAmp,
										  IAMCameraControl** ppOutIAMCameraControl,
										  IAMStreamConfig** ppOutIAMStreamConfig)=0;
	virtual void SetMediaType(AM_MEDIA_TYPE* pInMT)=0;
	virtual bool BuildStreamGraph(void)=0;
	virtual void GetMediaType(AM_MEDIA_TYPE* pOutMT)=0;
	virtual void Start(void)=0;
	virtual void Stop(void)=0;
	virtual void ShowPreviewWindow(BOOL bShow)=0;
	virtual void SendPreviewWindowMessage(HWND,UINT,WPARAM,LPARAM)=0;
	virtual void SetCallBack(IAtWareSampleGrabberCB* pCallBack,ULONG ulMethodToCall)=0;
	virtual void SetPreviewWindowPosition(RECT* rc)=0;
	virtual void Uninitialize(void)=0;
};

IAtWareVideoCapture* CreateAtWareVideoCapture(void);
void DestroyAtWareVideoCapture(IAtWareVideoCapture* pVC);


