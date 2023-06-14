#pragma once


#pragma comment(lib, "ws2_32")

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stack>
#include <unordered_map>

#include "ringBuffer.h"
#include "LockFreeQueue.h"

#include "errorDefine.h"
#include "packet.h"


struct overlapped;
class sessionPtr;
class session;
class Network;



#include <vector>

class session {
public:
	session();
	session(UINT64 id, SOCKET sock);
	void init(UINT64 id, SOCKET sock, UINT16 _port, ULONG _ip);

	UINT32 decrementIO();
	UINT32 incrementIO();

	/// <summary>
	/// 
	/// </summary>
	/// <returns>
	/// true : ������ ����
	/// false : � �����ε� ���� ���� �Ұ���
	/// </returns>
	/// 
	//[����1] ����
	//bool sendIO();
	void sendIO();

	/// <summary>
	/// send �Ϸ� �������� ȣ��, sendflag�� �����ϰ� sended ������ �ø���������� ��ȯ��. ���� sendBuffer�� ���� �����ִٸ� ���� send�� ��û�� (sendFlag ��ȭ ����)
	/// </summary>
	/// <param name="transfer"> ���� �Ϸ����� ũ��, sendedó�� �� 0�� �Ǿ����</param>
	/// <param name="sendPacketCount"> ���� �Ϸ� ��Ŷ ����, sended �������� Ȯ����</param>
	/// <returns>
	/// true : ������
	/// false : ������ ���� (transfer zero, sended buffer corrupted, sendbuffer corrupted, WSAsend error ...)
	/// </returns>
	int sended(DWORD& transfer);


	/// <summary>
	/// sendBuffer�� ��Ŷ�� ������ ȣ��
	/// </summary>
	/// <returns>
	/// true : ����
	/// false : ������ ����
	/// </returns>
	bool collectSendPacket(packet p);
	bool collectSendPacket(serializer* p);

	/// <summary>
	/// 
	/// </summary>
	/// <returns>
	/// true : ������ ����
	/// false : � �����ε� ���� ���� �Ұ���
	/// </returns>
	/// 
	//[����1] ����
	//bool recvIO();
	void recvIO();


	/// <summary>
	/// recv �Ϸ� �������� ȣ��, recvBuffer�� �а�, ���� ����ó������
	/// </summary>
	/// <param name="transfer"> �Ϸ����� ���� ũ��</param>
	/// <returns>
	/// true : ������
	/// false : ������ ���� (recvBuffer full, recvBuffer flow, transfer zero ...)
	/// </returns>
	bool recved(DWORD& transfer);

	/// <summary>
	/// recv �Ϸ� �������� ȣ��, recv ���ۿ��� ������ �ϼ��� �����Ͱ� ������ �Ű������� ����
	/// </summary>
	/// <returns>
	/// true : ������ ����
	/// false : ������ ���� 
	/// </returns>
	bool recvedPacket(serializer* p);

	/// <summary>
	/// �ش� ������ ������ ������ ���������� ���� ���� ��û�� ���, �ش� ��û ���
	/// </summary>
	/// <returns>
	/// ioCount�� �ش� �÷��׸� ���� ���� ��
	/// </returns>
	UINT32 cancelIORegist();

	/// <summary>
	/// �ش� ������ ������ ������ ���������� ���� ���� ��û�� ���, �ش� ��û���� �� �÷��� ����
	/// </summary>
	/// <returns>
	/// ioCount�� �ش� �÷��׸� ������ ���� ��
	/// </returns>
	UINT32 cancelIOUnregist();

	/// <summary>
	/// �ش� ������ ������ ������ ���������� ���� ���� ��û�� �������� üũ
	/// </summary>
	/// <returns>
	/// true : ���� ��û������
	/// false : �ƴ�
	/// </returns>
	bool hasCancelIOFlag();

	SOCKET getSocket();
	UINT16 getPort();
	ULONG getIp();

	bool sendOverlappedCheck(OVERLAPPED* o);
	bool recvOverlappedCheck(OVERLAPPED* o);

	void bufferClear();

public:
	unsigned long long int ID;			//8		- �����ܿ��� �ַ� ���

public:
	Network* core;
	SOCKET socket;						//8
	UINT16 port;						//2
	ULONG ip;							//4
	short sendFlag;						//2		- interlock
	UINT32 IOcount;						//4		- interlock
	short cancelIOFlag;				//2
	LockFreeQueue sendBuffer; //112

	OVERLAPPED sendOverlapped;			//32	- send-recv �Ϸ�����, send-recv IO ���� ���
	OVERLAPPED recvOverlapped;			//32	- send-recv �Ϸ�����, send-recv IO ���� ���

	ringBuffer sendedBuffer;			//32
	ringBuffer recvBuffer;				//32

	UINT64 releaseFlag;
};
//	total 288

//0	 1	 2	 3	 4	 5	 6	 7	 8	 9	 10	 11	 12	 13	 14	 15	 16	 17	 18	 19	 20	 21	 22	 23	 24	 25	 26	 27	 28	 29	 30	 31	 32	 33	 34	 35	 36	 37	 38	 39	 40	 41	 42	 43	 44	 45	 46	 47	 48	 49	 50	 51	 52	 53	 54	 55	 56	 57	 58	 59	 60	 61	 62	 63
//	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	|	
//		ID	(0~7) 8/8			|	socket (8~15) 8/8			| port	| sendF |  IOcount		| -���� ������ ����, ���� sendBuffer																											|	
//	send overlapped	(0 ~ 31) 32/8																								|	recv overlapped (32 ~  63) 32/8
//	sended buffer	(0 ~ 31) 32/8																								|	recv buffer (32 ~  63) 32/8
// 
//
//
//
//



/*/
class sessionPtr {
	friend class Network;
public:
	sessionPtr(session* _session, Network* _core) :ptr(_session), core(_core) {
		int i = 0;
		i++;
	}

	~sessionPtr() {
		if (ptr == nullptr)
			return;

		if (ptr->decrementIO() == 0)
			core->deleteSession(ptr);
	}

	sessionPtr(const sessionPtr& s) {
		if (s.ptr != nullptr)
			s.ptr->incrementIO();

		ptr = s.ptr;
		core = s.core;
	}

	sessionPtr(sessionPtr&& s) {
		ptr = s.ptr;
		core = s.core;

		s.ptr = nullptr;
	}

	sessionPtr& operator=(sessionPtr&& s) {
		core = s.core;

		if (ptr == s.ptr)
			return *this;

		if (ptr != nullptr) {
			if (ptr->decrementIO() == 0)
				core->deleteSession(ptr);
		}

		ptr = s.ptr;
		s.ptr = nullptr;

		return *this;
	}

	sessionPtr& operator=(const sessionPtr& s) {
		core = s.core;

		if (ptr == s.ptr)
			return *this;

		if (ptr != nullptr) {
			if (ptr->decrementIO() == 0)
				core->deleteSession(ptr);
		}
		if (s.ptr != nullptr)
			s.ptr->incrementIO();

		ptr = s.ptr;

		return *this;
	}

	



	inline bool valid() {
		return ptr != nullptr;
	}
	inline bool collectSendPacket(packet& p) {
		return ptr->collectSendPacket(p);
	}
	inline bool sendIO() {
		ptr->sendIO();
		return true;
	}
	inline UINT32 decrementIO() {
		return ptr->decrementIO();
	}
	inline UINT32 incrementIO() {
		return ptr->incrementIO();
	}

	UINT32 ID;

private:
	
	//sessionPtr(session* _session, Network* _core);
	session* ptr;
	Network* core;
};

/*/