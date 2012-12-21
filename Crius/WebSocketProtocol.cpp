#include "stdafx.h"
#include "WebSocketProtocol.h"
#define OP_CLOSE 8
#define OP_TEXT 1

CWebSocketProtocol::CWebSocketProtocol(void)
{
	mDataBuffer = NULL;
	mPacketBuffer = NULL;
	mRemainLength = 0;
	mIshandShaked = FALSE;
}


CWebSocketProtocol::~CWebSocketProtocol(void)
{
}

BOOL CWebSocketProtocol::Begin()
{
	mWebsocketVersion = 0;
	if (mDataBuffer) delete [] mDataBuffer;
	mDataBuffer = new BYTE[MAX_PACKET_BUFFER_LENGTH];
	if (mPacketBuffer) delete [] mPacketBuffer;
	mPacketBuffer = new BYTE[MAX_PACKET_BUFFER_LENGTH];
	mIshandShaked = FALSE;
	return TRUE;
}

BOOL CWebSocketProtocol::End()
{
	if (mDataBuffer) delete [] mDataBuffer;
	if (mPacketBuffer) delete [] mPacketBuffer;
	mPacketBuffer = 0;
	mWebsocketVersion = 0;
	mDataBuffer = 0;
	mRemainLength = 0;
	mIshandShaked = FALSE;
	return TRUE;
}

VOID CWebSocketProtocol::Translate(CConnectionSession<CWebSocketProtocol> &pConn, BYTE *data, DWORD dataLength)
{
	if (dataLength < 0 || ((dataLength + mRemainLength) > MAX_PACKET_BUFFER_LENGTH))
	{
		//����������������ȣ���Ϊ�����
		mRemainLength = 0;
		ZeroMemory(mDataBuffer, MAX_PACKET_BUFFER_LENGTH);
		return;
	}
	CopyMemory(mDataBuffer + mRemainLength, data, dataLength);
	mRemainLength += dataLength;

	if (!mIshandShaked)		//�����û���֣���������
	{
		HttpRequestHeader  RequestHeader;
		RequestHeader.Input(mDataBuffer, mRemainLength);
		if (RequestHeader.Complete())
		{
			HandShake(RequestHeader, pConn);
		}
	}
	else 
	{
		//��������������ݰ�������Handle����
		DWORD64 PacketLength;
		while((this->*GetPacket)(pConn, PacketLength))
		{
			if (!mPacketBuffer) return;
			mPacketBuffer[PacketLength] = '\0';
			if (pConn.GetHandle())
			pConn.GetHandle()->Handle((char *)mPacketBuffer);
		}
	}
}

