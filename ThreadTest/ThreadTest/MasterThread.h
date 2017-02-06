#ifndef MASTER_THREAD_CLASS_DECLARTION
#define MASTER_THREAD_CLASS_DECLARTION

#include <process.h>
#include <windows.h>  
#include <string>

#define BUFFER_SIZE				(16 * 1024 * 1024 )

typedef struct tagTHREAD_INFO
{
	std::string szName;
	HANDLE		hHandle;
	DWORD		dwID;
	CRITICAL_SECTION	CriticalSection;
	HANDLE				hDataAvailable;

	unsigned long	nFrameIndex;
	unsigned char * pBuffer;
}	THREAD_INFO, *LPTHREAD_INFO;

class CMasterThread
{
private:
	CMasterThread();
	~CMasterThread();

	HANDLE hThread;	
	unsigned threadID; 

public:
	static CMasterThread* Create();
	static void Clean();

	static unsigned __stdcall ThreadFunction(void* pArguments);

	void Start();
	void Stop();

	BOOL bStart;
	BOOL bStop;

	HANDLE	hShutdown;

	THREAD_INFO	DisplayInfo;
	THREAD_INFO	ComputingInfo;
};

#endif