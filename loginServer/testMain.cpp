#include <iostream>
#include <clocale>
#include <io.h>
#include <fcntl.h>

#include <thread>
#include <queue>

#include "loginServer.h"


#include "monitoringTools/monitoringTools/messageLogger.h"
#include "monitoringTools/monitoringTools/performanceProfiler.h"
#include "monitoringTools/monitoringTools/resourceMonitor.h"
#include "monitoringTools/monitoringTools/dump.h"

#pragma comment(lib, "monitoringTools\\x64\\Release\\monitoringTools")

void main()
{
	setlocale(LC_ALL, "");
	dump d;

	loginServer s;

	s.start(12002, 10000, 10, 3);

	for (;;)
	{
		cout << "\taccept TPS\t: " << s.getAcceptTPS() << "(total " << s.getAcceptTotal() << ")" << endl;
		cout << "\tsessionCount\t: " << (long long int)s.getSessionCount() << endl;

		cout << endl;
		Sleep(1000);
	}
}