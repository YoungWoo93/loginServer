#pragma comment(lib, "ws2_32")

#ifdef _DEBUG
#pragma comment(lib, "MemoryPoolD")

#else
#pragma comment(lib, "MemoryPool")

#endif

#include "session.h"

//#include <iostream>
//
//#include <WinSock2.h>
//#include <stack>
//#include <unordered_map>
//
//#include "RingBuffer/RingBuffer/RingBuffer.h"
//#include "MemoryPool/MemoryPool/MemoryPool.h"
//#include "MessageLogger/MessageLogger/MessageLogger.h"
//#include "crashDump/crashDump/crashDump.h"
//
#include "errorDefine.h"
#include "packet.h"
#include "network.h"
#include "monitoringTools/monitoringTools/messageLogger.h"
#include "monitoringTools/monitoringTools/performanceProfiler.h"
#include "monitoringTools/monitoringTools/resourceMonitor.h"
#pragma comment(lib, "monitoringTools\\x64\\Release\\monitoringTools")


/*/

sessionPtr::sessionPtr(session* _session, Network* _core) : ptr(_session), core(_core) {
	if (_session != nullptr)
	{
		if(_session->incrementIO() & 0x80000000 != 0){
			if (_session->decrementIO() == 0) {
				core->deleteSession(_session);
			}
		}

		ptr = nullptr;
	}
}
/*/
session::session()
	: ID(0), socket(0),
	IOcount(0), sendFlag(0), 
	sendBuffer(256), sendedBuffer(2048), recvBuffer(256) {

}
session::session(UINT64 id, SOCKET sock)
	: ID(id), socket(sock),
	IOcount(0), sendFlag(0),
	sendBuffer(256), sendedBuffer(2048), recvBuffer(256) {

}
void session::init(UINT64 id, SOCKET sock, UINT16 _port, ULONG _ip)
{
	ID = id;
	socket = sock;
	port = _port;
	ip = _ip;

	incrementIO();
	
	sendFlag = 0;
	cancelIOFlag = 0;

	InterlockedAnd((LONG*)&IOcount, (LONG)0x7fffffff);
}

UINT32 session::decrementIO() {
	return InterlockedDecrement(&IOcount);
}

UINT32 session::incrementIO() {
	return InterlockedIncrement(&IOcount);
}


/// <summary>
/// 
/// </summary>
/// <returns>
/// true : ������ ����
/// false : � �����ε� ���� ���� �Ұ���
/// </returns>



/*/
bool session::sendIO()
{
	if (hasCancelIOFlag())
		return true;

	if (InterlockedExchange16(&sendFlag, 1) == 1)
		return true;

	size_t sendBufferSize = sendBuffer.size();	// ������ ���� ����, size�� 1 �̻������� ������ �ȵ� �����ϵ�
	if (sendBufferSize == 0) { // ������ ��� ����, ������ �����Ȳ
		InterlockedExchange16(&sendFlag, 0);
		return true;
	}

	incrementIO();

	size_t packetCount = sendBufferSize;
	size_t WSAbufferCount = 0;
	WSABUF buffer[100];

	// size üũ�� ���� �� �ʿ䰡 ���� ���ɼ��� ����
	for (WSAbufferCount = 0; WSAbufferCount < packetCount; WSAbufferCount++)
	{
		serializer* temp = 0;
		if (sendBuffer.pop(&temp) == false)
		{
			WSAbufferCount--;
			sendBufferSize--;
			continue;
		}

		buffer[WSAbufferCount].buf = temp->getHeadPtr();
		buffer[WSAbufferCount].len = (ULONG)temp->size();

		sendedBuffer.push((char*)&temp, sizeof(serializer*));
	}

	DWORD temp1 = 0;

	memset(&sendOverlapped, 0, sizeof(OVERLAPPED));
	auto ret = WSASend(socket, buffer, (DWORD)WSAbufferCount, &temp1, 0, (LPWSAOVERLAPPED)&sendOverlapped, NULL);
	

	if (ret == SOCKET_ERROR) {
		auto errorCode = GetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			if (errorCode != 10054 && errorCode != 10053)
			{
				LOG(logLevel::Error, LO_TXT, "WSASend Error " + to_string(errorCode) + "\tby socket " + to_string(socket) + ", id " + to_string(ID));
				InterlockedExchange16(&sendFlag, 0);
				//int* temp = nullptr;
				//*temp = 10;
			}
			return false;
		}
	}

	if (hasCancelIOFlag()){
		CancelIoEx((HANDLE)socket, &sendOverlapped);
	}

	return true;
}
/*/

