#ifndef CARP_ROBOT_BIND_INCLUDED
#define CARP_ROBOT_BIND_INCLUDED

#include "carp_robot.hpp"
#include "carp_robot_model.hpp"
#include "carp_lua.hpp"
#include "carp_surface.hpp"

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

	void Copy(cr_real value)
	{
		m_value[0] = value;
	}

private:
	std::vector<cr_real> m_value;
	CarpRobotDim m_dim;
	CarpRobotExpression m_expression;
};

class CarpRobotSurface
{
public:
	bool SetImage(size_t address, int fixed_width, int fixed_height)
	{
		CarpSurface* surface = nullptr;
		memcpy(&surface, &address, sizeof(size_t));

		if (surface == nullptr) return false;
		if (surface->GetWidth() == 0 || surface->GetHeight() == 0) return false;

		// 如果图片不是指定大小那么就进行缩放
		if (surface->GetWidth() == fixed_width && surface->GetHeight() == fixed_height)
		{
			m_value.resize(fixed_width * fixed_height, 0.0f);
			for (int i = 0; i < fixed_width * fixed_height; ++i)
				m_value[i] = static_cast<cr_real>(surface->GetGray(i)) / 255.0f;
		}
		else
		{
			CarpSurface new_surface(fixed_width, fixed_height);
			new_surface.ScaleFrom(surface, nullptr, nullptr);

			m_value.resize(fixed_width * fixed_height, 0.0f);
			for (int i = 0; i < fixed_width * fixed_height; ++i)
				m_value[i] = static_cast<cr_real>(new_surface.GetGray(i)) / 255.0f;
		}

		return true;
	}

