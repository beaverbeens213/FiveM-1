#pragma once

#include "fiDevice.h"
#include <vector>

#ifdef COMPILING_GAMESPEC
#include <zlib.h>
#endif

struct StreamingItem
{
	int pad;
	int blockSize;
	rage::fiDevice* device;
	uint32_t streamCounter;
	DWORD flags;
	int pad2;
	union
	{
		int fileStart;
		const char* fileName;
	};

	void completeRequest();
};

static StreamingItem*& g_streamingItems = *(StreamingItem**)0x184A2EC;
static uint32_t& g_streamMask = *(uint32_t*)0x184A268;

struct StreamRequestPage
{
	void* buffer;
	uint32_t length;
	int pad;
};

struct StreamRequest
{
	int itemIdx;
	int pad;
	StreamRequestPage pages[127];
	char pad2[8];
	uint32_t reqLength;
	uint32_t reqStart; // 1544

	void(*completionCB)(int userData, void* pageBuffer, uint32_t length, int); // 1548
	int completionCBData; // 1552
};

#ifdef COMPILING_GAMESPEC
struct StreamRequestExt : public StreamRequest
{
	StreamingItem* item;
	bool isResource;
	OVERLAPPED overlapped;
	uint32_t reqRead;
	uint32_t pageNum;
	char* readBuffer;
	uint32_t handle;
	uint32_t lastAvailOut;

	z_stream* strm;

	void NextPage();
};

struct StreamThread_GtaLocal
{
	StreamRequest requests[16];
	CRITICAL_SECTION counterLock; // 24896
	char pad[12]; // 24920
	int requestNum; // 24932
	int pendingRequests; // 24936
	HANDLE hSemaphore;
};

class StreamThread;

class StreamEvent
{
public:
	virtual void RunEvent(StreamThread* thread) = 0;
};

class NewRequestEvent : public StreamEvent
{
private:
	StreamRequest* m_request;

public:
	NewRequestEvent(StreamRequest* request)
		: m_request(request)
	{

	}

	virtual void RunEvent(StreamThread* thread);
};

class IOCompletedEvent : public StreamEvent
{
private:
	StreamRequestExt* m_request;

public:
	IOCompletedEvent(StreamRequestExt* request)
		: m_request(request)
	{

	}

	virtual void RunEvent(StreamThread* thread);
};

class StreamThread
{
private:
	std::vector<StreamRequestExt> m_requests;

	StreamThread_GtaLocal* m_local;

	HANDLE m_objects[64];

	LPOVERLAPPED m_curOverlapped;

	int m_threadID;

	z_stream m_streams[16];

	char m_readBuffers[16][131072];

	int m_curStream;

	int m_curReadBuffer;

public:
	inline void SetThreadID(int num) { m_threadID = num; }

	z_stream* GetStream();

	char* GetReadBuffer();

	void RunThread(StreamThread_GtaLocal* local);

	StreamEvent* WaitIdly();

	inline LPOVERLAPPED GetCurrentOverlapped()
	{
		return m_curOverlapped;
	}

	void QueueRequest(StreamRequestExt& request);

	void RemoveRequest(StreamRequestExt* request);

	void TriggerNextIO(StreamRequestExt* request);
};
#endif

void GAMESPEC_EXPORT StreamWorker_Thread(int threadNum);
LPOVERLAPPED GAMESPEC_EXPORT StreamWorker_GetOverlapped(int streamThreadNum);