void session::sendIO()
{
	if (hasCancelIOFlag())
		return;

	if (InterlockedExchange16(&sendFlag, 1) == 1)
		return;

	size_t sendBufferSize = sendBuffer.size();	// ������ ���� ����, size�� 1 �̻������� ������ �ȵ� �����ϵ�
	if (sendBufferSize == 0) { // ������ ��� ����, ������ �����Ȳ
		InterlockedExchange16(&sendFlag, 0);
		return;
	}

	incrementIO();

	size_t packetCount = sendBufferSize;
	size_t WSAbufferCount = 0;
	WSABUF buffer[100];

	for (WSAbufferCount = 0; WSAbufferCount < packetCount; WSAbufferCount++)
	{
		serializer* temp = 0;
		if (sendBuffer.pop(&temp) == false)
		{
			WSAbufferCount--;
			sendBufferSize--;
			continue;
		}

		buffer[WSAbufferCount].buf = temp->getHeadPtr();
		buffer[WSAbufferCount].len = (ULONG)temp->size();

		sendedBuffer.push((char*)&temp, sizeof(serializer*));
	}

	DWORD temp1 = 0;

	memset(&sendOverlapped, 0, sizeof(OVERLAPPED));
	auto ret = WSASend(socket, buffer, (DWORD)WSAbufferCount, &temp1, 0, (LPWSAOVERLAPPED)&sendOverlapped, NULL);

	if (ret == SOCKET_ERROR) {
		auto errorCode = GetLastError();
		if (errorCode != WSA_IO_PENDING) // send�� ���� IO_PENDING�� Ÿ�� �ʴ´�.
		{
			if (errorCode != 10054 && errorCode != 10053)
			{
				LOG(logLevel::Error, LO_TXT, "WSASend Error " + to_string(errorCode) + "\tby socket " + to_string(socket) + ", id " + to_string(ID));
				InterlockedExchange16(&sendFlag, 0);
				//int* temp = nullptr;
				//*temp = 10;
			}

			cancelIORegist();

			if (CancelIoEx((HANDLE)socket, &(recvOverlapped)) == 0)
			{
				int errorCode = GetLastError();
				if (errorCode != ERROR_NOT_FOUND) {
					LOGOUT_EX(logLevel::Error, LO_TXT, "lib") << "disconnectReq recv CancelIoEx error " << errorCode << " index " << (USHORT)this->ID << LOGEND;
				}
			}

			if (decrementIO() == 0)
				core->deleteSession(this); // [����1] �̺κ�, �� ���ǿ��� ��Ʈ��ũ ������ ������ �˰��ְ� �ߴ°�?

			return;
		}

		if (hasCancelIOFlag()) 
		{
			if (CancelIoEx((HANDLE)socket, &sendOverlapped) == 0)
			{
				int errorCode = GetLastError();
				if (errorCode != ERROR_NOT_FOUND) {
					LOGOUT_EX(logLevel::Error, LO_TXT, "lib") << "disconnectReq send CancelIoEx error " << errorCode << " index " << (USHORT)this->ID << LOGEND;
				}
			}
		}
	}

	return;
}

/// <summary>
/// send �Ϸ� �������� ȣ��,sended ������ �ø���������� ��ȯ��. (sendFlag ��ȭ ����)
/// </summary>
/// <param name="transfer"> ���� �Ϸ����� ũ��, sendedó�� �� 0�� �Ǿ����</param>
/// <param name="sendPacketCount"> ���� �Ϸ� ��Ŷ ����, sended �������� Ȯ����</param>
/// <returns>
/// true : ������
/// false : ������ ���� (transfer zero, sended buffer corrupted, sendbuffer corrupted, WSAsend error ...)
/// </returns>
int session::sended(DWORD& transfer)
{
	DWORD temp = transfer;

	if (transfer <= 0)
		return -1;

	int sendedCount = 0;
	serializer* packetBuffer;

	while (transfer > 0)
	{
		int size = sendedBuffer.pop((char*)&packetBuffer, sizeof(serializer*));

		transfer -= (DWORD)packetBuffer->size();
		sendedCount++;

		int temp = packetBuffer->decReferenceCounter();
		if (temp == 0)
			serializerFree(packetBuffer);
	}

	if (transfer < 0)
		return -1;

	return sendedCount;
}


