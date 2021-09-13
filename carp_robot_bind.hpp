#ifndef CARP_ROBOT_BIND_INCLUDED
#define CARP_ROBOT_BIND_INCLUDED

#include "carp_robot.hpp"
#include "carp_robot_model.hpp"
#include "carp_lua.hpp"

class CarpRobotInput
{
public:
	CarpRobotInput(int dim_0, int dim_1, int dim_2)
	{
		std::vector<int> dims;
		if (dim_0 > 0)
		{
			dims.push_back(dim_0);
			if (dim_1 > 0)
			{
				dims.push_back(dim_1);
				if (dim_2 > 0)
					dims.push_back(dim_2);
			}
		}

		m_dim = CarpRobotDim(dims);
		m_value.resize(m_dim.GetTotalSize());
	}

public:
	void Build(CarpRobotComputationGraph* graph)
	{
		m_expression = graph->AddInput(m_dim, &m_value);
	}

	CarpRobotExpression Forward()
	{
		return m_expression;
	}

	int Calc(CarpRobotComputationGraph* graph)
	{
		return Forward().GetIndex();
	}

	int Update(lua_State* l_state)
	{
		if (!lua_istable(l_state, 3)) return 0;
		size_t offset = static_cast<size_t>(luaL_checkinteger(l_state, 2));

		lua_pushnil(l_state);
		while (lua_next(l_state, 3) != 0)
		{
			const auto index = static_cast<int>(lua_tointeger(l_state, -2));
			const auto value = static_cast<cr_real>(lua_tonumber(l_state, -1));

			if (offset >= m_value.size()) break;
			m_value[offset] = value;
			offset++;

			lua_pop(l_state, 1);
		}

		return 0;
	}

private:
	std::vector<cr_real> m_value;
	CarpRobotDim m_dim;
	CarpRobotExpression m_expression;
};

class CarpRobotBind
{
public:
	static void Bind(lua_State* l_state)
	{
		luabridge::getGlobalNamespace(l_state)
			.beginNamespace("carp")
			.beginClass<CarpRobotComputationGraph>("CarpRobotComputationGraph")
			.addConstructor<void(*)()>()
			.addFunction("Clear", &CarpRobotComputationGraph::Clear)
			.addFunction("Invalidate", &CarpRobotComputationGraph::Invalidate)
			.addFunction("Backward", &CarpRobotComputationGraph::Backward)
			.addFunction("AsScalar", &CarpRobotComputationGraph::AsScalar)

			.addFunction("Negate", &CarpRobotComputationGraph::Negate)
			.addFunction("Addition", &CarpRobotComputationGraph::Addition)
			.addFunction("Plus", &CarpRobotComputationGraph::Plus)
			.addFunction("Subtraction", &CarpRobotComputationGraph::Subtraction)
			.addFunction("Minus", &CarpRobotComputationGraph::Minus)
			.addFunction("Multiplication", &CarpRobotComputationGraph::Multiplication)
			.addFunction("Multiply", &CarpRobotComputationGraph::Multiply)
			.addFunction("Division", &CarpRobotComputationGraph::Division)
			.addFunction("Divide", &CarpRobotComputationGraph::Divide)

			.addFunction("Square", &CarpRobotComputationGraph::Square)
			.addFunction("PickNegLogSoftmax", &CarpRobotComputationGraph::PickNegLogSoftmax)
			.addFunction("BinaryLogLoss", &CarpRobotComputationGraph::BinaryLogLoss)

			.addFunction("Sigmoid", &CarpRobotComputationGraph::Sigmoid)
			.addFunction("Rectify", &CarpRobotComputationGraph::Rectify)
			.addFunction("Softmax", &CarpRobotComputationGraph::Softmax)
			.addFunction("LogSoftmax", &CarpRobotComputationGraph::LogSoftmax)

			.addFunction("Dropout", &CarpRobotComputationGraph::Dropout)
			.addFunction("Conv2D", &CarpRobotComputationGraph::Conv2D)
			.addFunction("MaxPooling2D", &CarpRobotComputationGraph::MaxPooling2D)
			.addFunction("Reshape", &CarpRobotComputationGraph::Reshape)
			.addFunction("PickElement", &CarpRobotComputationGraph::PickElement)
			.addFunction("MeanElements", &CarpRobotComputationGraph::MeanElements)
			.endClass()
			.beginClass<CarpRobotParameterCollection>("CarpRobotParameterCollection")
			.addConstructor<void(*)()>()
			.addFunction("Load", &CarpRobotParameterCollection::Load)
			.addFunction("Save", &CarpRobotParameterCollection::Save)
			.endClass()
			.beginClass<CarpRobotAdamTrainer>("CarpRobotAdamTrainer")
			.addConstructor<void(*)(CarpRobotParameterCollection*, float, float, float, float)>()
			.addFunction("Update", &CarpRobotAdamTrainer::Update)
			.endClass()
			.beginClass<CarpRobotInput>("CarpRobotInput")
			.addConstructor<void(*)(int, int, int)>()
			.addFunction("Build", &CarpRobotInput::Build)
			.addFunction("Calc", &CarpRobotInput::Calc)
			.addCFunction("Update", &CarpRobotInput::Update)
			.endClass()
			.beginClass<CarpRobotLinear>("CarpRobotLinear")
			.addConstructor<void(*)(CarpRobotParameterCollection*, int, int)>()
			.addFunction("Build", &CarpRobotLinear::Build)
			.addFunction("Copy", &CarpRobotLinear::Copy)
			.addFunction("Calc", &CarpRobotLinear::Calc)
			.endClass()
			.beginClass<CarpRobotConv2D>("CarpRobotConv2D")
			.addConstructor<void(*)(CarpRobotParameterCollection*, int, int, int, int, int, int, bool)>()
			.addFunction("Build", &CarpRobotConv2D::Build)
			.addFunction("Copy", &CarpRobotConv2D::Copy)
			.addFunction("Calc", &CarpRobotConv2D::Calc)
			.endClass()
		
			.endNamespace();
	}
};

#endif