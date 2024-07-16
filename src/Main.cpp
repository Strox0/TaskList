#include "WindowMgr.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
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