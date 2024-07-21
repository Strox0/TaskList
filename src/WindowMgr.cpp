#ifndef GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif // !GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"
#include "WindowMgr.h"
#include "TaskList.h"
#include "TaskView.h"
#include "TaskEdit.h"
#include <thread>
#include "IMAF/fonts.h"

ZenTask::WindowMgr* _mgr = nullptr;

ZenTask::WindowMgr::WindowMgr(IMAF::AppProperties props) :
	m_titlebar_height(0),
	mp_selected_task(nullptr),
	m_tasklist(std::make_shared<TaskList>(&m_titlebar_height, switchPanel)),
	m_app(props),
	m_imaf_id(m_tasklist->GetId()),
	m_titlebar_props(props.custom_titlebar_props)
{
	_mgr = this;
	m_app.SetUp([this](const IMAF::AppProperties& props,GLFWwindow* window)
		{
			IMAF::Application::SetPrurpleDarkColorTheme();
			ImGuiStyle& style = ImGui::GetStyle();
			style.WindowPadding = { 0,0 };
			style.ItemSpacing = { IMAF::Val(10.f),IMAF::Val(16.f) };
			style.ItemInnerSpacing = { IMAF::Val(10.f),IMAF::Val(10.f) };
			style.FrameRounding = IMAF::Val(10.f);
			style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(1, 1, 1, 0);
			style.Colors[ImGuiCol_PlotHistogram] = RGB2_IMVEC4(101, 177, 87);

			m_titlebar_props.height = IMAF::Val(45);
			m_titlebar_props.AddButton(0, IMAF::ButtonSpec(IMAF::Val(40), IMAF::Val(40), IMAF::Val(3), IMAF::ButtonType::Close), 0.001f, IMAF::GroupAlign::Right);
			m_titlebar_props.AddExclusion(IMAF::ExclusionSpec(0.f, IMAF::Val(5), IMAF::Val(40) + style.ItemSpacing.x, IMAF::Val(40)));
			m_app.SetTitlebarProperties(m_titlebar_props);
			m_titlebar_height.store((float)m_titlebar_props.height);
		}
	);

	m_app.AddPanel(m_tasklist);

	m_task_total = m_tasklist->GetPendingTaskCount();
}

void ZenTask::WindowMgr::Start()
{
	std::thread t([this]() { m_app.Run(); });

	//t.detach();
	t.join();
}

void ZenTask::switchPanel(uint64_t id,unsigned short target_panel_id)
{
	switch (target_panel_id)
	{
	case PANEL_ID_TASKLIST:
	{
		_mgr->m_app.AddPanel(_mgr->m_tasklist);
		_mgr->m_app.RemovePanel(id);
		_mgr->m_current_panel_id = PANEL_ID_TASKLIST;
		_mgr->m_imaf_id = _mgr->m_tasklist->GetId();
		break;
	}
	case PANEL_ID_TASKVIEW:
	{
		if (_mgr->mp_selected_task == nullptr)
			return;
		std::shared_ptr<TaskView> tv = std::make_shared<TaskView>(&_mgr->m_titlebar_height, switchPanel, *_mgr->mp_selected_task);
		_mgr->m_app.AddPanel(tv);
		_mgr->m_app.RemovePanel(id);
		_mgr->m_current_panel_id = PANEL_ID_TASKVIEW;
		_mgr->m_imaf_id = tv->GetId();
		break;
	}
	case PANEL_ID_TASKEDIT:
	{
		std::shared_ptr<TaskEdit> te = std::make_shared<TaskEdit>(&_mgr->m_titlebar_height, switchPanel, _mgr->mp_selected_task);
		_mgr->m_app.AddPanel(te);
		_mgr->m_app.RemovePanel(id);
		_mgr->m_current_panel_id = PANEL_ID_TASKEDIT;
		_mgr->m_imaf_id = te->GetId();

		break;
	}
	}
}

void ZenTask::TitlebarScalingCallback(IMAF::Titlebar_Properties* out_props, float scale)
{
	IMAF::DefCustomTitlebarScaling(out_props, scale);

	_mgr->m_titlebar_props = *out_props;
	_mgr->m_titlebar_height.store((float)out_props->height);
}

void ZenTask::TaskEdited()
{
	_mgr->m_tasklist->TaskEdited(*_mgr->mp_selected_task);
}

void ZenTask::CompletedTask()
{
	_mgr->m_task_completed++;
}

void ZenTask::Alert(Task::Task& due_task)
{
	SetCurrentTask(&due_task);
	switchPanel(_mgr->m_imaf_id, PANEL_ID_TASKVIEW);
	MessageBeep(MB_ICONEXCLAMATION);
	GLFWwindow* window = nullptr;
	while (window == nullptr)
		window = _mgr->m_app.GetWindowHandle();

	glfwRestoreWindow(window);
	glfwFocusWindow(window);

	FLASHWINFO fwi;
	fwi.cbSize = sizeof(fwi);
	fwi.hwnd = glfwGetWin32Window(window);
	fwi.uCount = 3;
	fwi.dwFlags = FLASHW_TRAY;
	fwi.dwTimeout = 0;

	FlashWindowEx(&fwi);
}