/// <summary>
/// sendBuffer�� ��Ŷ�� ������ ȣ��
/// </summary>
/// <returns>
/// true : ����
/// false : ������ ����
/// </returns>
bool session::collectSendPacket(packet p)
{

	if (sendBuffer.push(p.buffer))
		return true;
	else
		return false;
}

bool session::collectSendPacket(serializer* p)
{
	if (sendBuffer.push(p))
		return true;
	else
		return false;
}

/// <summary>
/// 
/// </summary>
/// <returns>
/// true : ������ ����
/// false : � �����ε� ���� ���� �Ұ���
/// </returns>
/// 
/*/ [����1] ����
bool session::recvIO()
{
	if (hasCancelIOFlag())
		return true;

	WSABUF buffer[2];
	int bufferCount = 1;
	size_t freeSize = recvBuffer.freeSize();

	buffer[0].buf = recvBuffer.tail();
	buffer[0].len = (ULONG)recvBuffer.DirectEnqueueSize();
	freeSize -= (ULONG)recvBuffer.DirectEnqueueSize();

	if (freeSize > 0)
	{
		buffer[1].buf = recvBuffer.bufferPtr();
		buffer[1].len = freeSize;
		bufferCount = 2;
	}


	DWORD temp1 = 0;
	DWORD temp2 = 0;

	memset(&recvOverlapped, 0, sizeof(OVERLAPPED));
	auto ret = WSARecv(socket, buffer, bufferCount, &temp1, &temp2, (LPWSAOVERLAPPED)&recvOverlapped, NULL);

	if (ret == SOCKET_ERROR) {
		auto errorCode = GetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			if (errorCode != 10054 && errorCode != 10053)
			{
				LOG(logLevel::Error, LO_TXT, "WSARecv Error " + to_string(errorCode) + "\tby socket " + to_string(socket) + ", id " + to_string(ID));
				//int* temp = nullptr;
				//*temp = 10;
			}
			
			return false;
		}
	}
	if (hasCancelIOFlag())
	{
		CancelIoEx((HANDLE)socket, &sendOverlapped);
	}
	return true;
}
/*/

void session::recvIO()
{
	if (hasCancelIOFlag())
		return;

	WSABUF buffer[2];
	int bufferCount = 1;
	size_t freeSize = recvBuffer.freeSize();

	buffer[0].buf = recvBuffer.tail();
	buffer[0].len = (ULONG)recvBuffer.DirectEnqueueSize();
	freeSize -= (ULONG)recvBuffer.DirectEnqueueSize();

	if (freeSize > 0)
	{
		buffer[1].buf = recvBuffer.bufferPtr();
		buffer[1].len = freeSize;
		bufferCount = 2;
	}

	DWORD temp1 = 0;
	DWORD temp2 = 0;

	incrementIO();

	memset(&recvOverlapped, 0, sizeof(OVERLAPPED));
	auto ret = WSARecv(socket, buffer, bufferCount, &temp1, &temp2, (LPWSAOVERLAPPED)&recvOverlapped, NULL);

	if (ret == SOCKET_ERROR) {
		auto errorCode = GetLastError(); // recv�� ���� IO_PENDING�� ź��

		if (errorCode != WSA_IO_PENDING)
		{
			if (errorCode != 10054 && errorCode != 10053)
			{
				LOG(logLevel::Error, LO_TXT, "WSARecv Error " + to_string(errorCode) + "\tby socket " + to_string(socket) + ", id " + to_string(ID));
			}

			cancelIORegist();

			if (CancelIoEx((HANDLE)socket, &sendOverlapped) == 0)
			{
				int errorCode = GetLastError();
				if (errorCode != ERROR_NOT_FOUND) {
					LOGOUT_EX(logLevel::Error, LO_TXT, "lib") << "disconnectReq recv CancelIoEx error " << errorCode << " index " << (USHORT)this->ID << LOGEND;
				}
			}

			if (decrementIO() == 0)
				core->deleteSession(this);

			return;
		}

		if (hasCancelIOFlag())
		{
			if (CancelIoEx((HANDLE)socket, &recvOverlapped) == 0)
			{
				int errorCode = GetLastError();
				if (errorCode != ERROR_NOT_FOUND) {
					LOGOUT_EX(logLevel::Error, LO_TXT, "lib") << "disconnectReq recv CancelIoEx error " << errorCode << " index " << (USHORT)this->ID << LOGEND;
				}
			}
		}
	}

	return;
}

