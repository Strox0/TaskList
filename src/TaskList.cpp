#include "TaskList.h"
#include "WindowMgr.h"
#include <IMAF/Application.h>
#include <fstream>
#include <cstdlib>
#include "imgui.h"
#include "IMAF/fonts.h"

TaskList::TaskList(std::atomic<float>* titlebar_height, void(*panel_switch_callback)(uint64_t caller_id,unsigned short target_panel_id)) :
	m_panel_switch_callback(panel_switch_callback),
	m_titlebar_height(titlebar_height)
{
	char* localAppDataPath = nullptr;
	size_t size = 0;
	errno_t err = _dupenv_s(&localAppDataPath, &size, "LOCALAPPDATA");
	if (err != 0 || localAppDataPath == nullptr)
		return;

	m_dir_path = std::filesystem::path(localAppDataPath);
	free(localAppDataPath);

	m_dir_path /= "ZenTask";

	if (!std::filesystem::exists(m_dir_path))
	{
		std::filesystem::create_directory(m_dir_path);
		return;
	}

	LoadTasks();
}

TaskList::~TaskList()
{
	SaveTasks();
}

void TaskList::UiRender()
{
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImVec2 pos = viewport->Pos;
	pos.y += m_titlebar_height->load();
	ImGui::SetNextWindowPos(pos);
	ImVec2 size = viewport->Size;
	size.y -= m_titlebar_height->load();
	ImGui::SetNextWindowSize(size);

	IMAF::Begin("##TaskList", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);

	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 spacing = ImGui::GetStyle().ItemSpacing;
	ImVec2 button_size = { size.x - spacing.x * 2, size.y * 0.1f };
	ImVec2 child_size = { size.x,size.y - button_size.y - spacing.y*3 };
	ImVec2 line_start = { pos.x + spacing.x, pos.y + child_size.y + spacing.y };
	ImVec2 line_end = { pos.x + size.x - spacing.x, line_start.y };

	ImGui::BeginChild("##TaskListChild", child_size);

	ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 255, 0));

	float y_pos = pos.y;
	for (auto& i : m_tasks)
	{
		if (i.IsCompleted())
			continue;
		y_pos = TaskNode(i, y_pos);
	}

	ImGui::PopStyleColor();

	ImGui::EndChild();

	draw_list->AddLine(line_start, line_end, IM_COL32(255, 255, 255, 255), 1.0f);
	
	ImGui::SetCursorPos({spacing.x,line_start.y + spacing.y - pos.y});
	ImGui::PushFont(IMAF::GetFont(FONT_BOLD_NORMAL));
	if (ImGui::Button("ADD TASK", button_size))
	{
		ZenTask::SetCurrentTask(nullptr);
		m_panel_switch_callback(this->id, PANEL_ID_TASKEDIT);
	}
	ImGui::PopFont();

	IMAF::End();
}

void TaskList::AddTask(const Task::Task& task)
{
	m_tasks.push_back(task);
}

void TaskList::TaskEdited()
{
	m_saved_task_count = 0;
	std::filesystem::remove(m_dir_path / "tasks.bin");
}

int TaskList::GetPendingTaskCount() const
{
	int count = 0;
	for (const auto& i : m_tasks)
	{
		if (!i.IsCompleted())
			count++;
	}
	return count;
}

void TaskList::LoadTasks()
{
	std::ifstream file(m_dir_path / "tasks.bin", std::ios::binary);

	while (file)
	{
		Task::Task task = Task::Deserialize(file);
		if (task.GetName().empty())
			break;
		m_tasks.push_back(task);
		m_saved_task_count++;
	}

	file.close();
}

void TaskList::SaveTasks()
{
	std::ofstream file(m_dir_path / "tasks.bin", std::ios::binary | std::ios::app);

	if (!file)
		return;

	for(size_t i = m_saved_task_count; i < m_tasks.size(); i++)
	{
		Task::Serialize(m_tasks[i], file);
	}

	file.close();
}

