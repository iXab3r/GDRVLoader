#pragma once
#include "../global.h"
#include <shlwapi.h>

static NTSTATUS LoadDriver(PWCHAR ServiceName)
{
    UNICODE_STRING ServiceNameUcs;
    RtlInitUnicodeString(&ServiceNameUcs, ServiceName);

    Log("Loading the driver via NtLoadDriver(%ls)\n", ServiceNameUcs.Buffer);
    return NtLoadDriver(&ServiceNameUcs);
}

static NTSTATUS UnloadDriver(PWCHAR ServiceName)
{
    UNICODE_STRING ServiceNameUcs;
    RtlInitUnicodeString(&ServiceNameUcs, ServiceName);

    Log("Unloading the driver via NtLoadDriver(%ls)\n", ServiceNameUcs.Buffer);
    return NtUnloadDriver(&ServiceNameUcs);
}

static void DeleteService(PWCHAR ServiceName)
{
    // TODO: shlwapi.dll? holy fuck this is horrible
    SHDeleteKeyW(HKEY_LOCAL_MACHINE, ServiceName + sizeof(NT_MACHINE) / sizeof(WCHAR) - 1);
}

static int ConvertToNtPath(PWCHAR Dst, size_t DstLen, PCWSTR Src)
{
    const wchar_t* prefix = L"\\??\\";
    size_t prefixLen = wcslen(prefix);
    size_t srcLen = wcslen(Src);

    if (prefixLen + srcLen + 1 > DstLen)
        return 0;

    wcscpy_s(Dst, DstLen, prefix);
    wcscat_s(Dst, DstLen, Src);

    return static_cast<int>((prefixLen + srcLen + 1) * sizeof(wchar_t));
}

static void FileNameToServiceName(PWCHAR ServiceName, PWCHAR FileName)
{
    int p = sizeof(SVC_BASE) / sizeof(WCHAR) - 1;
    wcscpy_s(ServiceName, sizeof(SVC_BASE) / sizeof(WCHAR), SVC_BASE);
    for (PWCHAR i = FileName; *i; ++i)
    {
        if (*i == L'\\')
            FileName = i + 1;
    }
    while (*FileName != L'\0' && *FileName != L'.')
        ServiceName[p++] = *FileName++;
    ServiceName[p] = L'\0';
}

static NTSTATUS GerOrCreateDriverService(PWCHAR ServiceName, PWCHAR FileName)
{
    FileNameToServiceName(ServiceName, FileName);
    NTSTATUS Status = RtlCreateRegistryKey(RTL_REGISTRY_ABSOLUTE, ServiceName);
    if (!NT_SUCCESS(Status))
        return Status;

    WCHAR NtPath[MAX_PATH];

    int servicePathLength = ConvertToNtPath(NtPath, MAX_PATH, FileName);
    if (servicePathLength == 0)
    {
        return STATUS_BUFFER_TOO_SMALL;
    }

    Status = RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                                   ServiceName,
                                   L"ImagePath",
                                   REG_EXPAND_SZ,
                                   NtPath,
                                   servicePathLength);
    if (!NT_SUCCESS(Status))
        return Status;

    ULONG ServiceType = SERVICE_KERNEL_DRIVER;
    Status = RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                                   ServiceName,
                                   L"Type",
                                   REG_DWORD,
                                   &ServiceType,
                                   sizeof(ServiceType));
    return Status;
}

static std::unique_ptr<PrivilegeGuard> EnableLoadDriverPrivilege()
{
    ULONG SE_LOAD_DRIVER_PRIVILEGE = 10UL;
    return std::make_unique<PrivilegeGuard>(SE_LOAD_DRIVER_PRIVILEGE);
}
