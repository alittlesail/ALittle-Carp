#ifndef CARP_ROBOT_MODEL_INCLUDED
#define CARP_ROBOT_MODEL_INCLUDED

#include "carp_robot.hpp"

class CarpRobotDnn
{
public:
	typedef CarpRobotExpression(*ActiveFunction)(const CarpRobotExpression&);

public:
	CarpRobotDnn(CarpRobotParameterCollection* model, int input_dim, int output_dim, ActiveFunction active)
	{
		m_input_dim = input_dim;
		m_output_dim = output_dim;
		m_active = active;

		m_w = model->AddParameters(CarpRobotDim(m_output_dim, m_input_dim), "FC-w");
		m_b = model->AddParameters(CarpRobotDim(m_output_dim), "FC-b");
	}

public:
	void Build(CarpRobotComputationGraph* graph)
	{
		m_W = graph->AddParameters(m_w);
		m_B = graph->AddParameters(m_b);
	}
	CarpRobotExpression Calc(CarpRobotExpression& input)
	{
		if (m_active)
			return m_active(m_W * input + m_B);
		else
			return m_W * input + m_B;
	}

public:
	void Serialize(CarpRobotModelSerializer& file, const char* name)
	{
		if (name)
			file.WriteString(name);
		else
			file.WriteString("FC");

		m_w->Serialize(file, "w");
		m_b->Serialize(file, "b");
	}
	void Deserialize(CarpRobotModelDeserializer& file, const char* name)
	{
		std::string name_copy = "FC";
		if (name) name_copy = name;
		std::string read_name = file.ReadString();
		CARP_ROBOT_ASSERT(read_name == name_copy, u8"反序列化名称错误, 当前是:" << read_name << u8"应当是:" << name_copy);

		m_w->Deserialize(file, "w");
		m_b->Deserialize(file, "b");
	}

private:
	CarpRobotParameter* m_w = nullptr;
	CarpRobotParameter* m_b = nullptr;
	CarpRobotExpression m_W;
	CarpRobotExpression m_B;
	int m_input_dim = 0;
	int m_output_dim = 0;
	ActiveFunction m_active = nullptr;

private:
	std::string m_tmp;
};


#endif