float TaskList::TaskNode(Task::Task& task, float y_pos)
{
	const ImGuiStyle& style = ImGui::GetStyle();
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	const ImVec2 wpos = ImGui::GetWindowPos();
	const ImVec2 wsize = ImGui::GetWindowSize();
	const ImVec2 scroll = window->Scroll;
	const ImVec2 text_size_caption = ImGui::CalcTextSize(task.GetName().c_str());
	const ImVec2 text_size_priority = ImGui::CalcTextSize(task.GetPriorityAsString().c_str());
	const ImVec2 text_size_due = ImGui::CalcTextSize(task.GetShortDueDateAsString().c_str());

	const float border_thickness = 2.0f;
	const float task_width = wsize.x * 0.85f;
	const float button_width = wsize.x * 0.075f;
	const float item_height = text_size_caption.y * 2.0f;
	const float rounding = style.FrameRounding;

	// Calculate positions
	ImVec2 rect_start = { wpos.x + style.ItemSpacing.x, y_pos + style.ItemSpacing.y };
	ImVec2 rect_end = { rect_start.x + task_width, rect_start.y + item_height };
	ImVec2 button_pos = { rect_end.x + style.ItemSpacing.x - wpos.x, rect_start.y - wpos.y };

	if (window->SkipItems)
		return rect_end.y;

	if (ImGui::IsMouseHoveringRect({ rect_start.x, rect_start.y - scroll.y },{ rect_end.x, rect_end.y - scroll.y }) && ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
	{
		if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
		{
			draw_list->AddRectFilled(
				ImVec2(rect_start.x, rect_start.y - scroll.y),
				ImVec2(rect_end.x, rect_end.y - scroll.y),
				IM_COL32(69, 31, 112, 255),
				rounding,
				ImDrawFlags_RoundCornersAll
			);
		}
		else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			ZenTask::SetCurrentTask(&task);
			m_panel_switch_callback(this->id, PANEL_ID_TASKVIEW);
		}
		else
		{
			draw_list->AddRectFilled(
				ImVec2(rect_start.x, rect_start.y - scroll.y),
				ImVec2(rect_end.x, rect_end.y - scroll.y),
				IM_COL32(79, 33, 128, 255),
				rounding,
				ImDrawFlags_RoundCornersAll
			);
		}
	}

	// Draw task rectangle
	draw_list->AddRect(
		ImVec2(rect_start.x, rect_start.y - scroll.y),
		ImVec2(rect_end.x, rect_end.y - scroll.y),
		IM_COL32(255, 255, 255, 255),
		rounding,
		ImDrawFlags_RoundCornersAll,
		border_thickness
	);

	// Draw and handle 'Done' button
	ImGui::SetCursorPos(button_pos);
	std::string id = ICON_FA_CHECK; id += "##" + task.GetName() + "Done";
	if (ImGui::Button(id.data(), ImVec2(button_width + border_thickness, item_height + border_thickness)))
	{
		task.SetCompletion(true, std::chrono::system_clock::now());
		ZenTask::CompletedTask();
		TaskEdited();
	}

	// Draw button border
	draw_list->AddRect(
		ImVec2(button_pos.x + wpos.x, button_pos.y + wpos.y - scroll.y),
		ImVec2(button_pos.x + wpos.x + button_width + border_thickness, button_pos.y + wpos.y + item_height + border_thickness - scroll.y),
		IM_COL32(255, 255, 255, 255),
		rounding,
		ImDrawFlags_RoundCornersAll,
		border_thickness
	);

	// Draw task name
	ImGui::SetCursorPos(ImVec2(rect_start.x - wpos.x + style.ItemInnerSpacing.x, rect_start.y - wpos.y + text_size_caption.y / 2));
	ImGui::Text(task.GetName().c_str());

	// Draw due date
	ImVec2 due_date_pos = ImVec2(rect_end.x - wpos.x - style.ItemInnerSpacing.x - text_size_due.x, rect_start.y - wpos.y + text_size_due.y / 2);
	ImGui::SetCursorPos(due_date_pos);
	ImGui::Text(task.GetShortDueDateAsString().c_str());

	// Draw sperator
	draw_list->AddLine(
		ImVec2(rect_end.x - style.ItemInnerSpacing.x*2 - text_size_due.x, rect_start.y - scroll.y + style.ItemInnerSpacing.y),
		ImVec2(rect_end.x - style.ItemInnerSpacing.x*2 - text_size_due.x, rect_end.y - scroll.y - style.ItemInnerSpacing.y),
		IM_COL32(255, 255, 255, 255),
		border_thickness
	);

	// Draw pritority
	ImGui::SetCursorPos(ImVec2(rect_end.x - wpos.x - style.ItemInnerSpacing.x*3 - text_size_priority.x - text_size_due.x, rect_start.y - wpos.y + text_size_priority.y / 2));
	Task::PushPriorityTextColor(task.GetPriority());
	ImGui::Text(task.GetPriorityAsString().c_str());
	ImGui::PopStyleColor();

	return rect_end.y;
}