BOOL CWebSocketProtocol::HandShake(HttpRequestHeader &header, CConnectionSession<CWebSocketProtocol> &pConn)
{
	if (header.GetField("Sec-WebSocket-Key").size() > 0 && header.GetField("Sec-WebSocket-Version") == "13")
	{
		//V76Э��
		std::string RawKey = header.GetField("Sec-WebSocket-Key") + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
		//��ȡSec-WebSocket-KeyȻ����Ϲ̶��ַ�����ϳ��µ�Key

		//��Key  SHA1����BASE64����ɵ��ַ�����ΪSec-WebSocket-Accept��ֵ���ظ��ͻ���
		BYTE DigestKey[20];
		CSHA1 sha1;
		CBase64 base64;
		sha1.Input((CHAR *)RawKey.c_str(), RawKey.size());
		sha1.Result(DigestKey);
		
		std::string str = base64.Encode(DigestKey, 20);

		std::string strResponse("HTTP/1.1 101 Switching Protocols\r\n");
		strResponse += "Upgrade: websocket\r\n";
		strResponse += "Connection: Upgrade\r\n";
		strResponse += "Sec-WebSocket-Accept: " + str + "\r\n";
		strResponse += "WebSocket-Origin: " + header.GetOrigin() + "\r\n";
		strResponse += "WebSocket-Location: ws://";
		strResponse += header.GetHost() + header.GetPath() + "\r\n\r\n";
		
		pConn.WritePacket((BYTE *)strResponse.c_str(), strResponse.size());
		mWebsocketVersion = 76;
		mIshandShaked = TRUE;
		mRemainLength = 0;
		ZeroMemory(mDataBuffer, MAX_PACKET_BUFFER_LENGTH);
		GetPacket = &CWebSocketProtocol::GetPacket76;
		WritePacket = &CWebSocketProtocol::WritePacket76;
		return TRUE;

	}
	else if (header.GetField("Sec-WebSocket-Key1").size() > 0)
	{
		//�����V75Э�飬ͷ���滹����8�ֽڵ�����
		if (header.ContentSize() == 8)
		{
			DWORD Space1Num = 0;
			DWORD Space2Num = 0;
			DWORD Key1Length;
			DWORD Key2Length;
			BYTE Digits1[20], Digits2[20];
			DWORD Index = 0;
			DWORD Result1, Result2;

			const char *szkey1 = header.GetField("Sec-WebSocket-Key1").c_str();
			const char *szKey2 = header.GetField("Sec-WebSocket-Key2").c_str();
			Key1Length = strlen(szkey1);
			Key2Length = strlen(szKey2);

			//���ո񣬻�ȡ����
			for (DWORD i = 0; i<Key1Length; i++)
			{
				if (szkey1[i] == ' ') Space1Num++;
				else if (szkey1[i] >= '0' && szkey1[i] <= '9')
				{
					Digits1[Index++] = szkey1[i];
				}
			}

			Index = 0;
			for (DWORD i = 0; i<Key2Length; i++)
			{
				if (szKey2[i] == ' ') Space2Num++;
				else if (szKey2[i] >= '0' && szKey2[i] <= '9')
				{
					Digits2[Index++] = szKey2[i];
				}
			}

			//����Key1, Key2��ȡ�����޷�����������Big Endian
			Result1 = htonl((DWORD)((strtoul((const char *)Digits1, NULL, 10)) / Space1Num));
			Result2 = htonl((DWORD)((strtoul((const char *)Digits2, NULL, 10)) / Space2Num));

			//����������������8���ֽںϲ���16�ֽ�����
			BYTE ResultKey[16];
			CopyMemory(ResultKey, &Result1, sizeof(DWORD));
			CopyMemory(ResultKey + sizeof(DWORD), &Result2, sizeof(DWORD));
			CopyMemory(ResultKey + sizeof(DWORD) * 2, header.Content(), 8);


			std::string strResponse("HTTP/1.1 101 Web Socket Protocol Handshake\r\n");
			strResponse += "Upgrade: WebSocket\r\n";
			strResponse += "Connection: Upgrade\r\n";
			strResponse += "Sec-WebSocket-Origin: ";
			strResponse += header.GetOrigin() + "\r\n";
			strResponse += "Sec-WebSocket-Location: ws://";
			strResponse += header.GetHost() + header.GetPath() + "\r\n\r\n";

			//Md5�õ���16�ֽ� ���ͳ�ȥ
			CMD5 md5(ResultKey, 16);
			BYTE *md5Key = (BYTE *)md5.Digest();

			BYTE Response[200];
			DWORD ResponseHeaderLength = strResponse.size();
			CopyMemory(Response, strResponse.c_str(), ResponseHeaderLength);
			CopyMemory(Response + ResponseHeaderLength, md5Key, 16);
			pConn.WritePacket(Response, ResponseHeaderLength+16);
			Response[ResponseHeaderLength+16] = '\0';
		
			GetPacket = &CWebSocketProtocol::GetPacket75;
			WritePacket = &CWebSocketProtocol::WritePacket75;
			ZeroMemory(mDataBuffer, MAX_PACKET_BUFFER_LENGTH);
			mRemainLength = 0;
			mIshandShaked = TRUE;
			return TRUE;
		}
	}
	else 
	{
		//�������δʵ�ֵ�Э�飬������
		mRemainLength = 0;
		return FALSE;
	}
	
	return FALSE;
}

