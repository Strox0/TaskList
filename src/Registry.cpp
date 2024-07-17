#include "Registry.h"
#include <winreg.h>

void WinReg::write(const DWORD type, const HKEY hRootKey, LPCTSTR subKey, LPCTSTR ValueName, const DWORD64& data)
{
	HKEY hKey;
	LONG nError = RegOpenKeyEx(hRootKey, subKey, NULL, KEY_ALL_ACCESS, &hKey);

	if (nError == ERROR_FILE_NOT_FOUND)
	{
		nError = RegCreateKeyEx(hRootKey, subKey, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	}

	nError = RegSetValueExW(hKey, ValueName, NULL, type, (LPBYTE)&data, sizeof(DWORD64));
	RegCloseKey(hKey);
}

void WinReg::write(const DWORD type, const HKEY hRootKey, LPCTSTR subKey, LPCTSTR ValueName, const std::wstring& data)
{
	HKEY hKey;
	LONG nError = RegOpenKeyExW(hRootKey, subKey, NULL, KEY_ALL_ACCESS, &hKey);

	if (nError == ERROR_FILE_NOT_FOUND)
	{
		nError = RegCreateKeyEx(hRootKey, subKey, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	}

	nError = RegSetValueExW(hKey, ValueName, NULL, type, (LPBYTE)(data.c_str()), data.size()*sizeof(wchar_t));
	RegCloseKey(hKey);
}

void WinReg::read(const DWORD type, const HKEY hRootKey, LPCTSTR subkey, LPCTSTR ValueName, DWORD64& data, bool& found)
{
	HKEY key;
	DWORD64 _value(0);
	DWORD value_length(sizeof(DWORD64));
	LRESULT lResult = RegOpenKeyEx(hRootKey, subkey, 0, KEY_ALL_ACCESS, &key);
	found = true;

	if (lResult == ERROR_SUCCESS) 
		RegQueryValueEx(key, ValueName, NULL, (LPDWORD)&type, (LPBYTE)&_value, &value_length);
	else if (lResult == ERROR_FILE_NOT_FOUND) 
		found = false;

	RegCloseKey(key);
	data = _value;
}

void WinReg::read(const DWORD type, const HKEY hRootKey, LPCTSTR subkey, LPCTSTR ValueName, std::wstring& data, bool& found)
{
	HKEY key;
	WCHAR _value[5504]{0};
	DWORD vlenght = 5503 * sizeof(WCHAR);
	LRESULT lResult = RegOpenKeyEx(hRootKey, subkey, 0, KEY_ALL_ACCESS, &key);
	found = true;

	if (lResult == ERROR_SUCCESS) 
		RegQueryValueExW(key, ValueName, NULL, (LPDWORD)&type, (LPBYTE)&_value, &vlenght);
	else if (lResult == ERROR_FILE_NOT_FOUND) 
		found = false;

	RegCloseKey(key);
	data = _value;
}