/// <summary>
/// recv �Ϸ� �������� ȣ��, recvBuffer�� �а�, ���� ����ó������
/// </summary>
/// <param name="transfer"> �Ϸ����� ���� ũ��</param>
/// <returns>
/// true : ������
/// false : ������ ���� (recvBuffer full, recvBuffer flow, transfer zero ...)
/// </returns>
bool session::recved(DWORD& transfer)
{
	if (recvBuffer.freeSize() <= transfer || transfer == 0)
		return false;

	recvBuffer.MoveRear(transfer);

	return true;
}

/// <summary>
/// recv �Ϸ� �������� ȣ��, recv ���ۿ��� ������ �ϼ��� �����Ͱ� ������ �Ű������� ����
/// </summary>
/// <returns>
/// true : ������ ����
/// false : ������ ���� 
/// </returns>
bool session::recvedPacket(serializer* p)
{
	if (recvBuffer.size() >= sizeof(networkHeader))
	{
		networkHeader h;
		int size = recvBuffer.front((char*)&h, sizeof(networkHeader));
		if (size != sizeof(networkHeader))
			return false;

		if (recvBuffer.size() >= sizeof(networkHeader) + h.size)
		{
			recvBuffer.pop((char*)p->getBufferPtr(), sizeof(networkHeader) + h.size);
			p->moveRear(h.size);
			p->moveFront(-(int)sizeof(networkHeader));

			return true;
		}
		else
			return false;
	}

	return false;
}

/*/
bool session::recvedPacket(serializer* p)
{
	if (recvBuffer.size() >= sizeof(networkHeader))
	{
		networkHeader h;
		int size = recvBuffer.front((char*)&h, sizeof(networkHeader));
		if (size != sizeof(networkHeader))
			return false;

		if (recvBuffer.size() >= sizeof(networkHeader) + h.size)
		{
			recvBuffer.pop((char*)p.getPacketHeader(), sizeof(networkHeader) + h.size);
			p.buffer->moveRear(h.size);
			p.buffer->moveFront(-(int)sizeof(networkHeader));

			return true;
		}
		else
			return false;
	}

	return false;
}
/*/



/// <summary>
/// �ش� ������ ������ ������ ���������� ���� ���� ��û�� ���, �ش� �÷��� ����
/// </summary>
/// <returns>
/// ioCount�� �ش� �÷��׸� ������ ���� ��
/// </returns>
UINT32 session::cancelIORegist()
{
	return cancelIOFlag = 1;
}

/// <summary>
/// �ش� ������ ������ ������ ���������� ���� ���� ��û�� ���, �ش� ��û���� �� �÷��� ����
/// </summary>
/// <returns>
/// ioCount�� �ش� �÷��׸� ������ ���� ��
/// </returns>
UINT32 session::cancelIOUnregist()
{
	return cancelIOFlag = 0;
}

/// <summary>
/// �ش� ������ ������ ������ ���������� ���� ���� ��û�� �������� üũ
/// </summary>
/// <returns>
/// true : ���� ��û������
/// false : �ƴ�
/// </returns>
bool session::hasCancelIOFlag()
{
	return cancelIOFlag == 1;
}

SOCKET session::getSocket() {
	return socket;
}
UINT16 session::getPort() {
	return port;
}
ULONG session::getIp() {
	return ip;
}


bool session::sendOverlappedCheck(OVERLAPPED* o) {
	return o == &sendOverlapped;
}
bool session::recvOverlappedCheck(OVERLAPPED* o) {
	return o == &recvOverlapped;
}

void session::bufferClear()
{
	serializer* packetBuffer;

	while (sendBuffer.pop(&packetBuffer))
	{
		if (packetBuffer->decReferenceCounter() == 0)
			serializerFree(packetBuffer);
	}

	while (!sendedBuffer.empty())
	{
		sendedBuffer.pop((char*)&packetBuffer, sizeof(serializer*));
		if (packetBuffer->decReferenceCounter() == 0)
			serializerFree(packetBuffer);
	}

	recvBuffer.clear();
}
