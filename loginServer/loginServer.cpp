#include <thread>

#include "loginServer.h"

#include "lib/IOCP netServer/serializer.h"

#include "monitoringTools/monitoringTools/messageLogger.h"
#include "monitoringTools/monitoringTools/performanceProfiler.h"
#include "monitoringTools/monitoringTools/resourceMonitor.h"

#pragma comment(lib, "monitoringTools\\x64\\Release\\monitoringTools")

using namespace std;

loginServer::loginServer(unsigned long long int _timeoutThreshold) : timeoutThreshold(_timeoutThreshold)
{
	run = true;

	timeoutThread = new thread(&loginServer::update, this);
}
loginServer::~loginServer() 
{
	run = false;

	timeoutThread->join();
}

void loginServer::OnNewConnect(unsigned long long sessionID)
{
	auto sessionPtr = findSession(sessionID);

	if (sessionPtr != nullptr)
	{
		connectionData* connection = connectionPool.New(sessionPtr->getIp());

		AcquireSRWLockExclusive(&containorLock);
		connectionMap[sessionID] = connection;
		ReleaseSRWLockExclusive(&containorLock);

		if (sessionPtr->decrementIO() == 0)
			deleteSession(sessionPtr);
			
	}
	else
	{
		LOGOUT(logLevel::Error, LO_TXT) << "�̰��� �� ���ɼ��� ����" << LOGEND;
	}
}
void loginServer::OnDisconnect(unsigned long long sessionID)
{
	AcquireSRWLockExclusive(&containorLock);
	connectionMap.erase(sessionID);
	ReleaseSRWLockExclusive(&containorLock);
}
bool loginServer::OnConnectionRequest(unsigned long ip, unsigned short port) 
{
	return true;
}
void loginServer::OnRecv(unsigned long long sessionID, serializer* _packet) 
{
	char code;
	short payloadSize;
	char key;
	char checkSum;

	*_packet >> code >> payloadSize >> key;

	if (_packet->decryption(getStatickey())){
		*_packet >> checkSum;
	}
	else
	{
		LOGOUT_EX(logLevel::Error, LO_TXT, "content") << "checkSum error ID " << sessionID << LOGEND;
		disconnectReq(sessionID);

		return;
	}




	WORD type;
	*_packet >> type;

	if (type != 101){
		disconnectReq(sessionID);
		return;
	}




	unsigned long long accountNo;
	char* sessionKey;

	*_packet >> accountNo;
	sessionKey = _packet->getHeadPtr();

	if (!sessionKeyCheckByDatabase(accountNo, sessionKey)){
		disconnectReq(sessionID);
		return;
	}





	redisUpdate(accountNo, sessionKey);


	if(true)
	{
		packet response;

		WCHAR temp[20];
		WCHAR chattingServerIp[16];// = L"127.0.0.1";
		memset(chattingServerIp, 0, sizeof(WCHAR) * 16);
		wmemcpy_s(chattingServerIp, 16, L"127.0.0.1", lstrlenW(L"127.0.0.1"));


		response << (WORD)102 << accountNo << (BYTE)1;
		response.setWstring(temp, 20);			//id
		response.setWstring(temp, 20);		//nickname
		response.setWstring(temp, 16);		//game IP
		response << (unsigned short)0;			//game port
		response.setWstring(chattingServerIp, 16);		//chat IP
		response << (unsigned short)12001;		//chat port

		response.setHeader(getRandKey());
		response.encryption(getStatickey());

		sendReq(sessionID, response);

	}
	else
	{
		serializer* response = serializerPool.Alloc();
		response->clear();
		response->moveRear(sizeof(networkHeader));
		response->moveFront(sizeof(networkHeader));
		response->incReferenceCounter();

		WCHAR temp[20];
		WCHAR chattingServerIp[16];// = L"127.0.0.1";
		memset(chattingServerIp, 0, sizeof(WCHAR) * 16);
		wmemcpy_s(chattingServerIp, 16, L"127.0.0.1", lstrlenW(L"127.0.0.1"));

		//int size = mbstowcs(NULL, connectionMap[sessionID]->ip, 0) + 1;
		//mbstowcs(chattingServerIp, connectionMap[sessionID]->ip, size);

		*response << (WORD)102 << accountNo << (BYTE)1;
		response->setWstring(temp, 20);			//id
		response->setWstring(temp, 20);		//nickname
		response->setWstring(temp, 16);		//game IP
		*response << (unsigned short)0;			//game port
		response->setWstring(chattingServerIp, 16);		//chat IP
		*response << (unsigned short)12001;		//chat port

		response->setHeader(getRandKey());
		response->encryption(getStatickey());

		sendReq(sessionID, response);


		if (response->decReferenceCounter() == 0)
			serializerPool.Free(response);
	}
}
void loginServer::OnSend(unsigned long long sessionID, int sendsize) 
{
	//
	// �Ұ� ����
}




void loginServer::update() 
{
	while (run)
	{
		Sleep(10000);
		unsigned long long int currentTick = GetTickCount64();
		AcquireSRWLockShared(&containorLock);
		for (auto it : connectionMap) {
			if (currentTick - it.second->timeOutTick > timeoutThreshold)
			{
				LOGOUT(logLevel::Info, LO_TXT | LO_CMD) << "login timeout " << it.first << LOGEND;
				disconnectReq(it.first);
			}
		}
		ReleaseSRWLockShared(&containorLock);
	}
}





bool loginServer::sessionKeyCheckByDatabase(unsigned long long _accuntNo, char* _sessionKey)
{
	//
	//	DB�� �����Ͽ� ���������� �ش� ����Ű�� ��ī��Ʈ �ѹ��� �´��� Ȯ����
	//
	//
	//
	//
	Sleep(1);
	return true;

}


void loginServer::redisUpdate(unsigned long long _accuntNo, char* _sessionKey)
{
	Sleep(1);
	return;
}