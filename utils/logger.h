#pragma once
typedef void (*LogCallback)(const wchar_t* message);

extern "C" __declspec(dllexport) void SetLogCallback(LogCallback callback);

void Log(const char* format, ...);
void Log(const wchar_t* format, ...);