
#include "stdafx.h"
#include "MasterThread.h"

#define	WAIT_FRAME_TIME ( 100 )
#define	COMPUTING_TIME ( 50 )
#define	DISPLAY_TIME ( 50 )

const static char EVENT_COMPUTING[] = "ComputingDataAvailable";
const static char EVENT_DISPLAY[] = "DisplayDataAvailable";

static CMasterThread* pMasterThread = NULL;

unsigned __stdcall DisplayThreadFunction( void* pArguments )  
{
	LPTHREAD_INFO pInfo = (LPTHREAD_INFO) pArguments;

    printf( "\tDisplay Thread Launch ...\n" );  
	HANDLE hDataEvent = OpenEvent(EVENT_ALL_ACCESS, false, EVENT_DISPLAY);

	while( TRUE )
	{
		HANDLE Wait[] = { pMasterThread->hShutdown, pInfo->hDataAvailable };
		DWORD dwWait = WaitForMultipleObjects(sizeof(Wait)/sizeof(HANDLE), Wait, FALSE, INFINITE);

		if (dwWait != (WAIT_OBJECT_0 + 1))
		{
			printf("\tDisplay Thread Exit ...\n");
			return 0;
		}

		if ( TryEnterCriticalSection( & pInfo->CriticalSection ) != 0)
		{
			Sleep( DISPLAY_TIME );
			printf( "\tDisplay Thread Frame %d\n", pInfo->nFrameIndex);

			if (pInfo->pBuffer[0] != (pInfo->nFrameIndex & 0xFF))
				printf("\tDisplay Data dismatch\n");

			int random_number = rand() % DISPLAY_TIME;
			printf("\tDisplay Time = %d\n", random_number);
			Sleep(random_number);

			if (pInfo->pBuffer[0] != (pInfo->nFrameIndex & 0xFF))
				printf("\tDisplay Data overwritten\n");

			LeaveCriticalSection ( &pInfo->CriticalSection );
			ResetEvent(hDataEvent);
		}
	}

   printf( "\tDisplay Thread Shutdown ...\n" );  
}

unsigned __stdcall ComputingThreadFunction( void* pArguments )  
{
	LPTHREAD_INFO pInfo = (LPTHREAD_INFO) pArguments;

    printf( "\tComputing Thread Launch ...\n" );  
	HANDLE hDataEvent = OpenEvent(EVENT_ALL_ACCESS, false, EVENT_COMPUTING);

	while( TRUE )
	{
		HANDLE Wait[] = { pMasterThread->hShutdown, hDataEvent };
		DWORD dwWait = WaitForMultipleObjects(sizeof(Wait)/sizeof(HANDLE), Wait, FALSE, INFINITE);

		if ( dwWait != ( WAIT_OBJECT_0 +1))
		{
			printf("\tComputing Thread Exit ...\n");
			return 0;
		}

		if ( TryEnterCriticalSection( & pInfo->CriticalSection ) != 0)
		{
			printf("\tComputing Thread Frame %d\n", pInfo->nFrameIndex);

			if (pInfo->pBuffer[0] != (pInfo->nFrameIndex & 0xFF) )
				printf("\tComputing Data dismatch\n");

			int random_number = rand() % COMPUTING_TIME;
			printf("\tComputing Time = %d\n", random_number);
			Sleep(random_number);

			if (pInfo->pBuffer[0] != (pInfo->nFrameIndex & 0xFF))
				printf("\tComputing Data overwritten\n");

			LeaveCriticalSection(&pInfo->CriticalSection);
			ResetEvent(hDataEvent);
		}
	}
   printf( "\tComputing Thread Shutdown ...\n" ); 
}


