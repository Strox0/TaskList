#include "Task.h"
#include "imgui.h"

Task::Task::Task() :
	m_name(""),
	m_description(""),
	m_due_date(std::chrono::system_clock::now()),
	m_priority(Priority::LOW),
	m_completed(false),
	m_completion_date(),
	m_creation_date(std::chrono::system_clock::now())
{
}

Task::Task::Task(std::string name, std::string description, TimePoint due_date, Priority priority) :
	m_name(name),
	m_description(description), 
	m_due_date(due_date), 
	m_priority(priority),
	m_completed(false),
	m_completion_date(),
	m_creation_date(std::chrono::system_clock::now())
{

}

Task::Task::Task(std::string name, std::string description, TimePoint due_date, Priority priority, bool completed, TimePoint completion_date, TimePoint creation_day) :
	m_name(name),
	m_description(description),
	m_due_date(due_date),
	m_priority(priority),
	m_completed(completed),
	m_completion_date(completion_date),
	m_creation_date(creation_day)
{

}

void Task::Task::SetCompletion(bool completed, const TimePoint& completion_date)
{
	m_completed = completed;
	m_completion_date = completion_date;
}

const std::string& Task::Task::GetName() const
{
	return m_name;
}

const std::string& Task::Task::GetDescription() const
{
	return m_description;
}

const Task::Task::TimePoint& Task::Task::GetDueDate() const
{
	return m_due_date;
}

Task::Task::Priority Task::Task::GetPriority() const
{
	return m_priority;
}

const Task::Task::TimePoint& Task::Task::GetCreationDate() const
{
	return m_creation_date;
}

const std::string Task::Task::GetPriorityAsString() const
{
	switch (m_priority)
	{
	case Task::Task::Priority::CRITICAL:	return "Critical";
	case Task::Task::Priority::URGENT:		return "Urgent";
	case Task::Task::Priority::HIGH:		return "High";
	case Task::Task::Priority::MEDIUM:		return "Medium";
	case Task::Task::Priority::LOW:			return "Low";
	}
	return "";
}

bool Task::Task::IsCompleted() const
{
	return m_completed;
}

const Task::Task::TimePoint& Task::Task::GetCompletionDate() const
{
	return m_completion_date;
}

const std::string Task::Task::GetShortDueDateAsString() const
{
	std::chrono::duration<double> diff = m_due_date - std::chrono::system_clock::now();

	int hours = std::chrono::duration_cast<std::chrono::hours>(diff).count();

	if (hours > 1344) //8 weeks
	{
		int months = hours / 720;

		return std::to_string(months) + " months";
	}
	else if (hours > 336) //14 days
	{
		int weeks = hours / 168;

		return std::to_string(weeks) + " weeks";
	}
	else if (hours > 24)
	{
		int days = hours / 24;

		return std::to_string(days) + " days";
	}
	else if (hours > 1)
	{
		return std::to_string(hours) + " hours";
	}
	else
	{
		int minutes = std::chrono::duration_cast<std::chrono::minutes>(diff).count();

		if (minutes < 0)
			return "Overdue";

		return std::to_string(minutes) + " minutes";
	}
}

const std::string Task::Task::GetLongDueDateAsString() const
{
	std::chrono::year_month_day t(std::chrono::floor<std::chrono::days>(m_due_date));
	
	std::string date = std::to_string((int)t.year()) + '.' + std::to_string((unsigned int)t.month()) + '.';
	
	if ((unsigned int)t.day() < 10)
		date += '0';
	date += std::to_string((unsigned int)t.day());

	return date;
}

void Task::Task::SetName(const std::string& name)
{
	m_name = name;
}

void Task::Task::SetDescription(const std::string& description)
{
	m_description = description;
}

void Task::Task::SetDueDate(const TimePoint& due_date)
{
	m_due_date = due_date;
}

void Task::Task::SetPriority(Priority priority)
{
	m_priority = priority;
}

void Task::Task::SetCreationDate(const TimePoint& creation_date)
{
	m_creation_date = creation_date;
}

