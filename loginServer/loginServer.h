#pragma once

#include "lib/IOCP netServer/network.h"
#include "lib/IOCP netServer/session.h"
#include <Windows.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <unordered_map>



#pragma comment(lib, "ws2_32.lib")

typedef unsigned long long int sessionID;

struct connectionData
{
	connectionData() {
	}
	connectionData(char* _ip) : timeOutTick(GetTickCount64()){
		strcpy_s(ip, strlen(_ip) + 1, _ip);
	}
	connectionData(unsigned long _ip) : timeOutTick(GetTickCount64()) {
		struct in_addr ipAddr;

		ipAddr.S_un.S_addr = htonl(_ip);
		inet_ntop(AF_INET, &(ipAddr.S_un.S_addr), ip, INET_ADDRSTRLEN);
	}
	char					ip[INET_ADDRSTRLEN];
	unsigned long long int	timeOutTick;
};

enum class jobID {
	newConnection,
	deleteConnection,
	packetProcess,
	dataBaseResult,
};

struct JobStruct
{
	JobStruct(jobID _id, sessionID _targetID, serializer* __serializer) : id(_id), targetID(_targetID), _serializer(__serializer) {
		//_serializer->incReferenceCounter();
	}
	JobStruct(jobID _id, sessionID _targetID) : id(_id), targetID(_targetID), _serializer() {
	}
	JobStruct() : id(jobID::newConnection), targetID(0), _serializer() {
	}

	~JobStruct() {
	}

	jobID id;
	sessionID targetID;
	serializer* _serializer;
};

class loginServer : public Network{
public:
	loginServer(unsigned long long int _timeoutThreshold = 10000);
	~loginServer();
	void DBInputOutputStart(
		const UINT8 _IOThreadSize = 100,
		const UINT8 _runningThreadSize = 1);

	virtual void OnNewConnect(unsigned long long sessionID);
	virtual void OnDisconnect(unsigned long long sessionID);
	virtual bool OnConnectionRequest(unsigned long ip, unsigned short port);
	virtual void OnRecv(unsigned long long sessionID, serializer* _packet);
	virtual void OnSend(unsigned long long sessionID, int sendsize);
	virtual void OnError(int errorcode, const char* msg) {

	};

	inline char getRandKey()
	{
		return rand() & 0x000000FF;
	}

	inline char getStatickey()
	{
		return 0x32; // 50
	}



	void update();




	void redisUpdate(unsigned long long _accuntNo, char* _sessionKey);
	bool sessionKeyCheckByDatabase(unsigned long long _accuntNo, char* _sessionKey);

private:
	unsigned long long int timeoutThreshold;
	int IOThreadSize;
	bool run;
	thread* timeoutThread;


	SRWLOCK containorLock;
	unordered_map<sessionID, connectionData*> connectionMap;
	ObjectPool<connectionData> connectionPool;
};