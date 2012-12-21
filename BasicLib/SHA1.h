#pragma once
class CSHA1
{
public:
	CSHA1();
	virtual ~CSHA1();

	//����
	VOID Reset();

	//����digest����
	BOOL Result(LPDWORD lpDigest);		//����Ϊ5��DWORD����
	BOOL Result(PUCHAR lpDigest);		//����Ϊ20��UCHAR����
    
	//����Դ
	VOID Input(const PUCHAR lpMessage, DWORD length);
	VOID Input(const PCHAR lpMessage, DWORD length);

private:
	//������һ��512bitmessage
	VOID ProcessMessageBlock();
	
	//��ǰ��message������512bit
	VOID PadMessages();
      
	//ѭ�����Ʋ���
    inline DWORD CircularShift(INT bits, DWORD word);
     
    DWORD mBuffer[5];	//digest������

	DWORD mLength_Low;		//message length(bits)
	DWORD mLength_High;

    UCHAR mMessageBlock[64]; //521-bit message blocks
    INT mMessageBlockIndex;	 //message block������
    BOOL mComputed;		//
	BOOL mCorrupted;    
};

