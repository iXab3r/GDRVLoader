#pragma once
#include <Windows.h>

class TempFileGuard
{
public:
	explicit TempFileGuard(const wchar_t* path)
		: _path(path)
	{}

	~TempFileGuard()
	{
		if (!_path.empty())
		{
			if (!DeleteFileW(_path.c_str()))
			{
				DWORD err = GetLastError();
				Log(L"TempFileGuard failed to delete %ls: %lu\n", _path.c_str(), err);
			}
			else
			{
				Log(L"TempFileGuard deleted %ls successfully\n", _path.c_str());
			}
		}
	}

private:
	std::wstring _path;
};