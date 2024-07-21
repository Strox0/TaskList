#pragma once
#include <chrono>
#include <string>

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
	Task(std::string name, std::string description, TimePoint due_date, Priority priority, bool completed, TimePoint completion_date, TimePoint creation_day, TimePoint original_due_date);

	const std::string& GetName() const;
	const std::string& GetDescription() const;
	const TimePoint& GetDueDate() const;
	const TimePoint& GetOriginalDueDate() const;
	Priority GetPriority() const;
	const TimePoint& GetCreationDate() const;
	const std::string GetPriorityAsString() const;
	bool IsCompleted() const;
	const TimePoint& GetCompletionDate() const;
	const std::string GetShortDueDateAsString() const;
	const std::string GetLongDueDateAsString() const;
	const std::string GetUniqueName() const;

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
	TimePoint m_original_due_date;
	Priority m_priority;
	bool m_completed;
	TimePoint m_completion_date;
	TimePoint m_creation_date;
};

std::ostream& Serialize(const Task& task, std::ostream& stream);
Task Deserialize(std::istream& stream);