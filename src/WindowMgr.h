#pragma once
#include "IMAF/Application.h"
#include <memory>
#include <atomic>

#define PANEL_ID_TASKLIST 0
#define PANEL_ID_TASKVIEW 1
#define PANEL_ID_TASKEDIT 2

class TaskList;

namespace Task
{
	class Task;
}

namespace ZenTask
{
	void SetCurrentTask(Task::Task* task);
	void switchPanel(uint64_t caller_id, unsigned short target_panel_id);
	void TitlebarScalingCallback(IMAF::Titlebar_Properties*, float);
	void TitlebarDraw(const IMAF::AppProperties*, GLFWwindow*);
	void AddTask(Task::Task* task);
	void TaskEdited();
	void CompletedTask();

	class WindowMgr
	{
	public:
		friend void CompletedTask();
		friend void TaskEdited();
		friend void TitlebarDraw(const IMAF::AppProperties*, GLFWwindow*);
		friend void AddTask(Task::Task* task);
		friend void TitlebarScalingCallback(IMAF::Titlebar_Properties* out_props, float scale);
		friend void SetCurrentTask(Task::Task* task);
		friend void switchPanel(uint64_t caller_id, unsigned short target_panel_id);

		WindowMgr(IMAF::AppProperties props);

		void Start();

	private:
		IMAF::Application m_app;
		IMAF::Titlebar_Properties m_titlebar_props;

		std::atomic<float> m_titlebar_height;

		Task::Task* mp_selected_task;

		std::shared_ptr<TaskList> m_tasklist;

		unsigned short m_current_panel_id = PANEL_ID_TASKLIST;
		uint64_t m_imaf_id;

		int m_task_total = 0;
		int m_task_completed = 0;
	};

}
