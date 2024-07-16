#pragma once
#include "IMAF/Panel.h"
#include "Task.h"
#include <atomic>

class TaskView : public IMAF::Panel
{
public:

	TaskView(std::atomic<float>* titlebar_height, void (*panel_switch_callback)(uint64_t caller_id,unsigned short target_panel_id), const Task::Task& task);

	void UiRender() override;

private:
	std::atomic<float>* m_titlebar_height;
	void (*m_panel_switch_callback)(uint64_t caller_id,unsigned short target_panel_id);
	const Task::Task& m_task;
};

