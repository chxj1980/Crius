#pragma once
class HttpRequestHeader
{
public:
	HttpRequestHeader(void);
	virtual ~HttpRequestHeader(void);
public:
	VOID Input(BYTE *sources, DWORD dataLength);
	BOOL Complete();
	VOID Add(std::string key, std::string value);
	const std::string& GetHost();
	const std::string& GetUpgrade();
	const std::string& GetConnection();
	const std::string& GetOrigin();
	const std::string& GetField(LPSTR key);
	const std::string& GetPath();
	inline DWORD ContentSize(){return mContentSize;}
	inline BYTE * Content() {return mContent;}

private:
	BYTE *mData;
	DWORD mDataLength;
	DWORD mHeadSize;

	std::string mMethod;						//���󷽷�
	std::string mPath;							//����·��
	std::string mHttpVersion;					//HttpЭ��汾
	std::map<std::string,std::string> mFields;	//�����������
	std::string empty;
	BYTE *mContent;								//����
	DWORD mContentSize;							
};

