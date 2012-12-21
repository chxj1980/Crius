#pragma once
class CNetworkSession:public CMultiThreadSync<CNetworkSession>
{
public:
	CNetworkSession(void);
	virtual ~CNetworkSession(void);
private:
	OVERLAPPED_EX mAcceptOverlapped;
	OVERLAPPED_EX mReadOverlapped;
	OVERLAPPED_EX mWriteOverlapped;
	SOCKET mSocket;
protected:
	BYTE mReadBuffer[MAX_BUFFER_LENGTH];
public:
	BOOL Begin();
	BOOL End();
	BOOL TcpBind();
	BOOL Close();
	BOOL Listen(USHORT port, INT backLog);
	BOOL Accept(SOCKET listenSocket);
	BOOL Connect(LPSTR address, USHORT port);
	BOOL InitializeReadForIocp();
	BOOL ReadForIocp(BYTE *data, DWORD &dataLength);		//��IOCP��ȡ����
	BOOL ReadForEventSelect(BYTE *data, DWORD &dataLength);	//ѡ��EventSelect��ʽ��ȡ����
	BOOL Write(BYTE *data, DWORD dataLength);				//����д�뻺������׼������
	SOCKET GetSocket(VOID);									//��ȡ��ʵ���������SOCKET����
};

