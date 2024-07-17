#pragma once
#include <string>
#include <Windows.h>

namespace WinReg
{
    void write(const DWORD type, const HKEY hRootKey, LPCTSTR subKey, LPCTSTR ValueName, const DWORD64& data);
    void write(const DWORD type, const HKEY hRootKey, LPCTSTR subKey, LPCTSTR ValueName, const std::wstring& data);
    void read(const DWORD type, const HKEY hRootKey, LPCTSTR subkey, LPCTSTR ValueName, DWORD64& data, bool& found);
    void read(const DWORD type, const HKEY hRootKey, LPCTSTR subkey, LPCTSTR ValueName, std::wstring& data, bool& found);
};