	void GetImage(CarpRobotInput* input)
	{
		input->Copy(m_value);
	}

private:
	std::vector<cr_real> m_value;
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
		if (index >= (int)m_images.size()) return false;
		input->Copy(m_images[index]);
		return true;
	}

	int GetLabel(int index)
	{
		if (index >= (int)m_labels.size()) return 0;
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

class CarpRobotSumTree
{
public:
	struct MemoryInfo
	{
		std::vector<cr_real> state;
		int action = 0;
		cr_real reward = 0.0f;
		std::vector<cr_real> next_state;
	};

public:
	CarpRobotSumTree(int memory_capacity)
	{
		// 保存容量
		m_memory_capacity = memory_capacity;

		// 计算最近的2幂次方
		m_memory_capacity = CeilToPowerOf2(m_memory_capacity);

		// 控制一下范围
		if (m_memory_capacity < 1024) m_memory_capacity = 1024;

		// 申请控件
		m_memory.reserve(m_memory_capacity);

		// 计算二叉树的层
		auto level = (int)(std::log(m_memory_capacity) / std::log(2) + 1);

		// 计算二叉树所有节点的个数
		auto count = (1 << level) - 1;
		m_tree.resize(count, 0);
	}

	uint32_t CeilToPowerOf2(uint32_t value)
	{
		value += value == 0 ? 1 : 0;
		value--;

		value |= value >> 1;
		value |= value >> 2;
		value |= value >> 4;
		value |= value >> 8;
		value |= value >> 16;

		value++;

		return value;
	}

	static std::default_random_engine& GetGen()
	{
		static thread_local std::unique_ptr<std::default_random_engine> gen;
		if (gen != nullptr) return *gen;
		std::random_device rd;
		gen = std::make_unique<std::default_random_engine>(rd());
		return *gen;
	}

	static cr_real GetRandFloat(cr_real min, cr_real max)
	{
		max -= std::numeric_limits<cr_real>::epsilon();
		if (min == max) return min;
		if (min > max) std::swap(min, max);
		std::uniform_real_distribution<cr_real> dis(min, max);  // [a, b)
		return dis(GetGen());
	}

	int SaveMemoryForLua(lua_State* l_state)
	{
		if (!lua_istable(l_state, 2))
		{
			lua_pushboolean(l_state, 0);
			return 1;
		}

		if (!lua_istable(l_state, 3))
		{
			lua_pushboolean(l_state, 0);
			return 1;
		}

		std::vector<cr_real> state;
		std::vector<cr_real> next_state;

		lua_pushnil(l_state);
		while (lua_next(l_state, 2) != 0)
		{
			const auto index = static_cast<int>(lua_tointeger(l_state, -2));
			const auto value = static_cast<cr_real>(lua_tonumber(l_state, -1));
			state.emplace_back(value);
			lua_pop(l_state, 1);
		}

		lua_pushnil(l_state);
		while (lua_next(l_state, 3) != 0)
		{
			const auto index = static_cast<int>(lua_tointeger(l_state, -2));
			const auto value = static_cast<cr_real>(lua_tonumber(l_state, -1));
			next_state.emplace_back(value);
			lua_pop(l_state, 1);
		}

		int action = static_cast<int>(luaL_checkinteger(l_state, 4));
		cr_real reward = static_cast<cr_real>(luaL_checknumber(l_state, 5));
		cr_real loss = static_cast<cr_real>(luaL_checknumber(l_state, 6));

		bool result = SaveMemory(state, next_state, action, reward, loss);
		lua_pushboolean(l_state, result ? 1 : 0);
		return 1;
	}

	// 返回是否存满
	bool SaveMemory(const std::vector<cr_real>& state, const std::vector<cr_real>& next_state, int action, cr_real reward, cr_real loss)
	{
		MemoryInfo info;
		info.state = state;
		info.next_state = next_state;
		info.action = action;
		info.reward = reward;

		size_t index = 0;
		if ((int)m_memory.size() < m_memory_capacity)
		{
			m_memory.emplace_back(info);
			index = m_memory.size() - 1;
		}
		else
		{
			index = m_memory_counter % m_memory_capacity;
			m_memory[index] = std::move(info);
			m_memory_counter++;
		}

		// 设置loss
		m_tree[index] = loss;

		// 从最底层不断更新上去
		int level_offset = 0;				// 当前层在m_tree中的偏移
		int level_count = m_memory_capacity; // 当前层的节点数量
		int level_index = (int)index;			// 当前层的节点偏移
		while (true)
		{
			// 获取左右节点的下标
			auto left = level_index;
			auto right = level_index + 1;
			if (level_index % 2 == 1)
			{
				left = level_index - 1;
				right = level_index;
			}
			left += level_offset;
			right += level_offset;

			// 获取父节点下标
			auto parent = level_index / 2 + level_offset + level_count;
			// 如果超出范围，那么就跳出
			if (parent >= m_tree.size()) break;
			// 更新父节点
			m_tree[parent] = m_tree[left] + m_tree[right];

			// 更新
			level_index = level_index / 2;
			level_offset += level_count;
			level_count = level_count >> 1;
		}

		return (int)m_memory.size() >= m_memory_capacity;
	}

	int SelectMemoryForLua(lua_State* l_state)
	{
		int count = static_cast<int>(luaL_checkinteger(l_state, 2));
		
		std::map<int, const MemoryInfo*> out_map;
		SelectMemory(count, out_map);

		lua_newtable(l_state);
		int index = 0;
		for (auto& pair : out_map)
		{
			lua_pushinteger(l_state, pair.first);
			lua_rawseti(l_state, -2, index + 1);
			++index;
		}
		return 1;
	}

	void GetState(int index, CarpRobotInput* input)
	{
		if (index >= m_memory.size()) return;
		input->Copy(m_memory[index].state);
	}

	void GetNextState(int index, CarpRobotInput* input)
	{
		if (index >= m_memory.size()) return;
		input->Copy(m_memory[index].next_state);
	}

	cr_real GetReward(int index, CarpRobotInput* input)
	{
		if (index >= m_memory.size()) return 0.0f;
		input->Copy(m_memory[index].reward);
		return m_memory[index].reward;
	}

	int GetAction(int index, CarpRobotLabel* label)
	{
		if (index >= m_memory.size()) return 0;
		label->Update(m_memory[index].action);
		return m_memory[index].action;
	}

	// 选择记忆
	void SelectMemory(int count, std::map<int, const MemoryInfo*>& out_map)
	{
		if (m_memory.empty()) return;

		// 取最大的那个数
		auto max = m_tree.back();

		// 根据数量分段
		auto split = max / count;

		auto cur_max = split;
		for (int i = 0; i < count; ++i)
		{
			auto score = GetRandFloat(cur_max - split, cur_max);

			// 开始选择
			int level_count = 1;
			int level_offset = (int)m_tree.size() - level_count;
			int level_index = 0;
			while (true)
			{
				// 这里说明已经到叶子节点了
				if (level_offset <= 0)
				{
					if (level_index < m_memory.size())
						out_map[level_index] = &m_memory[level_index];
					break;
				}

				auto parent_score = m_tree[level_index + level_offset];

				int left = level_index * 2;
				int right = left + 1;
				level_count = level_count << 1;
				level_offset -= level_count;

				auto left_score = m_tree[left + level_offset];
				auto right_score = m_tree[right + level_offset];

				// 如果比左边比当前分数大，那么就选左边
				if (left_score >= score)
				{
					level_index = left;
				}
				else
				{
					score -= left_score;
					level_index = right;
				}
			}

			cur_max += split;
		}
	}

private:
	std::vector<MemoryInfo> m_memory;
	int m_memory_capacity = 0;
	int m_memory_counter = 0;

private:
	std::vector<cr_real> m_tree;
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
			.addFunction("AsVectorAndMaxValue", &CarpRobotComputationGraph::AsVectorAndMaxValue)

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
			.beginClass<CarpRobotLabel>("CarpRobotLabel")
			.addConstructor<void(*)()>()
			.addFunction("Update", &CarpRobotLabel::Update)
			.endClass()
		
			.beginClass<CarpRobotMnist>("CarpRobotMnist")
			.addConstructor<void(*)()>()
			.addFunction("Load", &CarpRobotMnist::Load)
			.addFunction("GetImage", &CarpRobotMnist::GetImage)
			.addFunction("GetLabel", &CarpRobotMnist::GetLabel)
			.addFunction("GetCount", &CarpRobotMnist::GetCount)
			.endClass()

			.beginClass<CarpRobotSurface>("CarpRobotSurface")
			.addConstructor<void(*)()>()
			.addFunction("SetImage", &CarpRobotSurface::SetImage)
			.addFunction("GetImage", &CarpRobotSurface::GetImage)
			.endClass()

			.beginClass<CarpRobotSumTree>("CarpRobotSumTree")
			.addConstructor<void(*)(int)>()
			.addCFunction("SaveMemory", &CarpRobotSumTree::SaveMemoryForLua)
			.addCFunction("SelectMemory", &CarpRobotSumTree::SelectMemoryForLua)
			.addFunction("GetState", &CarpRobotSumTree::GetState)
			.addFunction("GetNextState", &CarpRobotSumTree::GetNextState)
			.addFunction("GetAction", &CarpRobotSumTree::GetAction)
			.addFunction("GetReward", &CarpRobotSumTree::GetReward)
			.endClass()

			.endNamespace();
	}
};

#endif