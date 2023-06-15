#include <stdio.h>
#include <string>


#include "include/mysql.h"
#include "include/errmsg.h"

#pragma comment(lib, "lib/mysql connector/lib/libmysql")

#include "dbQuery.h"

using namespace std;

void testFunc(unsigned long long int _accountNo, dbResult& _result)
{
	MYSQL conn;
	MYSQL* connection = NULL;
	MYSQL_RES* sql_result;
	MYSQL_ROW sql_row;
	int query_stat;
	int erno = 0;
	// �ʱ�ȭ
	auto ret = mysql_init(&conn);
	erno = mysql_errno(&conn);
	// DB ����

	connection = mysql_real_connect(&conn, "127.0.0.1", "root", "1!qQ2@wW3#eE4$rR", "accountDB", 3306, (char*)NULL, 0);
	if (connection == NULL)
	{
		fprintf(stderr, "Mysql connection error : %s", mysql_error(&conn));
		return;
	}








	// Select ������
	string query("SELECT * FROM sessionKey where accountNo = ");
	query += to_string(_accountNo);	// From ���� DB�� �����ϴ� ���̺� ������ �����ϼ���
	query_stat = mysql_query(connection, query.c_str());
	if (query_stat != 0)
	{
		printf("Mysql query error : %s", mysql_error(&conn));
		return;
	}

	// ������
	sql_result = mysql_store_result(connection);		// ��� ��ü�� �̸� ������
	//	sql_result=mysql_use_result(connection);		// fetch_row ȣ��� 1���� ������
	int n = mysql_num_fields(sql_result);
	while ((sql_row = mysql_fetch_row(sql_result)) != NULL)
	{
		int index = 0;
		for (auto &it : _result.result)
		{
			if (sql_row[index] == nullptr)
				it.second = "";
			else
				it.second = string(sql_row[index++]);
		}
		//for (int i = 0; i < n; i++)
		//{
		//	printf("%s\t", sql_row[i]);
		//	strcpy_s(_result[i], strlen(sql_row[i]), sql_row[i]);
		//}
	}
	mysql_free_result(sql_result);

	// DB ����ݱ�








	mysql_close(connection);




	//	int a = mysql_insert_id(connection);


	//	query_stat = mysql_set_server_option(connection, MYSQL_OPTION_MULTI_STATEMENTS_ON);
	//	mysql_next_result(connection);				// ��Ƽ���� ���� ���� ��� ���
	//	sql_result=mysql_store_result(connection);	// next_result �� ��� ����


}