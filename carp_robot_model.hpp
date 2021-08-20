#ifndef CARP_ROBOT_MODEL_INCLUDED
#define CARP_ROBOT_MODEL_INCLUDED

#include "carp_robot.hpp"

class CarpRobotLinear
{
public:
	CarpRobotLinear(CarpRobotParameterCollection* model, int input_dim, int output_dim)
	{
		m_w = model->AddParameters(CarpRobotDim(output_dim, input_dim), "FC-w");
		m_b = model->AddParameters(CarpRobotDim(output_dim), "FC-b");
	}

public:
	void Build(CarpRobotComputationGraph* graph)
	{
		m_W = graph->AddParameters(m_w);
		m_B = graph->AddParameters(m_b);
	}

	CarpRobotExpression Forward(CarpRobotExpression& input)
	{
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
};


#endif