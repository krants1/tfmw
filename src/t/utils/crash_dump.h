// MVC Only
#include <dbghelp.h>
#include <ctime>

#pragma comment(lib, "dbghelp.lib")

LONG WINAPI UnhandledException(LPEXCEPTION_POINTERS exceptionInfo) {
	char timeStr[255];
	time_t t = time(NULL);

	struct tm timeinfo;
	localtime_s(&timeinfo, &t);
	struct tm *p_timeinfo = &timeinfo;

	strftime(timeStr, 255, "CrashDump_%d_%m_%Y_%H_%M_%S.dmp", p_timeinfo);
	HANDLE hFile = CreateFile(
		timeStr,
		GENERIC_WRITE | GENERIC_READ,
		0,
		NULL,
		CREATE_ALWAYS,
		0,
		NULL
	);

	if (hFile != NULL) {
		MINIDUMP_EXCEPTION_INFORMATION info =
		{
				GetCurrentThreadId(),
				exceptionInfo,
				TRUE
		};

		MiniDumpWriteDump(
			GetCurrentProcess(),
			GetCurrentProcessId(),
			hFile,
			MiniDumpWithIndirectlyReferencedMemory,
			&info,
			NULL,
			NULL
		);

		CloseHandle(hFile);
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

/*
LONG WINAPI UnhandledException(LPEXCEPTION_POINTERS exceptionInfo) {
	std::ofstream f;
	const char *exceptionString = NULL;
#define EXCEPTION_CASE(code) case code: exceptionString = #code; break
	switch (exceptionInfo->ExceptionRecord->ExceptionCode) {
		EXCEPTION_CASE(EXCEPTION_ACCESS_VIOLATION);
		EXCEPTION_CASE(EXCEPTION_DATATYPE_MISALIGNMENT);
		EXCEPTION_CASE(EXCEPTION_BREAKPOINT);
		EXCEPTION_CASE(EXCEPTION_SINGLE_STEP);
		EXCEPTION_CASE(EXCEPTION_ARRAY_BOUNDS_EXCEEDED);
		EXCEPTION_CASE(EXCEPTION_FLT_DENORMAL_OPERAND);
		EXCEPTION_CASE(EXCEPTION_INT_DIVIDE_BY_ZERO);
	case 0xE06D7363:
		exceptionString = "C++ exception (using throw)";
		break;
	default:
		exceptionString = "Unknown exception";
		break;
	}

	//HMODULE codeBase = GetModuleHandle(NULL);
	f.open("CrashDumb.txt", std::ios::out | std::ios::app | std::ios::ate);
	f << "=====================================" << std::endl;
	f << "ExceptionName: " << exceptionString << std::endl;
	f << "ExceptionCode: " << std::hex << exceptionInfo->ExceptionRecord->ExceptionCode << std::endl;
	f << "ExceptionAddress: " << std::hex << exceptionInfo->ExceptionRecord->ExceptionAddress << std::endl;
	f << "ExceptionFlags: " << std::hex << exceptionInfo->ExceptionRecord->ExceptionFlags << std::endl;
	for (DWORD i = 0; i < exceptionInfo->ExceptionRecord->NumberParameters; i++)
		f << "\tParam" << i + 1 << ": " << exceptionInfo->ExceptionRecord->ExceptionInformation[i] << std::endl;
	//f << "ExceptionOffset: " << std::hex << pExceptionInfo->ExceptionRecord->ExceptionAddress - codeBase << std::endl;

	//f << "CodeBase: " << std::hex << codeBase << std::endl;
	f.close();

	return EXCEPTION_CONTINUE_SEARCH;
}
*/