BOOL CWebSocketProtocol::GetPacket76(CConnectionSession<CWebSocketProtocol> &pConn, DWORD64 &packetLength)
{
	if (mRemainLength < 6)
		return FALSE;
	DWORD64 PacketLength = 0;
	UCHAR opCode = mDataBuffer[0] & 0x0F;
	

	if (opCode != OP_TEXT)
	{
		//Ŀǰֻ�����ı���Ϣ
		if (opCode == OP_CLOSE)
		{
			pConn.End();
		}
		mRemainLength = 0;
		return FALSE;
	}
	
	if (!(mDataBuffer[0] & 0x80))
	{
		mRemainLength = 0;
		ZeroMemory(mDataBuffer, MAX_PACKET_BUFFER_LENGTH);
		return FALSE;
		//��֡��Ϣ���ݲ�����
	}

	if(!(mDataBuffer[1] & 0x80))		//û������, �ݲ�����
	{
		mRemainLength = 0;
		ZeroMemory(mDataBuffer, MAX_PACKET_BUFFER_LENGTH);
		return FALSE;
	}
	
	PacketLength = mDataBuffer[1] & 0x7F;
	BYTE Masks[4] = {0,};
	DWORD64 TotalPacketLength;
	
	DWORD MaskOffset = 2;
	if (PacketLength < 126)
	{
		TotalPacketLength = PacketLength + 6;
	}
	else if (PacketLength == 126)
	{
		MaskOffset = 4;
		CopyMemory(&PacketLength, mDataBuffer+2, 2);	//�������=126,��ô����2�ֽڲ���֡���ݵ���ʵ���� 
		PacketLength = ntohs((USHORT)PacketLength);		//Big Endianת��
		TotalPacketLength = PacketLength + 8;			//�������ݰ�����
	}
	else if (PacketLength == 127)
	{
		MaskOffset = 10;
		CopyMemory(&PacketLength, mDataBuffer+2, 8);	//�������=127,��ô����8�ֽڲ���֡���ݵ���ʵ���� 
		PacketLength = NTOH64(PacketLength);
		TotalPacketLength = PacketLength + 14;
		//��������Ϊ��������ԭ�򣬲�֧����ô�������ݰ���PASS
	}

	if (PacketLength < 0 || TotalPacketLength > MAX_PACKET_BUFFER_LENGTH)
	{
		//������ݰ����ȳ������������Ȼ���С��0����Ϊ�Ǵ���İ����ӵ�
		mRemainLength = 0;
		ZeroMemory(mDataBuffer, MAX_PACKET_BUFFER_LENGTH);
		return  FALSE;
	}

	//����ѽ������ݳ���>�����ȣ���ʾ�Ѿ��յ�һ�������İ�
	if (TotalPacketLength <= mRemainLength)
	{
		//��������,��������
		CopyMemory(Masks, mDataBuffer+MaskOffset, 4);
		CopyMemory(mPacketBuffer, mDataBuffer+MaskOffset+4, (size_t)PacketLength);

		//ʹ�������������
		for (INT i=0; i<PacketLength; i++)
			mPacketBuffer[i] = mPacketBuffer[i] ^  Masks[i % 4];

		packetLength = PacketLength;

		//�������������ʣ�µ����ݣ���ô��ʣ�µ���ǰ��
		if(mRemainLength - TotalPacketLength > 0)
		{
			MoveMemory(mDataBuffer, mDataBuffer + (size_t)TotalPacketLength, mRemainLength-(DWORD)TotalPacketLength);
		}
		mRemainLength -= (DWORD)TotalPacketLength;

		//���軺����
		if (mRemainLength <= 0)
		{
			mRemainLength = 0;
			ZeroMemory(mDataBuffer, sizeof(mDataBuffer));
		}
		return TRUE;
	}
	return FALSE;
}

