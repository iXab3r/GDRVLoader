// SCM-based driver service operations (signature-compatible)
#pragma once
#include "../global.h"
#include <windows.h>
#include <winsvc.h>
#include <memory>

static NTSTATUS LoadDriver(PWCHAR ServiceName)
{
    SC_HANDLE hSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (!hSCManager)
    {
        Log(L"OpenSCManager failed: %lu\n", GetLastError());
        return HRESULT_FROM_WIN32(GetLastError());
    }

    SC_HANDLE hService = OpenServiceW(hSCManager, ServiceName, SERVICE_START);
    if (!hService)
    {
        Log(L"OpenService failed: %lu\n", GetLastError());
        CloseServiceHandle(hSCManager);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    BOOL result = StartServiceW(hService, 0, nullptr);
    DWORD err = GetLastError();
    if (!result && err != ERROR_SERVICE_ALREADY_RUNNING)
    {
        Log(L"StartService failed: %lu\n", err);
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCManager);
        return HRESULT_FROM_WIN32(err);
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
    return STATUS_SUCCESS;
}

static NTSTATUS UnloadDriver(PWCHAR serviceName)
{
    SC_HANDLE hSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (!hSCManager)
    {
        Log(L"OpenSCManager failed: %lu\n", GetLastError());
        return HRESULT_FROM_WIN32(GetLastError());
    }

    SC_HANDLE hService = OpenServiceW(hSCManager, serviceName, SERVICE_STOP | DELETE);
    if (!hService)
    {
        Log(L"OpenService (for stop/delete) failed: %lu\n", GetLastError());
        CloseServiceHandle(hSCManager);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    SERVICE_STATUS status = {};
    ControlService(hService, SERVICE_CONTROL_STOP, &status); 

    BOOL deleted = DeleteService(hService);
    if (!deleted)
    {
        Log(L"DeleteService failed: %lu\n", GetLastError());
    }
    else
    {
        Log(L"Service %ls deleted successfully.\n", serviceName);
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
    return STATUS_SUCCESS;
}

static void FileNameToServiceName(PWCHAR ServiceName, PCWSTR FileName)
{
    // Strip path and extension to get the base file name
    PCWSTR lastSlash = wcsrchr(FileName, L'\\');
    PCWSTR baseName = lastSlash ? lastSlash + 1 : FileName;

    size_t i = 0;
    while (baseName[i] && baseName[i] != L'.' && i < MAX_PATH - 1)
    {
        ServiceName[i] = baseName[i];
        ++i;
    }
    ServiceName[i] = L'\0';
}

static NTSTATUS GerOrCreateDriverService(PWCHAR ServiceName, PWCHAR FileName)
{
    SC_HANDLE hSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);
    if (!hSCManager)
    {
        Log(L"OpenSCManager failed: %lu\n", GetLastError());
        return HRESULT_FROM_WIN32(GetLastError());
    }

    SC_HANDLE hService = CreateServiceW(
        hSCManager,
        ServiceName,
        ServiceName,
        SERVICE_ALL_ACCESS,
        SERVICE_KERNEL_DRIVER,
        SERVICE_DEMAND_START,
        SERVICE_ERROR_NORMAL,
        FileName,
        nullptr, nullptr, nullptr, nullptr, nullptr);

    DWORD err = GetLastError();
    if (!hService && err != ERROR_SERVICE_EXISTS)
    {
        Log(L"CreateService failed: %lu\n", err);
        CloseServiceHandle(hSCManager);
        return HRESULT_FROM_WIN32(err);
    }
    else if (!hService)
    {
        hService = OpenServiceW(hSCManager, ServiceName, SERVICE_ALL_ACCESS);
        if (!hService)
        {
            Log(L"OpenService fallback failed: %lu\n", GetLastError());
            CloseServiceHandle(hSCManager);
            return HRESULT_FROM_WIN32(GetLastError());
        }
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
    return STATUS_SUCCESS;
}

static std::unique_ptr<PrivilegeGuard> EnableLoadDriverPrivilege()
{
    ULONG SE_LOAD_DRIVER_PRIVILEGE = 10UL;
    return std::make_unique<PrivilegeGuard>(SE_LOAD_DRIVER_PRIVILEGE);
}