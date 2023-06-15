#pragma once
#include <string>
#include <vector>
#include <unordered_map>

#include "include/mysql.h"

using namespace std;

class dbResult
{
public:
	dbResult(vector<string>& fildNames) {
		for (string name : fildNames)
		{
			result[name] = string();
		}
	}
	unordered_map<string, string> result;
};

class mysqlConnector
{
public:
	mysqlConnector();
	~mysqlConnector();

	mysqlConnector(const mysqlConnector& c) = delete;
	mysqlConnector& operator=(const mysqlConnector& c) = delete;


	static void query(const char* _queryStr, dbResult& _result);
	static void init();

public:
	static __declspec(thread) MYSQL* conn;
	static __declspec(thread) MYSQL* mysqlConnect;
};