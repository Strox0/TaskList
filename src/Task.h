#pragma once
#include <chrono>
#include <string>
#include "IMAF/Application.h"

struct ImVec4;

namespace Task
{
	constexpr ImVec4 CRITICAL_COLOR = RGB2_IMVEC4(230, 57, 70);
	constexpr ImVec4 URGENT_COLOR = RGB2_IMVEC4(241, 130, 60);
	constexpr ImVec4 HIGH_COLOR = RGB2_IMVEC4(252, 191, 73);
	constexpr ImVec4 MEDIUM_COLOR = RGB2_IMVEC4(6, 214, 160);
	constexpr ImVec4 LOW_COLOR = RGB2_IMVEC4(17, 138, 178);

	class Task
	{
	public: 
		using TimePoint = std::chrono::system_clock::time_point;
		enum Priority
		{
			CRITICAL,
			URGENT,
			HIGH,
			MEDIUM,
			LOW
		};

		Task();
		Task(std::string name, std::string description, TimePoint due_date, Priority priority);
		Task(std::string name, std::string description, TimePoint due_date, Priority priority, bool completed, TimePoint completion_date, TimePoint creation_day);

		const std::string& GetName() const;
		const std::string& GetDescription() const;
		const TimePoint& GetDueDate() const;
		Priority GetPriority() const;
		const TimePoint& GetCreationDate() const;
		const std::string GetPriorityAsString() const;
		bool IsCompleted() const;
		const TimePoint& GetCompletionDate() const;
		const std::string GetShortDueDateAsString() const;
		const std::string GetLongDueDateAsString() const;

		void SetCompletion(bool completed, const TimePoint& completion_date);
		void SetName(const std::string& name);
		void SetDescription(const std::string& description);
		void SetDueDate(const TimePoint& due_date);
		void SetPriority(Priority priority);
		void SetCreationDate(const TimePoint& creation_date);

	private:
		std::string m_name;
		std::string m_description;
		TimePoint m_due_date;
		Priority m_priority;
		bool m_completed;
		TimePoint m_completion_date;
		TimePoint m_creation_date;
	};

	std::ostream& Serialize(const Task& task, std::ostream& stream);
	Task Deserialize(std::istream& stream);

	void PushPriorityTextColor(const Task::Priority& priority);
}