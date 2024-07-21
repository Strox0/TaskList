#include "TaskEdit.h"
#include "imgui_stdlib.h"
#include "WindowMgr.h"
#include "imguidatechooser.h"
#include "IMAF/fonts.h"

TaskEdit::TaskEdit(std::atomic<float>* titlebar_height, void(*panel_switch_callback)(uint64_t caller_id, unsigned short target_panel_id), Task::Task* task) :
	m_titlebar_height(titlebar_height),
	m_panel_switch_callback(panel_switch_callback),
	m_task(task),
	m_adding_task(false)
{
	if (m_task == nullptr)
	{
		m_adding_task = true;
		ImGui::SetDateToday(&m_due_date_tm);
		m_priority = Task::Task::Priority::CRITICAL;
		m_priority_index = Task::Task::Priority::CRITICAL;
	}
	else
	{
		m_title = m_task->GetName();
		m_description = m_task->GetDescription();
		time_t tmp = std::chrono::system_clock::to_time_t(m_task->GetDueDate());
		m_due_date_tm = *std::localtime(&tmp);
		m_priority = m_task->GetPriority();
		m_priority_index = m_priority;
	}
}

TaskEdit::~TaskEdit()
{
	if (m_adding_task)
		delete m_task;
}

void TaskEdit::UiRender()
{
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImVec2 pos = viewport->Pos;
	pos.y += m_titlebar_height->load();
	ImGui::SetNextWindowPos(pos);
	ImVec2 size = viewport->Size;
	size.y -= m_titlebar_height->load();
	ImGui::SetNextWindowSize(size);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(size.x * 0.1f, 0));

	IMAF::Begin("##TaskEdit", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);
	
	const ImVec2 group_spacing = {0,IMAF::Val(3.4f)};
	const ImVec2 spacing = ImGui::GetStyle().ItemSpacing;

	const float title_size = size.x * 0.25f;
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, group_spacing);
	ImGui::SetCursorPosY(spacing.y);
	ImGui::Text("Title:");
	ImGui::PopStyleVar();
	ImGui::SetNextItemWidth(title_size);
	ImGui::InputText("##title_input", &m_title);
	
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, group_spacing);
	const char* items[] = { "Critical", "Urgent", "High", "Medium", "Low" };
	const char* combo_preview_value = items[m_priority_index];
	ImGui::Text("Priority:");
	ImGui::PopStyleVar();
	Task::PushPriorityTextColor((Task::Task::Priority)m_priority_index);
	if (ImGui::BeginCombo("##priority_combo", combo_preview_value, ImGuiComboFlags_WidthFitPreview))
	{
		for (int n = 0; n < IM_ARRAYSIZE(items); n++)
		{
			const bool is_selected = (m_priority_index == n);
			Task::PushPriorityTextColor((Task::Task::Priority)n);
			if (ImGui::Selectable(items[n], is_selected))
				m_priority_index = n;

			if (is_selected)
				ImGui::SetItemDefaultFocus();
			ImGui::PopStyleColor();
		}
		ImGui::EndCombo();
	}
	ImGui::PopStyleColor();

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(IMAF::Val(3.5f), group_spacing.y));
	ImGui::Text("Due Date:");

	std::string day_str = std::to_string(m_due_date_tm.tm_mday);
	day_str += "##dayi";
	std::string month_str = std::to_string(m_due_date_tm.tm_mon + 1);
	month_str += "##monthi";
	std::string year_str = std::to_string(m_due_date_tm.tm_year + 1900);
	year_str += "##yeari";
	std::string hour_str = std::to_string(m_due_date_tm.tm_hour);
	hour_str += "##houri";
	std::string minute_str = std::to_string(m_due_date_tm.tm_min);
	minute_str += "##minutei";

	ImVec2 double_digit_size = ImGui::CalcTextSize("00");
	double_digit_size.x += ImGui::GetStyle().ItemInnerSpacing.x*2;
	double_digit_size.y = 0;

	if (ImGui::Button(day_str.c_str(), double_digit_size))
		ImGui::OpenPopup("date_chooser");
	ImGui::SameLine();
	if (ImGui::Button(month_str.c_str(), double_digit_size))
		ImGui::OpenPopup("date_chooser");
	ImGui::SameLine();
	if (ImGui::Button(year_str.c_str(), { double_digit_size.x * 2 - ImGui::GetStyle().ItemInnerSpacing.x*2, 0 }))
		ImGui::OpenPopup("date_chooser");
	ImGui::SameLine(); ImGui::Text(":"); ImGui::SameLine();
	if (ImGui::Button(hour_str.c_str(), double_digit_size))
		ImGui::OpenPopup("time_chooser");
	ImGui::SameLine();
	ImGui::PopStyleVar();
	if (ImGui::Button(minute_str.c_str(), double_digit_size))
		ImGui::OpenPopup("time_chooser");

	ImGui::DateChooserPopup("date_chooser", m_due_date_tm, &m_datechooser_start, ICON_FA_ARROW_LEFT, ICON_FA_ARROW_RIGHT);

	ImGui::TimeChooserPopup("time_chooser", m_due_date_tm, &m_timechooser_start, ICON_FA_ARROW_UP, ICON_FA_ARROW_DOWN);

	const ImVec2 button_size = { -1,size.y * 0.1f };

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, group_spacing);
	ImGui::Text("Description");
	const ImVec2 desc_size = { -1, size.y - button_size.y - ImGui::GetCursorPosY() - spacing.y * 4 };
	ImGui::InputTextMultiline("##description_input", &m_description,desc_size);
	ImGui::PopStyleVar();

	ImGui::PushFont(IMAF::GetFont(FONT_BOLD_NORMAL));
	std::string button_text = m_adding_task ? "ADD" : "SAVE";
	ImGui::SetCursorPosY(size.y - button_size.y - spacing.y*2);
	if (ImGui::Button(button_text.c_str(), button_size))
	{
		if (m_adding_task)
			m_task = new Task::Task(m_title, m_description, std::chrono::system_clock::from_time_t(std::mktime(&m_due_date_tm)), (Task::Task::Priority)m_priority_index);
		else
		{
			m_task->SetName(m_title);
			m_task->SetDescription(m_description);
			m_task->SetPriority((Task::Task::Priority)m_priority_index);
			m_task->SetDueDate(std::chrono::system_clock::from_time_t(std::mktime(&m_due_date_tm)));
			m_task->SetCreationDate(std::chrono::system_clock::now());
		}

		if (m_adding_task)
			ZenTask::AddTask(m_task);
		else
			ZenTask::TaskEdited();
		
		m_panel_switch_callback(this->GetId(), PANEL_ID_TASKLIST);
	}
	ImGui::PopFont();
	ImGui::PopStyleVar();

	IMAF::End();
}