void ZenTask::TitlebarDraw(const IMAF::AppProperties* app_props, GLFWwindow* window)
{
	int w, h;
	glfwGetWindowSize(window, &w, &h);
	int x, y;
	glfwGetWindowPos(window, &x, &y);

	const ImVec2 spacing = ImGui::GetStyle().ItemSpacing;
	const IMAF::Titlebar_Properties* props = &app_props->custom_titlebar_props;

	ImGui::SetNextWindowSize({ (float)w,(float)props->height });
	ImGui::SetNextWindowPos({ (float)x,(float)y });
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0,0 });
	IMAF::Begin("##CustomTitlebar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoDocking);

	ImGui::PushStyleColor(ImGuiCol_Button, RGBA2_IMVEC4(37, 37, 37, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 35);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0,0 });
	ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, { 0.48f,0.48f });

	ImVec2 button_pos;
	ImVec2 button_size;
	button_pos.x = w - props->button_groups[0].edge_offset * w - props->button_groups[0].buttons[0].width;
	button_pos.y = props->button_groups[0].buttons[0].top_offset;
	button_size.x = props->button_groups[0].buttons[0].width;
	button_size.y = props->button_groups[0].buttons[0].height;

	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, RGBA2_IMVEC4(195, 65, 65, 255));
	ImGui::SetCursorPos(button_pos);
	ImGui::PushFont(IMAF::GetFont(FONT_SEMIBOLD_NORMAL));
	ImGui::Button("X", button_size);
	ImGui::PopStyleColor();
	ImGui::PopFont();

	float height = _mgr->m_titlebar_height * 0.7f;

	switch (_mgr->m_current_panel_id)
	{
	case PANEL_ID_TASKLIST:
	{
		glfCustomTitlebarRemoveExclusions(window);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, IMAF::Val(10.f));
		ImGui::SetCursorPos({ spacing.x,(_mgr->m_titlebar_height-height)/2 });
		char buf[32];
		sprintf_s(buf, "%d/%d", _mgr->m_task_completed, _mgr->m_task_total);
		if (_mgr->m_task_total > 0)
			ImGui::ProgressBar((float)_mgr->m_task_completed / (float)_mgr->m_task_total, {w - spacing.x - 5.f - button_size.x, height }, buf);
		ImGui::PopStyleVar();
		break;
	}
	case PANEL_ID_TASKVIEW:
	case PANEL_ID_TASKEDIT:
	{		
		_mgr->m_app.SetTitlebarProperties(_mgr->m_titlebar_props);

		std::string text = "Edit Task Details";
		if (_mgr->m_current_panel_id == PANEL_ID_TASKVIEW)
			text = _mgr->mp_selected_task->GetName();

		ImVec2 text_pos;
		text_pos.x = (float)w / 2 - ImGui::CalcTextSize(text.c_str()).x / 2;
		text_pos.y = props->height / 2.f - ImGui::CalcTextSize(text.c_str()).y / 2;

		ImGui::SetCursorPos(text_pos);
		ImGui::Text(text.c_str());

		ImGui::SetCursorPos({ spacing.x,(_mgr->m_titlebar_height - height) / 2 });
		if (ImGui::Button(ICON_FA_ARROW_LEFT, { height,height }))
			switchPanel(_mgr->m_imaf_id, PANEL_ID_TASKLIST);

		break;
	}
	}

	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	DrawList->AddLine({ (float)x+spacing.x,(float)(props->height - 2 + y)}, {(float)(w + x)-spacing.x,(float)(props->height - 2 + y)}, IM_COL32(163, 163, 163, 255), 1.0f);

	//Draw exclusions
	//for (const auto& i : props->exclusions)
	//{
	//	ImVec2 start_pos = { (float)(i.start_offset * w + x),(float)(i.top_offset + y) };
	//	ImVec2 end_pos = { (float)(i.start_offset * w + x + i.width),(float)(i.top_offset + i.height + y) };
	//	DrawList->AddRectFilled(start_pos, end_pos, IM_COL32(0, 175, 0, 100));
	//}

	ImGui::PopStyleVar(3);
	ImGui::PopStyleColor();

	IMAF::End();
	ImGui::PopStyleVar();
}

void ZenTask::AddTask(Task::Task* task)
{
	_mgr->m_tasklist->AddTask(*task);
	_mgr->m_task_total++;
}

void ZenTask::SetCurrentTask(Task::Task* task)
{
	_mgr->mp_selected_task = task;
}
