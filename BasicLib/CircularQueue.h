#pragma once

//����ѭ�������е����ݽṹ
typedef struct _QUEUE_DATA
{
	VOID *Object;		//ָʾԭ�ж���
	BYTE Data[MAX_PACKET_BUFFER_LENGTH];
	DWORD DataLength;
	CHAR RemoteAddress[32];		//Զ�����ӵĵ�ַ
	USHORT Remoteport;			//���Ӷ˿�

} QUQUE_DATA;

class CCircularQueue:public CMultiThreadSync<CCircularQueue>
{
public:
	CCircularQueue();
	~CCircularQueue();
private:
	QUQUE_DATA mQueue[MAX_QUEUE_LENGTH];
	DWORD mQueueHead;		//ָʾHead��λ��
	DWORD mQueueTail;		//ָʾTail��λ��
public:
	BOOL Begin();
	BOOL End();
	BYTE * Push(VOID *object, BYTE *data, DWORD dataLength, LPSTR remoteAddress, USHORT remotePort);
	BYTE * Push(VOID *object, BYTE *data, DWORD dataLength);
	BOOL Pop(VOID **object, BYTE *data, DWORD &dataLength);
	BOOL Pop(VOID **object, BYTE *data, DWORD &dataLength, LPSTR remoteAddress, USHORT &remotePort);
	BOOL Pop();
	BOOL IsEmpty();
};

