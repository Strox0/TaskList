#include <Windows.h>
#include <TlHelp32.h>
#include <fstream>
#include <vector>
#include <filesystem>
#include <algorithm>

#include "Task.h"
#include "Registry.h"

#ifdef _DEBUG
#define PRINT_DEBUG(x) printf(x)
#else
#define PRINT_DEBUG(x)
#endif

#define TASKLIST_PROCESS_NAME L"ZenTask.exe"

bool IsProcessRunning(const wchar_t* proc_name)
{
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	PROCESSENTRY32W pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32W);

	if (Process32FirstW(hProcessSnap, &pe32))
	{
		do
		{
			if (wcscmp(pe32.szExeFile, proc_name) == 0)
			{
				PRINT_DEBUG("Process found\n");
				CloseHandle(hProcessSnap);
				return true;
			}

		} while (Process32NextW(hProcessSnap, &pe32));
	}

	CloseHandle(hProcessSnap);
	PRINT_DEBUG("Process not found\n");
	return false;
}

#ifdef _DEBUG
int main() {
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
#endif
	{
		bool found = false;
		DWORD64 start_background_srvc = 0;
		WinReg::read(REG_QWORD, HKEY_CURRENT_USER, L"Software\\ZenTask", L"BkgdSrvc", start_background_srvc, found);
		if (found && start_background_srvc == 0)
			return 0;
	}
	PRINT_DEBUG("Starting background service\n");
	{
		WCHAR app_path[MAX_PATH + 2] = { 0 };
		app_path[0] = L'"';
		DWORD p_size = GetModuleFileNameW(NULL, &app_path[1], MAX_PATH);
		app_path[p_size + 1] = L'"';

		std::wstring val;
		bool found = false;
		WinReg::read(REG_SZ, HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", L"ZenTask_BkgdSrvc", val, found);

		if (!found || val != app_path)
			WinReg::write(REG_SZ, HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", L"ZenTask_BkgdSrvc", app_path);
	}
	
	std::wstring file_path;
	bool found = false;
	WinReg::read(REG_SZ, HKEY_CURRENT_USER, L"Software\\ZenTask", L"AppPath", file_path, found);
	
	if (!found)
		return 1;
	PRINT_DEBUG("Retrived application path\n");
	std::ifstream file;
	std::vector<Task> tasks;
	std::filesystem::path dir_path;
	{
		char* localAppDataPath = nullptr;
		size_t size = 0;
		errno_t err = _dupenv_s(&localAppDataPath, &size, "LOCALAPPDATA");
		if (err != 0 || localAppDataPath == nullptr)
			return 1;

		dir_path = std::filesystem::path(localAppDataPath);
		free(localAppDataPath);

		dir_path /= "ZenTask";
	}
	PRINT_DEBUG("Retrived directory path\n");
	Sleep(10000);

	while (true)
	{
		if (!IsProcessRunning(TASKLIST_PROCESS_NAME))
		{
			if (!std::filesystem::exists(dir_path))
				continue;

			if (!file.is_open())
			{
				PRINT_DEBUG("Opening tasks file\n");
				file.open(dir_path / "tasks.bin");
				tasks.clear();

				while (file)
				{
					Task task = Deserialize(file);
					if (task.GetName().empty())
						break;
					PRINT_DEBUG(std::string(task.GetName() + " task found\n").c_str());
					tasks.push_back(task);
				
				}
				std::sort(tasks.begin(), tasks.end(), [](const Task& a, const Task& b) { return a.GetDueDate() < b.GetDueDate(); });
			}

			for (auto&  task : tasks)
			{
				if (task.IsCompleted())
					continue;

				if (task.GetDueDate() < std::chrono::system_clock::now() + std::chrono::minutes(15))
				{
					PRINT_DEBUG(std::string(task.GetName() + " task is due\n").c_str());

					STARTUPINFO si = { 0 };
					si.cb = sizeof(si);
					PROCESS_INFORMATION pi = { 0 };
					if (CreateProcessW(NULL, file_path.data(), NULL, NULL, FALSE, CREATE_UNICODE_ENVIRONMENT | CREATE_NEW_PROCESS_GROUP, NULL, NULL, &si, &pi))
					{
						file.close();
						tasks.clear();
						WaitForSingleObject(pi.hProcess, INFINITE);

						CloseHandle(pi.hProcess);
						CloseHandle(pi.hThread);
						break;
					}
					else
					{
						DWORD error = GetLastError();
						MessageBoxA(NULL, std::to_string(error).c_str(), "Error ZT-BkgdSrvc", MB_OK | MB_ICONERROR);
					}
				}
			}
		}
		else
		{
			if (file.is_open())
			{
				file.close();
				tasks.clear();
			}
		}

		Sleep(100);
	}

	return 0;
}