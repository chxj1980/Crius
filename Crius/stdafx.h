// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once
#define MAX_PACKET_BUFFER_LENGTH 10240
#define MAX_QUEUE_LENGTH 10

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include "../BasicLib/global.h"
#include "../BasicLib/SHA1.h"
#include "../BasicLib/MD5.h"
#include "../BasicLib/Base64.h"
#include "../BasicLib/MulitThreadSync.h"
#include "../BasicLib/NetworkSession.h"
#include "../BasicLib/CircularQueue.h"
#include "../BasicLib/Iocp.h"
#include "../BasicLib/MiniDump.h"
#include "../BasicLib/Log.h"
#include "../BasicLib/ConnectionHandle.h"
#include "../BasicLib/ConnectionSession.h"
#include "../BasicLib/NetworkListener.h"
#ifdef _DEBUG
	#pragma comment(lib, "../build/BasicLib_d.lib")
#else 
	#pragma comment(lib, "../build/BasicLib.lib")
#endif
#pragma comment(lib, "winmm.lib")
#include "HttpRequestHeader.h"

