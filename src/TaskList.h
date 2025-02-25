#pragma once
#include "IMAF/Panel.h"
#include <vector>
#include <filesystem>
#include <atomic>
#include <unordered_set>

#include "Task.h"

class TaskList : public IMAF::Panel
{
public:

	TaskList(std::atomic<float>* titlebar_height, void (*panel_switch_callback)(uint64_t caller_id,unsigned short target_panel_id));

	~TaskList();

	void UiRender() override;

	void AddTask(const Task::Task& task);
	void TaskEdited(const Task::Task& task);

	int GetPendingTaskCount() const;

	Task::Task& GetDueTask();

private:

	void LoadTasks();
	void SaveTasks();

	float TaskNode(Task::Task& task, float x_pos);

private:
	void (*m_panel_switch_callback)(uint64_t caller_id,unsigned short target_panel_id);
	std::atomic<float>* m_titlebar_height;

	std::vector<Task::Task> m_tasks;
	std::unordered_set<std::string> m_alerted_tasks;
	size_t m_saved_task_count = 0;

	std::filesystem::path m_dir_path;
};

