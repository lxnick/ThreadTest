// ThreadTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdlib.h>  
#include <stdio.h>  
#include <time.h>  

#include "MasterThread.h"

int _tmain(int argc, _TCHAR* argv[])
{
	for (int i = 0; i < 100; i++)
	{
		srand((unsigned)time(NULL));

		CMasterThread * pMasterThread = CMasterThread::Create();
		Sleep(100);

		pMasterThread->Start();
		Sleep(1000 * 5);
		pMasterThread->Stop();

		CMasterThread::Clean();
	}

	return 0;
}