BOOL CWebSocketProtocol::GetPacket75(CConnectionSession<CWebSocketProtocol> &pConn, DWORD64 &packetLength)
{
	if (mDataBuffer[0] != 0)
	{
		//�������00��ͷ,��ʾΪ��������ݰ�,�ӵ�
		mRemainLength = 0;
		ZeroMemory(mDataBuffer, MAX_PACKET_BUFFER_LENGTH);
		return FALSE;
	}
	else 
	{
		DWORD PacketLength = 0;
		DWORD dwEnd = 0;
		for (DWORD i=0; i<mRemainLength; i++)
		{
			//Ѱ��0xFF��λ��
			if (mDataBuffer[i] == 0xFF)
			{
				dwEnd = i;
				break;
			}
		}

		if (dwEnd > 1)
		{
			PacketLength = dwEnd;
			CopyMemory(mPacketBuffer, mDataBuffer+1, PacketLength-1);
			packetLength = PacketLength-1;

			//�������������ʣ�µ����ݣ���ô��ʣ�µ���ǰ��
			if (mRemainLength-PacketLength-1 > 0)
			{
				MoveMemory(mDataBuffer, mDataBuffer + PacketLength+1, mRemainLength-PacketLength-1);
			}
			mRemainLength -= (PacketLength+1);

			//���軺����
			if (mRemainLength <= 0)
			{
				mRemainLength = 0;
				ZeroMemory(mDataBuffer, MAX_PACKET_BUFFER_LENGTH);
			}
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CWebSocketProtocol::Write(CConnectionSession<CWebSocketProtocol> &pConn, BYTE *data, DWORD dataLength)
{
	return (this->*WritePacket)(pConn, data, dataLength);
}

BOOL CWebSocketProtocol::WritePacket76(CConnectionSession<CWebSocketProtocol> &pConn, BYTE *data, DWORD dataLength)
{
	if (!data || dataLength <= 0) return FALSE;
	//v76��Э���װ
	BYTE TempBuffer[MAX_PACKET_BUFFER_LENGTH];
	TempBuffer[0] = 0x81;
	if (dataLength < 126)
	{

		BYTE TempLength = (BYTE)dataLength;
		CopyMemory(TempBuffer+1, &TempLength, 1);
		CopyMemory(TempBuffer+2, data, dataLength);
		return pConn.WritePacket(TempBuffer, dataLength+2);
		
	}
	else if (dataLength < USHRT_MAX)
	{
		BYTE TempLength = 126;
		USHORT shortLength = htons((USHORT)dataLength);
		CopyMemory(TempBuffer+1, &TempLength, 1);
		CopyMemory(TempBuffer+2, &shortLength, sizeof(USHORT));
		CopyMemory(TempBuffer+2 + sizeof(USHORT), data, dataLength);
		return pConn.WritePacket(TempBuffer, dataLength+2 + sizeof(USHORT));
	}
	else 
	{
		BYTE TempLength = 127;
		DWORD64 Len64 = NTOH64((DWORD64)dataLength);
		CopyMemory(TempBuffer+1, &TempLength, 1);
		CopyMemory(TempBuffer+2, &Len64, sizeof(DWORD64));
		CopyMemory(TempBuffer+2 + sizeof(DWORD64), data, dataLength);
		return pConn.WritePacket(TempBuffer, dataLength+2 + sizeof(DWORD64));

	}
	return FALSE;
}

BOOL CWebSocketProtocol::WritePacket75(CConnectionSession<CWebSocketProtocol> &pConn, BYTE *data, DWORD dataLength)
{
	if (!data || dataLength <= 0 || dataLength > (MAX_PACKET_BUFFER_LENGTH-2))
		return FALSE;

	BYTE TempBuffer[MAX_PACKET_BUFFER_LENGTH] = {0,};
	TempBuffer[0] = 0;
	CopyMemory(TempBuffer+1, data, dataLength);
	TempBuffer[dataLength+1] = 0xFF;
	return pConn.WritePacket(TempBuffer, dataLength+2);
}