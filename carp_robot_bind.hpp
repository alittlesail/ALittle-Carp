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

	void Copy(const std::vector<cr_real>& data)
	{
		m_value = data;
	}

private:
	std::vector<cr_real> m_value;
	CarpRobotDim m_dim;
	CarpRobotExpression m_expression;
};

class CarpRobotMnist
{
public:
	static unsigned int CarpRtpReadUint32(unsigned int value)
	{
		char* ptr = (char*)&value;
		std::swap(ptr[0], ptr[3]);
		std::swap(ptr[1], ptr[2]);
		return value;
	}

public:
	bool Load(const char* root_path)
	{
		if (root_path == nullptr) return false;

		m_images.clear();
		m_labels.clear();

		std::string mnist_path = root_path;

		// 读取图片
		{
			std::string file_path = mnist_path + "/train-images-idx3-ubyte";

			FILE* file = nullptr;
#ifdef _WIN32
			fopen_s(&file, file_path.c_str(), "rb");
#else
			file = fopen(file_path.c_str(), "rb");
#endif
			if (file == nullptr) return false;

			struct ImageHead
			{
				unsigned int desc = 0;
				unsigned int count = 0;
				unsigned int width = 0;
				unsigned int height = 0;
			};

			// 读取头信息
			ImageHead head;
			size_t read = fread(&head, 1, sizeof(head), file);
			if (read != sizeof(head))
			{
				fclose(file);
				return false;
			}

			head.desc = CarpRtpReadUint32(head.desc);
			head.width = CarpRtpReadUint32(head.width);
			head.height = CarpRtpReadUint32(head.height);
			head.count = CarpRtpReadUint32(head.count);

			// 图片像素数量
			auto pixel_count = head.width * head.height;

			m_images.resize(head.count);
			std::vector<unsigned char> buffer;
			buffer.resize(pixel_count);
			for (size_t i = 0; i < m_images.size(); ++i)
			{
				auto& image = m_images[i];
				image.resize(pixel_count);
				read = fread(buffer.data(), 1, pixel_count, file);
				if (read != pixel_count)
				{
					fclose(file);
					return false;
				}
				for (size_t j = 0; j < buffer.size(); ++j)
					image[j] = buffer[j] / 255.0f;
			}

			fclose(file);
		}

		// 读取标签
		{
			std::string file_path = mnist_path + "/train-labels-idx1-ubyte";

			FILE* file = nullptr;
#ifdef _WIN32
			fopen_s(&file, file_path.c_str(), "rb");
#else
			file = fopen(file_path.c_str(), "rb");
#endif
			if (file == nullptr) return false;

			struct LabelHead
			{
				unsigned int desc = 0;
				unsigned int count = 0;
			};

			// 读取头信息
			LabelHead head;
			size_t read = fread(&head, 1, sizeof(head), file);
			if (read != sizeof(head))
			{
				fclose(file);
				return false;
			}

			head.desc = CarpRtpReadUint32(head.desc);
			head.count = CarpRtpReadUint32(head.count);

			m_labels.resize(head.count);
			std::vector<unsigned char> buffer;
			buffer.resize(head.count);
			read = fread(buffer.data(), 1, head.count, file);
			if (read != head.count)
			{
				fclose(file);
				return false;
			}

			for (size_t j = 0; j < m_labels.size(); ++j)
				m_labels[j] = buffer[j];

			fclose(file);
		}

		return true;
	}

	bool GetImage(int index, CarpRobotInput* input)
	{
		if (index < (int)m_images.size()) return false;
		input->Copy(m_images[index]);
		return true;
	}

	int GetLabel(int index)
	{
		if (index < (int)m_labels.size()) return 0;
		return m_labels[index];
	}

	int GetCount()
	{
		return (int)m_images.size();
	}

private:
	std::vector<std::vector<cr_real>> m_images;
	std::vector<int> m_labels;
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
			.addFunction("AsVectorAndArgmax", &CarpRobotComputationGraph::AsVectorAndArgmax)

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
		

			.beginClass<CarpRobotMnist>("CarpRobotMnist")
			.addConstructor<void(*)()>()
			.addFunction("Load", &CarpRobotMnist::Load)
			.addFunction("GetImage", &CarpRobotMnist::GetImage)
			.addFunction("GetLabel", &CarpRobotMnist::GetLabel)
			.addFunction("GetCount", &CarpRobotMnist::GetCount)
			.endClass()

			.endNamespace();
	}
};

#endif