void Task::PushPriorityTextColor(const Task::Priority& prio)
{
	switch (prio)
	{
	case Task::Task::Priority::CRITICAL:
		ImGui::PushStyleColor(ImGuiCol_Text, CRITICAL_COLOR);
		break;
	case Task::Task::Priority::URGENT:
		ImGui::PushStyleColor(ImGuiCol_Text, URGENT_COLOR);
		break;
	case Task::Task::Priority::HIGH:
		ImGui::PushStyleColor(ImGuiCol_Text, HIGH_COLOR);
		break;
	case Task::Task::Priority::MEDIUM:
		ImGui::PushStyleColor(ImGuiCol_Text, MEDIUM_COLOR);
		break;
	case Task::Task::Priority::LOW:
		ImGui::PushStyleColor(ImGuiCol_Text, LOW_COLOR);
		break;
	}
}

std::ostream& Task::Serialize(const Task& task, std::ostream& stream)
{
	if (!stream.good())
		return stream;

	typedef char sbyte;
	static constexpr uint32_t magic = 0x7a2300ff;

	uint32_t name_size = task.GetName().size();
	uint32_t desc_size = task.GetDescription().size();

	uint32_t size = name_size + desc_size + sizeof(uint32_t) * 4 + sizeof(std::chrono::system_clock::rep) * 3 + sizeof(Task::Priority);

	stream.write((sbyte*)&magic, sizeof(magic));
	stream.write((sbyte*)&size, sizeof(size));

	stream.write((sbyte*)&name_size, sizeof(name_size));
	stream.write(task.GetName().c_str(), name_size);

	stream.write((sbyte*)&desc_size, sizeof(desc_size));
	stream.write(task.GetDescription().c_str(), desc_size);

	std::chrono::system_clock::rep creation_date = task.GetCreationDate().time_since_epoch().count();
	std::chrono::system_clock::rep due_date = task.GetDueDate().time_since_epoch().count();
	std::chrono::system_clock::rep completion_date = task.IsCompleted() ? task.GetCompletionDate().time_since_epoch().count() : 0;

	stream.write((sbyte*)&creation_date,sizeof(creation_date));
	stream.write((sbyte*)&due_date, sizeof(due_date));
	stream.write((sbyte*)&completion_date, sizeof(completion_date));

	Task::Priority prio = task.GetPriority();
	stream.write((sbyte*)&prio, sizeof(prio));

	return stream;
}

Task::Task Task::Deserialize(std::istream& stream)
{
	typedef char sbyte;

	if (!stream.good())
		return Task("", "", std::chrono::system_clock::now(), Task::Priority::LOW);

	long long start_pos = stream.tellg();
	stream.seekg(0, std::ios::end);
	long long data_size = stream.tellg() - start_pos;
	stream.seekg(start_pos, std::ios::beg);

	struct
	{
		uint32_t magic;
		uint32_t size;
	} _info;

	stream.read((sbyte*)&_info, sizeof(_info));

	if (_info.magic != 0x7a2300ff || data_size < _info.size)
		return Task("", "", std::chrono::system_clock::now(), Task::Priority::LOW);

	uint32_t name_size = 0;
	stream.read((sbyte*)&name_size, sizeof(name_size));

	std::string name(name_size, '\0');
	stream.read(&name[0], name_size);

	uint32_t desc_size = 0;
	stream.read((sbyte*)&desc_size, sizeof(desc_size));
	
	std::string desc(desc_size, '\0');
	stream.read(&desc[0], desc_size);

	std::chrono::system_clock::rep creation_date = 0;
	std::chrono::system_clock::rep due_date = 0;
	std::chrono::system_clock::rep completion_date = 0;

	stream.read((sbyte*)&creation_date, sizeof(creation_date));
	stream.read((sbyte*)&due_date, sizeof(due_date));
	stream.read((sbyte*)&completion_date, sizeof(completion_date));

	Task::Priority prio = Task::Priority::LOW;
	stream.read((sbyte*)&prio, sizeof(prio));

	return Task(
		name, 
		desc, 
		Task::Task::TimePoint(std::chrono::system_clock::duration(due_date)), 
		prio, 
		completion_date != 0,
		Task::Task::TimePoint(std::chrono::system_clock::duration(completion_date)),
		Task::Task::TimePoint(std::chrono::system_clock::duration(creation_date))
	);
}
