#include "packet.h"



ObjectPool<serializer> serializerPool(10, 100);
SRWLOCK serializerPoolLock;

std::pair<size_t, size_t> packetPoolMemoryCheck()
{
	std::pair<size_t, size_t> ret;
	AcquireSRWLockExclusive(&serializerPoolLock);
	ret.first = serializerPool.GetCapacityCount();
	ret.second = serializerPool.GetUseCount();
	ReleaseSRWLockExclusive(&serializerPoolLock);

	return ret;
}


void serializerFree(serializer* s)
{
	//AcquireSRWLockExclusive(&serializerPoolLock);
	serializerPool.Free(s);
	//ReleaseSRWLockExclusive(&serializerPoolLock);
}


serializer* serializerAlloc()
{
	//AcquireSRWLockExclusive(&serializerPoolLock);
	auto ret = serializerPool.Alloc();
	//ReleaseSRWLockExclusive(&serializerPoolLock);
	ret->clear(); //���⼭ ref count�� 0���� �ʱ�ȭ������ �ʴ´�, �� ��ȯ�Ҷ� 0���� Ȯ���ϰ� ��ȯ�ؾ��Ѵ�. (���� �ؾ� �� ���� ����)

	return ret;
}

