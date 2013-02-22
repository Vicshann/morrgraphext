#include "LOG.h"
#include "Modules/Modules.h"
#include <stdio.h>

HANDLE LOG::handle= INVALID_HANDLE_VALUE;

LOG::LOG() { }

LOG::~LOG() { close(); }

bool LOG::open(const char* filename) {
	Logger::LogProc = Logger::LogToFile;
	Logger::BinProc = Logger::LogBinToFile;
	return Logger::CreateLogFile((LPSTR)filename);		
}

int LOG::log(const char* fmt, ...) {
	char* buf = new char[4096] ();
	int result= 0;
	va_list args;
	va_start(args,fmt);
	if(fmt) {
		result= vsprintf(buf,fmt,args);
	} else {
		result= 4;
		strcpy(buf,"LOG::log(null)\r\n");
	}
	va_end(args);
	_LogText(buf);
	delete[] buf;
	return result;
}

int LOG::logline(const char* fmt, ...) {
	char* buf = new char[4096] ();
	int result= 0;
	va_list args;
	va_start(args,fmt);
	if(fmt) {
		result= vsprintf(buf,fmt,args);
		strcat_s(buf, 4096, "\r\n");
	} else {
		result= 4;
		strcpy(buf,"LOG::log(null)\r\n");
	}
	va_end(args);
	_LogText(buf);
	delete[] buf;
	return result;
}

int LOG::logbinary(void* addr, int size) {
	_LogBinary(addr, size);
	return size;
}

void LOG::close() {
	
}
