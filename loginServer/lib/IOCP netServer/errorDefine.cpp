#include "errorDefine.h"

std::unordered_map<int, std::string> errorCodeComment = {
	// �ý��� ����
	{	ERRORCODE_WSASTARTUP			,	"WSAStartup Error "},
	{	ERRORCODE_SOCKET				,	"Create socket Error "},
	{	ERRORCODE_BIND					,	"Socket bind Error "},
	{	ERRORCODE_LISTEN				,	"Socket listen Error "},
	{	ERRORCODE_ACCEPT				,	"Accept Error "},
	{	ERRORCODE_CREATEKERNELOBJECT	,	"Create kernel object Error "},
	{	ERRORCODE_CONNECT				,	"Socket Connect Error "},

	// 100���� - send / recv ����
	{	ERRORCODE_WSASEND				,	"WSAsend API Error "},
	{	ERRORCODE_WSARECV				,	"WSArecv API Error "},

	// ���� ���� ����
	{	ERRORCODE_SENDBUFFER			,	"Send buffer Error "},
	{	ERRORCODE_RECVBUFFER			,	"Recv buffer Error "},
	{	ERRORCODE_SESSIONARRAY			,	"Session array Error "},


	// ��ƿ ���� ����
	{	ERRORCODE_OBJECTPOOL			,	"ObjectPool Error"},

	//
	{	ERRORCODE_NOTDEFINE				,	""},
};