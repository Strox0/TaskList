#include "TaskView.h"
#include "imgui_stdlib.h"
#include "WindowMgr.h"

TaskView::TaskView(std::atomic<float>* titlebar_height, void(*panel_switch_callback)(uint64_t caller_id, unsigned short target_panel_id), const Task::Task& task) : 
    m_task(task),
    m_titlebar_height(titlebar_height),
    m_panel_switch_callback(panel_switch_callback)
{
}

void TaskView::UiRender()
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 pos = viewport->Pos;
    pos.y += m_titlebar_height->load();
    ImVec2 size = viewport->Size;
    size.y -= m_titlebar_height->load();

    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize(size);
    IMAF::Begin("##TaskList", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    const ImVec2& spacing = IMAF::Val(33.5f, 33.5f);
    ImVec2 rect_start = { pos.x + spacing.x, pos.y + spacing.y };
    ImVec2 rect_end = { pos.x + size.x - spacing.x, pos.y + size.y - spacing.y };

    draw_list->AddRect(rect_start, rect_end, IM_COL32(255, 255, 255, 255), ImGui::GetStyle().FrameRounding, 0, 2.f);

    ImVec2 date_short_size = ImGui::CalcTextSize(m_task.GetShortDueDateAsString().c_str());
    ImVec2 date_long_size = ImGui::CalcTextSize(m_task.GetLongDueDateAsString().c_str());

    ImGui::PushFont(IMAF::GetFont(FONT_SEMIBOLD_NORMAL));

    // Render dates
    ImGui::SetCursorPos({ rect_start.x + spacing.x - pos.x, rect_start.y + spacing.y - pos.y });
    ImGui::Text(m_task.GetLongDueDateAsString().c_str());
    ImGui::SetCursorPos({ rect_end.x - date_short_size.x - spacing.x - pos.x, rect_start.y + spacing.y - pos.y });
    ImGui::Text(m_task.GetShortDueDateAsString().c_str());

    // Draw line
    ImVec2 line_start = { rect_start.x + spacing.x, rect_start.y + date_long_size.y + spacing.y };
    ImVec2 line_end = { rect_end.x - spacing.x, line_start.y };
    draw_list->AddLine(line_start, line_end, IM_COL32(255, 255, 255, 255), 1);

    // Render priority
    ImVec2 priority_size = ImGui::CalcTextSize(m_task.GetPriorityAsString().c_str());
    ImGui::SetCursorPos({ line_start.x - pos.x, line_start.y + spacing.y / 2 - pos.y });
    Task::PushPriorityTextColor(m_task.GetPriority());
    ImGui::Text(m_task.GetPriorityAsString().c_str());
    ImGui::PopStyleColor();

    ImGui::PopFont();

    // Render description
    ImVec2 button_size = { size.x - spacing.x * 4, size.y * 0.08f };
    ImVec2 child_size = { size.x - spacing.x * 4, size.y - line_start.y - spacing.y * 3 - priority_size.y + pos.y - button_size.y };
    ImGui::SetNextWindowPos({ line_start.x, line_start.y + spacing.y + priority_size.y });
    ImGui::BeginChild("##Description", child_size);
    ImGui::TextWrapped(m_task.GetDescription().c_str());
    ImGui::EndChild();

    // Render edit button
    ImGui::SetCursorPos({ line_start.x - pos.x, line_start.y + child_size.y + spacing.y * 2 - pos.y });
    ImGui::PushFont(IMAF::GetFont(FONT_SEMIBOLD_NORMAL));
    if (ImGui::Button("Edit", button_size))
    {
        m_panel_switch_callback(this->id, PANEL_ID_TASKEDIT);
    }
    ImGui::PopFont();

    IMAF::End();
}
