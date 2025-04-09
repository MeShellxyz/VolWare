#pragma once
#include <Windows.h>
#include <string>

class AutoStart {
public:
    static bool SetAutoStart(bool enable) {
        HKEY hKey;
        const wchar_t *keyPath =
            L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";

        if (RegOpenKeyExW(HKEY_CURRENT_USER, keyPath, 0, KEY_SET_VALUE,
                          &hKey) != ERROR_SUCCESS) {
            return false;
        }

        wchar_t execPath[MAX_PATH];
        GetModuleFileNameW(NULL, execPath, MAX_PATH);

        if (enable) {
            RegSetValueExW(hKey, L"VolWare", 0, REG_SZ, (BYTE *)execPath,
                           (wcslen(execPath) + 1) * sizeof(wchar_t));
        } else {
            RegDeleteValueW(hKey, L"VolWare");
        }

        RegCloseKey(hKey);
        return true;
    }
};