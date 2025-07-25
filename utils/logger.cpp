#include "logger.h"
#include "../global.h"

static LogCallback g_LogCallback;

extern "C" __declspec(dllexport) void SetLogCallback(LogCallback callback)
{
	g_LogCallback = callback;
}

void Log(const char* format, ...)
{
	char narrowBuf[1024];
	wchar_t wideBuf[1024];

	va_list args;
	va_start(args, format);
	vsnprintf(narrowBuf, sizeof(narrowBuf), format, args);
	va_end(args);

	// Safely convert to wide (assumes current locale; can be UTF-8 if configured)
	size_t converted = 0;
	mbstowcs_s(&converted, wideBuf, narrowBuf, _TRUNCATE);

	if (g_LogCallback)
		g_LogCallback(wideBuf);
	else
		printf("%s", narrowBuf); // still use narrow fallback
}

void Log(const wchar_t* format, ...)
{
	wchar_t buffer[1024];

	va_list args;
	va_start(args, format);
	_vsnwprintf_s(buffer, _countof(buffer), _TRUNCATE, format, args);
	va_end(args);

	if (g_LogCallback)
		g_LogCallback(buffer);
	else
		wprintf(L"%s", buffer);
}