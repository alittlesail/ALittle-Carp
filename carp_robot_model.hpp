#ifndef CARP_ROBOT_MODEL_INCLUDED
#define CARP_ROBOT_MODEL_INCLUDED

#include "carp_robot.hpp"

class CarpRobotLinear
{
public:
	CarpRobotLinear(CarpRobotParameterCollection* model, int input_dim, int output_dim)
	{
		m_w = model->AddParameters(CarpRobotDim({ output_dim, input_dim  }), "FC-w");
		m_b = model->AddParameters(CarpRobotDim({ output_dim }), "FC-b");
	}

public:
	void Copy(const CarpRobotLinear* linear)
	{
		CARP_ROBOT_ASSERT(m_w->GetValue().GetDim().GetTotalSize() == linear->m_w->GetValue().GetDim().GetTotalSize(), u8"w数据长度不一致");
		CARP_ROBOT_ASSERT(m_b->GetValue().GetDim().GetTotalSize() == linear->m_b->GetValue().GetDim().GetTotalSize(), u8"b数据长度不一致");
		memcpy(m_w->GetValue().GetValue(), linear->m_w->GetValue().GetValue(), m_w->GetValue().GetDim().GetTotalSize() * sizeof(cr_real));
		memcpy(m_b->GetValue().GetValue(), linear->m_b->GetValue().GetValue(), m_b->GetValue().GetDim().GetTotalSize() * sizeof(cr_real));
	}

	void Build(CarpRobotComputationGraph* graph)
	{
		m_W = graph->AddParameters(m_w);
		m_B = graph->AddParameters(m_b);
	}

	CarpRobotExpression Forward(CarpRobotExpression input)
	{
		return m_W * input + m_B;
	}

	int Calc(CarpRobotComputationGraph* graph, int i)
	{
		return Forward(CarpRobotExpression(graph, i)).GetIndex();
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

class CarpRobotConv2D
{
public:
	CarpRobotConv2D(CarpRobotParameterCollection* model
		, int input_dim, int output_dim, int kernel_width, int kernel_height
		, int stride_width = 1, int stride_height = 1, bool padding_type = true)
		: m_stride_width(stride_width), m_stride_height(stride_height), m_padding_type(padding_type)
	{
		std::vector<int> dim;
		dim.push_back(kernel_width);
		dim.push_back(kernel_height);
		dim.push_back(input_dim);
		dim.push_back(output_dim);
		m_k = model->AddParameters(CarpRobotDim(dim), "CONV2D-kernel");
	}

public:
	void Copy(const CarpRobotConv2D* conv2d)
	{
		CARP_ROBOT_ASSERT(m_k->GetValue().GetDim().GetTotalSize() == conv2d->m_k->GetValue().GetDim().GetTotalSize(), u8"w数据长度不一致");
		memcpy(m_k->GetValue().GetValue(), conv2d->m_k->GetValue().GetValue(), m_k->GetValue().GetDim().GetTotalSize() * sizeof(cr_real));
	}

	void Build(CarpRobotComputationGraph* graph)
	{
		m_K = graph->AddParameters(m_k);
	}

	CarpRobotExpression Forward(CarpRobotExpression input)
	{
		return input.Conv2D(m_K, m_stride_width, m_stride_height, m_padding_type);
	}

	int Calc(CarpRobotComputationGraph* graph, int i)
	{
		return Forward(CarpRobotExpression(graph, i)).GetIndex();
	}

public:
	void Serialize(CarpRobotModelSerializer& file, const char* name)
	{
		if (name)
			file.WriteString(name);
		else
			file.WriteString("CONV2D");

		m_k->Serialize(file, "k");
	}
	void Deserialize(CarpRobotModelDeserializer& file, const char* name)
	{
		std::string name_copy = "CONV2D";
		if (name) name_copy = name;
		std::string read_name = file.ReadString();
		CARP_ROBOT_ASSERT(read_name == name_copy, u8"反序列化名称错误, 当前是:" << read_name << u8"应当是:" << name_copy);

		m_k->Deserialize(file, "k");
	}

private:
	CarpRobotParameter* m_k = nullptr;
	CarpRobotExpression m_K;

private:
	int m_stride_width = 0;
	int m_stride_height = 0;
	bool m_padding_type = true;
};

#endif