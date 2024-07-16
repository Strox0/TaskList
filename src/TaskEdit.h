#pragma once
#include "IMAF/Panel.h"
#include "Task.h"
#include <atomic>

class TaskEdit : public IMAF::Panel
{
public:
	
	TaskEdit(std::atomic<float>* titlebar_height, void (*panel_switch_callback)(uint64_t caller_id, unsigned short target_panel_id), Task::Task* task);

	~TaskEdit();

	void UiRender() override;

private:
	std::atomic<float>* m_titlebar_height;
	void (*m_panel_switch_callback)(uint64_t caller_id, unsigned short target_panel_id);

	bool m_adding_task;

	std::string m_title;
	std::string m_description;
	tm m_due_date_tm;
	Task::Task::Priority m_priority;

	int m_priority_index;
	bool m_datechooser_start = true;
	bool m_timechooser_start = true;

	Task::Task* m_task;
};

