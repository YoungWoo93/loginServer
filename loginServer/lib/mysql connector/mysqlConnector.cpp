#include "include/mysql.h"
#include "include/errmsg.h"

#include "mysqlConnector.h"

#pragma comment(lib, "lib/mysql connector/lib/libmysql")

__declspec(thread) MYSQL* mysqlConnector::mysqlConnect = nullptr;
__declspec(thread) MYSQL* mysqlConnector::conn = nullptr;

mysqlConnector::mysqlConnector()
{

}
mysqlConnector::~mysqlConnector()
{

}


void mysqlConnector::query(const char* _queryStr, dbResult& _result)
{
	do
	{
		while (mysqlConnect == nullptr)
		{
			init();

			mysqlConnect = mysql_real_connect(conn, "127.0.0.1", "root", "1!qQ2@wW3#eE4$rR", "accountDB", 3306, (char*)NULL, 0);

			if (mysqlConnect == nullptr)
			{
				fprintf(stderr, "Mysql connection error : %s", mysql_error(conn));
			}
		}

		if (mysql_query(mysqlConnect, _queryStr) == 0)
			break;

		fprintf(stderr, "Mysql query error : %s", mysql_error(conn));
		mysql_close(mysqlConnect);

	} while (true);



	int index = 0;
	MYSQL_ROW sql_row;
	auto sql_result = mysql_store_result(mysqlConnect);

	while ((sql_row = mysql_fetch_row(sql_result)) != NULL)
	{
		for (auto& it : _result.result)
		{
			if (sql_row[index] == nullptr)
				it.second = "";
			else
				it.second = string(sql_row[index++]);
		}
	}
	mysql_free_result(sql_result);
}

void mysqlConnector::init()
{
	if (conn == nullptr)
		conn = new MYSQL;
	else
		mysql_close(conn);

	if (mysql_init(conn) == nullptr)
	{
		int erno = mysql_errno(conn);
		fprintf(stderr, "Mysql Init error : %s", mysql_error(conn));
	}
}