unsigned __stdcall CMasterThread::ThreadFunction( void* pArguments )
{  
    printf( "Master Thread Launch ...\n" );  

	memset( &pMasterThread->DisplayInfo, 0, sizeof ( pMasterThread->DisplayInfo));
	pMasterThread->DisplayInfo.szName = "Display";
	InitializeCriticalSection( & pMasterThread->DisplayInfo.CriticalSection );
	pMasterThread->DisplayInfo.hDataAvailable = CreateEvent( NULL, TRUE, FALSE, EVENT_DISPLAY);
	pMasterThread->DisplayInfo.pBuffer = new unsigned char [ BUFFER_SIZE ];

	memset( &pMasterThread->ComputingInfo, 0, sizeof ( pMasterThread->ComputingInfo));
	pMasterThread->ComputingInfo.szName = "Display";
	InitializeCriticalSection( & pMasterThread->ComputingInfo.CriticalSection );
	pMasterThread->ComputingInfo.hDataAvailable = CreateEvent( NULL, TRUE, FALSE, EVENT_COMPUTING);
	pMasterThread->ComputingInfo.pBuffer = new unsigned char [ BUFFER_SIZE ];

	pMasterThread->hShutdown = CreateEvent( NULL, TRUE, FALSE, "MasterShutdown" );

	pMasterThread->DisplayInfo.hHandle =  (HANDLE)_beginthreadex( 
		NULL, 
		0, 
		&DisplayThreadFunction, 
		&pMasterThread->DisplayInfo, 
		0, 
		NULL );  

 	pMasterThread->ComputingInfo.hHandle =  (HANDLE)_beginthreadex( 
		NULL, 
		0, 
		&ComputingThreadFunction, 
		&pMasterThread->ComputingInfo, 
		0, 
		NULL );   

	unsigned char * pFrameBuffer = new unsigned char [ BUFFER_SIZE ];
	int FrameIndex = 0;

	while( ! pMasterThread->bStart)
		Sleep( 100 );

	while( ! pMasterThread->bStop )
	{
		int random_number = rand() % WAIT_FRAME_TIME;
		printf("Master Wait Frame = %d\n", random_number);
		Sleep(random_number);

		FrameIndex ++;
		printf( "Master Frame Index = %d\n", FrameIndex );  

		int FramePattern = FrameIndex & 0xFF;
		pFrameBuffer[0] = (unsigned char) FramePattern;
//		memset(pFrameBuffer, FramePattern, sizeof(unsigned char) * BUFFER_SIZE);

		if ( TryEnterCriticalSection( & pMasterThread->ComputingInfo.CriticalSection ) != 0)
		{
//			printf("Master Send to Computing = %d\n", FrameIndex);
			memcpy( pMasterThread->ComputingInfo.pBuffer, pFrameBuffer, sizeof( unsigned char ) * BUFFER_SIZE );
			pMasterThread->ComputingInfo.nFrameIndex = FrameIndex;
			LeaveCriticalSection ( & pMasterThread->ComputingInfo.CriticalSection );
			SetEvent( pMasterThread->ComputingInfo.hDataAvailable );
		}

		if ( TryEnterCriticalSection( & pMasterThread->DisplayInfo.CriticalSection ) != 0)
		{
//			printf("Master Send to Display = %d\n", FrameIndex);
			memcpy( pMasterThread->DisplayInfo.pBuffer, pFrameBuffer, sizeof( unsigned char ) * BUFFER_SIZE );
			pMasterThread->DisplayInfo.nFrameIndex = FrameIndex;
			LeaveCriticalSection ( & pMasterThread->DisplayInfo.CriticalSection );
			SetEvent( pMasterThread->DisplayInfo.hDataAvailable );
		}
	}

	printf( "Master Thread End Run\n");  
	SetEvent( pMasterThread->hShutdown );
	HANDLE hWaitThread[] = { pMasterThread->DisplayInfo.hHandle, pMasterThread->ComputingInfo.hHandle };
	WaitForMultipleObjects( sizeof(hWaitThread)/sizeof(HANDLE), hWaitThread, TRUE, INFINITE );
	printf("Child Thread Exit\n");

	CloseHandle( pMasterThread->hShutdown );
	CloseHandle( pMasterThread->ComputingInfo.hDataAvailable  );
	DeleteCriticalSection( & pMasterThread->ComputingInfo.CriticalSection );
	delete [] pMasterThread->ComputingInfo.pBuffer;
		 

	CloseHandle( pMasterThread->DisplayInfo.hDataAvailable );
	DeleteCriticalSection( & pMasterThread->DisplayInfo.CriticalSection );
	delete [] pMasterThread->DisplayInfo.pBuffer;

	ResetEvent(pMasterThread->hShutdown);

	delete [] pFrameBuffer;

    return 0;  
}  



CMasterThread::CMasterThread()
{

}

CMasterThread::~CMasterThread()
{

}

CMasterThread* CMasterThread::Create()
{
	if ( pMasterThread != NULL)
		return pMasterThread;

	pMasterThread = new CMasterThread;
	pMasterThread->bStart = FALSE;
	pMasterThread->bStop = FALSE;

	pMasterThread->hThread =  (HANDLE)_beginthreadex( NULL, 0, &ThreadFunction, NULL, 0, &pMasterThread->threadID );

	return pMasterThread;
}

void CMasterThread::Clean()
{
	if ( pMasterThread != NULL)
		delete pMasterThread;

	pMasterThread = NULL;
}

void CMasterThread::Start()
{
	pMasterThread->bStart = TRUE;
}

void CMasterThread::Stop()
{
	pMasterThread->bStop = TRUE;
	WaitForSingleObject( pMasterThread->hThread, INFINITE );

	CloseHandle(pMasterThread->hThread);
}

