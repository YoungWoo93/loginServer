#include <thread>

#include "loginServer.h"

#include "lib/IOCP netServer/serializer.h"
#include "lib/mysql connector/mysqlConnector.h"
#include "lib/redis connector/redisConnector.h"

#include "monitoringTools/monitoringTools/messageLogger.h"
#include "monitoringTools/monitoringTools/performanceProfiler.h"
#include "monitoringTools/monitoringTools/resourceMonitor.h"

#pragma comment(lib, "monitoringTools\\x64\\Release\\monitoringTools")

using namespace std;

loginServer::loginServer(unsigned long long int _timeoutThreshold) : timeoutThreshold(_timeoutThreshold)
{
	mysqlConnector::init();
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
		LOGOUT(logLevel::Error, LO_TXT) << "이곳을 올 가능성은 없음" << LOGEND;
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



		packet response;

		WCHAR temp[20];
		WCHAR chattingServerIp[16];// = L"127.0.0.1";
		memset(chattingServerIp, 0, sizeof(WCHAR) * 16);
		wmemcpy_s(chattingServerIp, 16, L"10.0.1.1", lstrlenW(L"10.0.1.1"));


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
void loginServer::OnSend(unsigned long long sessionID, int sendsize) 
{
	//
	// 할거 없음
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





bool loginServer::sessionKeyCheckByDatabase(unsigned long long _accountNo, char* _sessionKey)
{
	//
	//	DB와 연결하여 동기적으로 해당 세션키와 어카운트 넘버가 맞는지 확인함
	//
	//
	//
	
	vector<string> temp = {"sessionKey" };
	dbResult result(temp);
	string q = "select sessionkey from sessionkey where accountno = ";
	q += to_string(_accountNo);
	q += ";";
	mysqlConnector::query(q.c_str(), result);

	return true;

}


void loginServer::redisUpdate(unsigned long long _accountNo, char* _sessionKey)
{
	redisConnector::set(to_string(_accountNo), string(_sessionKey, 64), 1);

	return;
}
