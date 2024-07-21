#include "WindowMgr.h"
#include "Registry.h"
#include <filesystem>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
	WCHAR app_path[MAX_PATH + 2] = { 0 };
	app_path[0] = L'"';
	DWORD p_size = GetModuleFileNameW(NULL, &app_path[1], MAX_PATH);
	app_path[p_size + 1] = L'"';

	WinReg::write(REG_SZ, HKEY_CURRENT_USER, L"Software\\ZenTask", L"AppPath", app_path);

	DWORD64 should_start = 0;
	bool f = false;
	bool startup = false;
	WinReg::read(REG_QWORD, HKEY_CURRENT_USER, L"Software\\ZenTask", L"StartUp", should_start, f);
	if (!f || should_start == 1)
		startup = true;

	if (startup)
	{
		std::wstring val;
		bool found = false;
		WinReg::read(REG_SZ, HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", L"ZenTask", val, found);

		if (!found || val != app_path)
			WinReg::write(REG_SZ, HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", L"ZenTask", app_path);
	}

	IMAF::AppProperties props;
	props.center_window = true;
	props.custom_titlebar = true;
	props.dpi_aware = true;
	props.gen_ini = false;
	props.imgui_docking = false;
	props.resizeable = false;
	props.name = "Zen Task";
	props.font_size = 20;
	props.width.relative = 0.25f;
	props.height.relative = 0.65f;
	props.custom_titlebar_props.top_border = IMAF::TopBorder::Thin;
	props.custom_titlebar_props.titlebar_scaling_f = ZenTask::TitlebarScalingCallback;
	props.custom_titlebar_props.titlebar_draw_f = ZenTask::TitlebarDraw;

	ZenTask::WindowMgr wndmgr(props);

	wndmgr.Start();
	//Sleep(500000);

	return 0;
}