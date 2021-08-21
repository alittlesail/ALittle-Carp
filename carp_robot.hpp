#ifndef CARP_ROBOT_INCLUDED
#define CARP_ROBOT_INCLUDED

#include <string>
#include <fstream>
#include <sstream>
#include <vector>

// 定义异常宏
#define CARP_ROBOT_ASSERT(e, text) \
do { \
	if (!(e)) { std::ostringstream oss; oss << __FUNCTION__ << "() " << text; throw std::runtime_error(oss.str()); } \
} while (0)

// 定义浮点数类型
typedef float cr_real;

// 序列化
class CarpRobotModelSerializer
{
public:
	~CarpRobotModelSerializer() { Close(); }

public:
	// 打开和关闭文件
	bool Open(const std::string& model_path) { m_out_file.open(model_path, std::ios::binary | std::ios::trunc); return m_out_file.is_open(); }
	void Close() { if (m_out_file.is_open()) m_out_file.close(); }

public:
	// 写入字符串
	void WriteString(const std::string& str)
	{
		CARP_ROBOT_ASSERT(m_out_file.is_open(), u8"WriteString Failed: out_file open failed");
		int length = (int)str.size();
		m_out_file.write((char*)&length, sizeof(length));
		m_out_file.write(str.c_str(), length);
	}
	// 写入一个浮点数
	void WriteReal(cr_real value)
	{
		CARP_ROBOT_ASSERT(m_out_file.is_open(), u8"WriteReal Failed: out_file open failed");
		m_out_file.write((char*)&value, sizeof(cr_real));
	}
	// 写入一个浮点数数组
	void WriteRealArray(cr_real* value, size_t count)
	{
		CARP_ROBOT_ASSERT(m_out_file.is_open(), u8"WriteRealArray Failed: out_file open failed");
		int length = (int)count;
		m_out_file.write((char*)&length, sizeof(length));
		m_out_file.write((char*)value, sizeof(cr_real) * count);
	}
	// 写入一个整形数值
	void WriteInt(int value)
	{
		CARP_ROBOT_ASSERT(m_out_file.is_open(), u8"WriteInt Failed: out_file open failed");
		m_out_file.write((char*)&value, sizeof(int));
	}

private:
	std::ofstream m_out_file;
};

// 反序列化
class CarpRobotModelDeserializer
{
public:
	~CarpRobotModelDeserializer() { Close(); }

public:
	// 打开和关闭文件
	bool Open(const std::string& model_path) { m_in_file.open(model_path, std::ios::binary); return m_in_file.is_open(); }
	void Close() { if (m_in_file.is_open()) m_in_file.close(); }

public:
	// 读取字符串
	const char* ReadString()
	{
		CARP_ROBOT_ASSERT(m_in_file.is_open(), "ReadString Failed: in_file open failed");

		// 先读取大小
		int length = 0;
		m_in_file.read((char*)&length, sizeof(int));
		CARP_ROBOT_ASSERT(m_in_file.gcount() == sizeof(int), u8"ReadString Failed: length error 1");

		// 设置大小
		m_string.resize(length + 1);
		m_string[length] = 0;
		m_in_file.read(m_string.data(), length);
		CARP_ROBOT_ASSERT(m_in_file.gcount() == length, u8"ReadString Failed: length error 2");

		return m_string.data();
	}
	// 读取一个浮点数
	cr_real ReadReal()
	{
		CARP_ROBOT_ASSERT(m_in_file.is_open(), u8"ReadReal Failed: in_file open failed");

		cr_real value = 0;
		m_in_file.read((char*)&value, sizeof(cr_real));
		CARP_ROBOT_ASSERT(m_in_file.gcount() == sizeof(cr_real), u8"ReadReal Failed: length error 1");

		return value;
	}
	// 读取浮点数数组
	void ReadRealArray(cr_real* value, size_t count)
	{
		CARP_ROBOT_ASSERT(m_in_file.is_open(), u8"ReadRealArray Failed: in_file open failed");

		// 先读取大小，并检查
		int length = 0;
		m_in_file.read((char*)&length, sizeof(int));
		CARP_ROBOT_ASSERT(m_in_file.gcount() == sizeof(int), u8"ReadRealArray Failed: length error 1");
		CARP_ROBOT_ASSERT(length == count, u8"ReadRealArray Failed: length error 2");

		// 读取数组
		m_in_file.read((char*)value, sizeof(cr_real) * count);
		CARP_ROBOT_ASSERT(m_in_file.gcount() == sizeof(cr_real) * count, u8"ReadRealArray Failed: length error 1");
	}
	// 读取一个整型数
	int ReadInt()
	{
		CARP_ROBOT_ASSERT(m_in_file.is_open(), u8"ReadInt Failed: in_file open failed");

		int value = 0;
		m_in_file.read((char*)&value, sizeof(int));
		CARP_ROBOT_ASSERT(m_in_file.gcount() == sizeof(int), u8"ReadInt Failed: length error 1");

		return value;
	}

private:
	std::ifstream m_in_file;
	std::vector<char> m_string;
};

// 用来存储表达式的数据维度相关信息
class CarpRobotDim
{
public:
	// 构造函数，初始化
	CarpRobotDim() { }
	CarpRobotDim(const std::vector<int>& d) : m_d(d) { m_total = m_d.empty() ? 0 : 1; for (size_t i = 0; i < m_d.size(); ++i) m_total *= m_d[i]; }

public:
	// 获取总大小
	int GetTotalSize() const { return m_total; }

public:
	// 强制相等
	bool Equal(const CarpRobotDim& right) const { return m_total == right.m_total && m_d == right.m_d; }
	// 非强制相等
	bool SoftEqual(const CarpRobotDim& right) const
	{
		// 如果总大小不一致，那么肯定不一致了
		if (m_total != right.m_total) return false;
		// 取最小的那个，然后遍历，如果不相等肯定就不一致
		auto min_nd = std::min(m_d.size(), right.m_d.size());
		for (size_t i = 0; i < min_nd; ++i)
		{
			if (m_d[i] != right.m_d[i]) return false;
		}
		// 剩下比较大的那个，因为total一致，所以尾部都是1，也算是相等了
		return true;
	}

public:
	// 确认当前是二维时，获取第一个维度的大小
	int Rows() const { return m_d.empty() ? 0 : m_d[0]; }
	// 确认当前是二维时，获取第二个维度的大小，如果没有就返回1
	int Cols() const { return m_d.size() > 1 ? m_d[1] : 1; }
	// 获取几维
	int Count() const { return (int)m_d.size(); }
	// 随机存取
	int operator[](size_t index) const { return index < m_d.size() ? m_d[index] : 1; }

	// 序列化
	void Serialize(CarpRobotModelSerializer& file)
	{
		file.WriteInt((int)m_d.size());
		for (size_t i = 0; i < m_d.size(); ++i)
			file.WriteInt(m_d[i]);
	}
	// 反序列化
	void Deserialize(CarpRobotModelDeserializer& file)
	{
		m_d.resize(file.ReadInt());
		for (size_t i = 0; i < m_d.size(); ++i)
			m_d[i] = file.ReadInt();
		m_total = m_d.empty() ? 0 : 1;
		for (size_t i = 0; i < m_d.size(); ++i) m_total *= m_d[i];
	}
	// 转字符串
	std::string ToString() const
	{
		std::string result = "{";
		for (size_t i = 0; i < m_d.size(); ++i)
		{
			if (i) result += ",";
			result += std::to_string(m_d[i]);
		}
		result += "}";
		return result;
	}

private:
	// 用来存放维度的数组，比如是2×3×4，那么d数组的长度为3，第0个元素是2，第2个元素是3，第三个元素是4
	std::vector<int> m_d;
	// 总大小
	int m_total = 0;
};

#include <Eigen/Eigen/Eigen>
#include <Eigen/unsupported/Eigen/CXX11/Tensor>

class CarpRobotTensor
{
public:
	// 构造一个张量
	CarpRobotTensor() { }
	CarpRobotTensor(const CarpRobotDim& d, bool init_zero=false) { SetDim(d, init_zero); }
	CarpRobotTensor(const CarpRobotTensor& t) { Copy(t); }
	~CarpRobotTensor() { ReleaseMemory(); }
	void operator = (const CarpRobotTensor& t) { Copy(t); }

public:
	// 引用其他的Tensor内存
	void RefrenceTensor(const CarpRobotTensor& t)
	{
		if (&t == this) return;
		ReleaseMemory();
		m_dim = t.m_dim;
		m_value = t.m_value;
		m_shared = true;
	}
	// 直接引用内存
	void RefrenceMemory(const CarpRobotDim& dim, cr_real* value)
	{
		ReleaseMemory();
		m_dim = dim;
		m_value = value;
		m_shared = true;
	}

	// 复制
	void Copy(const CarpRobotTensor& t)
	{
		if (&t == this) return;

		ReleaseMemory();
		m_dim = t.m_dim;
		// 申请内存，并拷贝值
		auto total_bytes = m_dim.GetTotalSize() * sizeof(cr_real);
		if (total_bytes > 0 && t.m_value)
		{
			m_value = (cr_real*)malloc(total_bytes);
			memcpy(m_value, t.m_value, total_bytes);
		}
	}

	// 设置维度信息，
	void SetDim(const CarpRobotDim& dim, bool init_zero = false)
	{
		// 如果维度高度一致，只需要判断是否需要清零
		if (m_dim.Equal(dim))
		{
			if (init_zero && m_dim.GetTotalSize() > 0) Zero();
			return;
		}

		// 释放内存
		ReleaseMemory();
		m_dim = dim;

		// 获取总大小
		auto real_count = m_dim.GetTotalSize();
		if (real_count <= 0) return;

		// 申请内存
		m_value = (cr_real*)malloc(real_count * sizeof(cr_real));
		if (init_zero) Zero();
	}

	// 获取维度
	const CarpRobotDim& GetDim() const { return m_dim; }

public:
	// 转为字符串展示
	std::string ToString() const
	{
		std::string result = m_dim.ToString() + "\n";
		int total_size = m_dim.GetTotalSize();
		// 如果是二维，那么就进行二维展示
		if (m_dim.Count() == 2)
		{
			auto row_count = m_dim.Rows();
			auto col_count = m_dim.Cols();
			for (int row = 0; row < row_count; ++row)
			{
				for (int col = 0; col < col_count; ++col)
				{
					result += std::to_string(m_value[row * col_count + col]);
					if (col + 1 != col_count) result += ",";
				}
				if (row + 1 != row_count) result += "\n";
			}
		}
		// 否则直接使用一维展示
		else
		{
			for (int i = 0; i < total_size; ++i)
			{
				result += std::to_string(m_value[i]);
				if (i + 1 != total_size) result += ",";
			}
		}

		result += "\n";
		return result;
	}

private:
	// 释放内存
	void ReleaseMemory()
	{
		if (m_value && m_shared == false) free(m_value);
		m_value = nullptr;
		m_shared = false;
	}

private:
	bool m_shared = false;	// 是否是共用其他Tensor的内存
	cr_real* m_value = nullptr;		// 内存
	CarpRobotDim m_dim;		// 当前张量的维度信息

public:
	// 设置为常量
	void Constant(cr_real c) { tvec() = tvec().constant(c); }
	// 全部设置为0
	void Zero() { Constant(0); }
	// 随机数
	void RandomizeUniform(std::mt19937& gen, cr_real left=0.0f, cr_real right=1.0f)
	{
		std::uniform_real_distribution<cr_real> distribution(left, right);
		auto b = [&] { return distribution(gen); };
		std::generate(m_value, m_value + m_dim.GetTotalSize(), b);
	}

public:
	// 获取创建二维矩阵对象
	Eigen::Map<Eigen::MatrixXf> m() { return Eigen::Map<Eigen::MatrixXf>(m_value, m_dim.Rows(), m_dim.Cols()); }
	const Eigen::Map<Eigen::MatrixXf> m() const { return Eigen::Map<Eigen::MatrixXf>(m_value, m_dim.Rows(), m_dim.Cols()); }

	// 使用当前数据，创建一个只有一维的Tensor
	Eigen::TensorMap<Eigen::Tensor<cr_real, 1>> tvec() { return Eigen::TensorMap<Eigen::Tensor<cr_real, 1>>(m_value, m_dim.GetTotalSize()); }
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 1>> tvec() const { return Eigen::TensorMap<Eigen::Tensor<cr_real, 1>>(m_value, m_dim.GetTotalSize()); }

	// 使用当前数据，创建一个Order维度的Tensor对象
	template <int Order>
	Eigen::TensorMap<Eigen::Tensor<cr_real, Order>> t()
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= Order, u8"非法维度信息 t<" << Order << ">(): dim=" << m_dim.ToString());
		std::array<int, Order> dim; for (int i = 0; i < Order; ++i) dim[i] = m_dim[i];
		return Eigen::TensorMap<Eigen::Tensor<cr_real, Order>>(m_value, dim);
	}
	template <int Order>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, Order>> t() const
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= Order, u8"非法维度信息 t<" << Order << ">(): dim=" << m_dim.ToString());
		std::array<int, Order> dim; for (int i = 0; i < Order; ++i) dim[i] = m_dim[i];
		return Eigen::TensorMap<Eigen::Tensor<cr_real, Order>>(m_value, dim);
	}

	template <>
	Eigen::TensorMap<Eigen::Tensor<cr_real, 1>> t()
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= 1, u8"非法维度信息 t<1>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 1>>(m_value, m_dim[0]);
	}
	template <>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 1>> t() const
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= 1, u8"非法维度信息 t<1>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 1>>(m_value, m_dim[0]);
	}

	template <>
	Eigen::TensorMap<Eigen::Tensor<cr_real, 2>> t()
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= 2, u8"非法维度信息 t<2>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 2>>(m_value, m_dim[0], m_dim[1]);
	}
	template <>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 2>> t() const
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= 2, u8"非法维度信息 t<2>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 2>>(m_value, m_dim[0], m_dim[1]);
	}

	template <>
	Eigen::TensorMap<Eigen::Tensor<cr_real, 3>> t()
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= 3, u8"非法维度信息 t<3>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 3>>(m_value, m_dim[0], m_dim[1], m_dim[2]);
	}
	template <>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 3>> t() const
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= 3, u8"非法维度信息 t<3>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 3>>(m_value, m_dim[0], m_dim[1], m_dim[2]);
	}

	template <>
	Eigen::TensorMap<Eigen::Tensor<cr_real, 4>> t()
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= 4, u8"非法维度信息 t<4>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 4>>(m_value, m_dim[0], m_dim[1], m_dim[2], m_dim[3]);
	}
	template <>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 4>> t() const
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= 4, u8"非法维度信息 t<4>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 4>>(m_value, m_dim[0], m_dim[1], m_dim[2], m_dim[3]);
	}

public:
	// 当作标量返回
	cr_real AsScalar() const { CARP_ROBOT_ASSERT(m_dim.GetTotalSize() == 1, u8"当前Tensor不是标量"); return m_value[0]; }
	
public:
	void Serialize(CarpRobotModelSerializer& file) { m_dim.Serialize(file); file.WriteRealArray(m_value, m_dim.GetTotalSize()); }
	void Deserialize(CarpRobotModelDeserializer& file) { CarpRobotDim tmp; tmp.Deserialize(file); SetDim(tmp); file.ReadRealArray(m_value, m_dim.GetTotalSize()); }
};

class ICarpRobotParameter
{
public:
	virtual ~ICarpRobotParameter() {}

public:
	// 获取参数名
	virtual const std::string& GetFullName() const = 0;
};

class CarpRobotParameter : public ICarpRobotParameter
{
public:
	CarpRobotParameter(const CarpRobotDim& dim, const std::string& name)
		: m_name(name)
	{
		m_values.SetDim(dim, true);
		m_grad.SetDim(dim, true);
		m_grad.Zero();
	}
	virtual ~CarpRobotParameter() {}

public:
	// 累加权重 grad <- grad + d
	void AccumulateGrad(const CarpRobotTensor& d)
	{
		// 标记梯度不为0
		m_has_grad = true;

		// 累加梯度
		m_grad.tvec() += d.tvec();
	}

	// 误差项设置为0
	void Clear()
	{
		// 误差项置标记为是0
		m_has_grad = false;

		// 如果m_grad的值存在就置0
		if (m_grad.GetDim().GetTotalSize() > 0) m_grad.Zero();
	}

	// 获取张量
	CarpRobotTensor& GetValue() { return m_values; }
	// 获取梯度
	CarpRobotTensor& GetGradient() { return m_grad; }

public:
	// 获取参数名
	virtual const std::string& GetFullName() const override { return m_name; }

public:
	// 序列化
	void Serialize(CarpRobotModelSerializer& file, const char* name)
	{
		file.WriteString(name ? name : "Parameter");
		m_values.Serialize(file);
	}
	// 反序列化
	void Deserialize(CarpRobotModelDeserializer& file, const char* name)
	{
		std::string name_copy = "Parameter";
		if (name) name_copy = name;
		std::string read_name = file.ReadString();
		CARP_ROBOT_ASSERT(read_name == name_copy, u8"反序列化名称错误, 当前是:" << read_name << "应当是:" << name_copy);
		m_values.Deserialize(file);
	}

private:
	std::string m_name = "Parameter";				// 当前参数的名字，用来调试用的
	CarpRobotTensor m_values;				// 当前参数的值
	CarpRobotTensor m_grad;					// 当前参数的误差项
	bool m_has_grad = false;			// 是否有误差项
};

class CarpRobotParameterCollection
{
public:
	CarpRobotParameterCollection() : m_gen(m_rd()) { }
	virtual ~CarpRobotParameterCollection()
	{
		for (size_t i = 0; i < m_params.size(); ++i)
			delete m_params[i];
	}

public:
	virtual CarpRobotParameter* AddParameters(const CarpRobotDim& d, const std::string& name = "")
	{
		auto* p = new CarpRobotParameter(d, name);
		p->GetValue().RandomizeUniform(m_gen);
		m_params.push_back(p);
		return p;
	}

	virtual const std::vector<CarpRobotParameter*>& GetParameters() const { return m_params; }

public:
	// 获取参数收集器名字
	const std::string& GetFullName() const { return m_name; }

	std::string ToString() const
	{
		std::string result;
		for (size_t i = 0; i < m_params.size(); ++i)
		{
			result += m_params[i]->GetFullName() + "\n";
			result += m_params[i]->GetValue().ToString() + "\n";
		}

		return result;
	}

private:
	std::vector<CarpRobotParameter*> m_params;					// Parameter参数的集合
	
private:
	std::string m_name;									// 收集器名字
	std::random_device m_rd;
	std::mt19937 m_gen;
};

// 运算节点
class CarpRobotNode
{
public:
	CarpRobotNode() {};
	CarpRobotNode(const std::vector<int>& args) : m_args(args) {}
	virtual ~CarpRobotNode() {}

public:
	// 获取参数下标
	const std::vector<int>& GetArgs() const { return m_args; }

public:
	// 执行向前计算，对Batch进行处理，然后调用ForwardImpl
	virtual void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) = 0;
	// 执行反向计算，对Batch进行处理，然后调用BackwardImpl
	virtual void Backward(const std::vector<const CarpRobotTensor*>& xs,	// 该节点的输入值
		const CarpRobotTensor& fx,											// 该节点的输出值
		const CarpRobotTensor& dEdf,										// 该节点的输出节点的误差项
		unsigned int xs_i,													// 该节点的输入节点索引
		CarpRobotTensor& dEdxi) = 0;									// 输入节点的误差项
	
private:
	// 保存输入节点的下标
	std::vector<int> m_args;
};

// 矩阵输入节点
class CarpRobotInputNode : public CarpRobotNode
{
public:
	// 值拷贝
	CarpRobotInputNode(const CarpRobotDim& dim, const std::vector<cr_real>& data) : m_dim(dim), m_data(data), m_pdata(&m_data) {}
	// 地址拷贝
	CarpRobotInputNode(const CarpRobotDim& dim, std::vector<cr_real>* pdata) : m_dim(dim), m_pdata(pdata) {}

protected:
	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 0, "Failed dimension");
		// 直接引用内存
		fx.RefrenceMemory(m_dim, m_pdata->data());
	}
	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		unsigned int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		// 输入节点，不能反向传播
		CARP_ROBOT_ASSERT(0, "called backward() on arity 0 node: i = " << xs_i);
	}

public:
	CarpRobotDim m_dim;
	std::vector<cr_real> m_data;
	std::vector<cr_real>* m_pdata;
};

// 标量输入节点
class CarpRobotScalarInputNode : public CarpRobotNode
{
public:
	// 值拷贝
	CarpRobotScalarInputNode(cr_real scalar) : m_data(scalar), m_pdata(&m_data), m_dim({ 1 }) {}
	// 地址拷贝
	CarpRobotScalarInputNode(cr_real* pscalar) : m_pdata(pscalar), m_dim({ 1 }) {}

protected:
	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 0, "Failed dimension");
		// 直接引用内存
		fx.RefrenceMemory(m_dim, m_pdata);
	}
	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		unsigned int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		// 输入节点，不能反向传播
		CARP_ROBOT_ASSERT(0, "called backward() on arity 0 node: i = " << xs_i);
	}

public:
	CarpRobotDim m_dim;
	cr_real m_data = 0;
	cr_real* m_pdata = nullptr;
};

// 参数节点输入
class ICarpRobotParameterNode : public CarpRobotNode
{
public:
	virtual ~ICarpRobotParameterNode() {}

public:
	// 反向传播
	virtual void AccumulateGrad(const CarpRobotTensor& graph) {}
};

// 普通参数输入节点
class CarpRobotParameterNode : public ICarpRobotParameterNode
{
public:
	CarpRobotParameterNode(CarpRobotParameter* p) : m_params(p) {}

public:
	// 执行反向传递梯度
	void AccumulateGrad(const CarpRobotTensor& g) override { m_params->AccumulateGrad(g); }

protected:
	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		fx.SetDim(m_params->GetValue().GetDim());
		fx.tvec() = m_params->GetValue().tvec();
	}
	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		unsigned int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		CARP_ROBOT_ASSERT(0, "called backward() on arity 0 node: i = " << xs_i);
	}

private:
	CarpRobotParameter* m_params = nullptr;
};

// 常量参数输入节点
class CarpRobotConstParameterNode : public CarpRobotNode
{
public:
	CarpRobotConstParameterNode(CarpRobotParameter* p) : m_params(p) { }

protected:
	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		fx.SetDim(m_params->GetValue().GetDim());
		fx.tvec() = m_params->GetValue().tvec();
	}
	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		unsigned int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		CARP_ROBOT_ASSERT(0, "called backward() on arity 0 node: i = " << xs_i);
	}

private:
	CarpRobotParameter* m_params = nullptr;
};

// 一个张量求负数
// 正向函数 y = -x_1
// 求导函数 dx_1 = -1
class CarpRobotNegateNode : public CarpRobotNode
{
public:
	CarpRobotNegateNode(const std::vector<int>& a) : CarpRobotNode(a) {}

protected:
	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"CarpRobotNegateNode 必须是一个输入");
		// v的每个元素取负数
		fx.SetDim(xs[0]->GetDim());
		fx.tvec() = -xs[0]->tvec();
	}
	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		unsigned int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		// 正规要这么写dEdxi.tvec() += dEdf.tvec() * -1; 但是为了减少一个乘法运算，于是写成下面那样
		dEdxi.tvec() -= dEdf.tvec();
	}
};

// 两个张量加法
// 正向函数 y = x_1 + x_2
// 求导函数 dx_1 = 1
// 求导函数 dx_2 = 1
class CarpRobotCwiseSumNode : public CarpRobotNode
{
public:
	CarpRobotCwiseSumNode(const std::vector<int>& a) : CarpRobotNode(a) {}

protected:
	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 2, u8"CarpRobotCwiseSumNode 必须是两个输入");
		CARP_ROBOT_ASSERT(xs[0]->GetDim().SoftEqual(xs[1]->GetDim()), u8"CarpRobotCwiseSumNode 两个张量的维度必须一致, xs[0]:" << xs[0]->GetDim().ToString() << " != xs[1]:" << xs[1]->GetDim().ToString());

		fx.SetDim(xs[0]->GetDim());
		fx.tvec() = xs[0]->tvec() + xs[1]->tvec();
	}
	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		unsigned int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		// 正规要这么写dEdxi.tvec() += dEdf.tvec() * 1; 但是为了减少一个乘法运算，于是写成下面那样
		dEdxi.tvec() += dEdf.tvec();
	}
};

// 常量 加上 张量
// 正向函数 y = c + x_1
// 求导函数 dx_1 = 1;
class CarpRobotConstantPlusXNode : public CarpRobotNode
{
public:
	CarpRobotConstantPlusXNode(const std::vector<int>& a, cr_real v) : CarpRobotNode(a), m_value(v) {}

protected:
	struct ConstAddOp
	{
		ConstAddOp(cr_real c) : v(c) {}
		inline const cr_real operator() (cr_real x) const { return v + x; }
		cr_real v;
	};

	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"CarpRobotConstantPlusXNode 必须是一个输入");
		
		fx.SetDim(xs[0]->GetDim());
		// unaryExpr表示对xs[0]->tvec()所有元素进行遍历，每个元素都进行ConstAddOp操作
		fx.tvec() = xs[0]->tvec().unaryExpr(ConstAddOp(m_value));
	}

	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		unsigned int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		// 正规要这么写dEdxi.tvec() += dEdf.tvec() * 1; 但是为了减少一个乘法运算，于是写成下面那样
		dEdxi.tvec() += dEdf.tvec();
	}

private:
	cr_real m_value;
};

// 常量 减 张量
// 正向函数 y = c - x_1
// 求导函数 dx_1 = -1;
class CarpRobotConstantMinusXNode : public CarpRobotNode
{
public:
	CarpRobotConstantMinusXNode(const std::vector<int>& a, cr_real v) : CarpRobotNode(a), m_value(v) {}

protected:
	struct ConstMinusOp
	{
		ConstMinusOp(cr_real c) : v(c) {}
		inline const cr_real operator() (cr_real x) const { return v - x; }
		cr_real v;
	};
	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"CarpRobotConstantMinusXNode 必须是一个输入");

		fx.SetDim(xs[0]->GetDim());
		// unaryExpr表示对xs[0]->tvec()所有元素进行遍历，每个元素都进行ConstAddOp操作
		fx.tvec() = xs[0]->tvec().unaryExpr(ConstMinusOp(m_value));
	}
	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		unsigned int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		// 正规要这么写dEdxi.tvec() += dEdf.tvec() * -1; 但是为了减少一个乘法运算，于是写成下面那样
		dEdxi.tvec() -= dEdf.tvec();
	}

private:
	cr_real m_value;
};

// 两个矩阵相乘
// 正向函数 y = x_1 * x_2
// 求导函数 dx_1 = dy * x_2^T
// 求导函数 dx_2 = x_1^T * dy
class CarpRobotMatrixMultiplyNode : public CarpRobotNode
{
public:
	CarpRobotMatrixMultiplyNode(const std::vector<int>& a) : CarpRobotNode(a) {}

protected:
	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 2, "CarpRobotMatrixMultiplyNode 必须是两个输入");
		CARP_ROBOT_ASSERT(xs[0]->GetDim().Cols() == xs[1]->GetDim().Rows(), u8"CarpRobotMatrixMultiplyNode 前项的列必须等于后项的行");
		CARP_ROBOT_ASSERT(xs[0]->GetDim().Count() <= 2 && xs[1]->GetDim().Count() <= 2, "CarpRobotMatrixMultiplyNode 矩阵相乘最多是2维的");

		CarpRobotDim dim({ xs[0]->GetDim().Rows(), xs[1]->GetDim().Cols() });
		fx.SetDim(dim);
		fx.m().noalias() = xs[0]->m() * xs[1]->m();
	}
	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		unsigned int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		CARP_ROBOT_ASSERT(xs_i < 2, "CarpRobotMatrixMultiplyNode 必须是两个输入");
		if (xs_i == 0)
			dEdxi.m().noalias() += dEdf.m() * xs[1]->m().transpose();
		else
			dEdxi.m().noalias() += xs[0]->m().transpose() * dEdf.m();
	}
};

// 常量 乘 张量
// 正向函数 y = c * x_1
// 求导函数 dx_1 = c
class CarpRobotConstScalarMultiplyNode : public CarpRobotNode
{
public:
	CarpRobotConstScalarMultiplyNode(const std::vector<int>& a, cr_real v) : CarpRobotNode(a), m_value(v) {}

protected:
	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, "CarpRobotConstScalarMultiplyNode 必须是一个输入");
		fx.SetDim(xs[0]->GetDim());
		fx.tvec() = xs[0]->tvec() * m_value;
	}
	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		unsigned int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		CARP_ROBOT_ASSERT(xs_i == 0, "CarpRobotConstScalarMultiplyNode 必须是一个输入");
		dEdxi.tvec() += dEdf.tvec() * m_value;
	}

private:
	cr_real m_value;
};

// 张量 除 张量
// 正向函数 y = x_1 / x_2
// 求导函数 dx_1 = 1 / x_2
// 求导函数 dx_2 = - (dy / x_2^2 * x_1)
class CarpRobotCwiseQuotientNode : public CarpRobotNode
{
public:
	CarpRobotCwiseQuotientNode(const std::vector<int>& a) : CarpRobotNode(a) {}

protected:
	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 2, u8"CarpRobotCwiseQuotientNode 必须是两个输入");
		CARP_ROBOT_ASSERT(xs[0]->GetDim().SoftEqual(xs[1]->GetDim()), u8"CarpRobotCwiseQuotientNode 两个张量的维度必须一致, xs[0]:" << xs[0]->GetDim().ToString() << " != xs[1]:" << xs[1]->GetDim().ToString());

		fx.SetDim(xs[0]->GetDim());

		// 因为两个输入的张量维度一致
		// 又因为t模板函数支持最高到4个维度
		fx.t<4>() = xs[0]->t<4>() / xs[1]->t<4>();
	}
	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		unsigned int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		if (xs_i == 0)
			dEdxi.t<4>() += dEdf.t<4>() / xs[1]->t<4>();
		else
			dEdxi.t<4>() -= dEdf.t<4>() / xs[1]->t<4>().square() * xs[0]->t<4>();
	}
};

// y = \sigma(x_1)
class CarpRobotLogisticSigmoidNode : public CarpRobotNode
{
public:
	CarpRobotLogisticSigmoidNode(const std::vector<int>& a) : CarpRobotNode(a) {}
	~CarpRobotLogisticSigmoidNode() {}

protected:
	struct ScalarLogisticSigmoidOp
	{
		inline const cr_real operator() (cr_real x) const { return 1.0f / (1.0f + std::exp(-x)); }
	};
	struct ScalarLogisticSigmoidBackwardOp
	{
		inline const cr_real operator() (cr_real t, cr_real d) const { return (1.0f - t) * t * d; }
	};

	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx)
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"LogisticSigmoidNode 必须是一个输入");
		fx.SetDim(xs[0]->GetDim());
		fx.tvec() = xs[0]->tvec().unaryExpr(ScalarLogisticSigmoidOp());
	}
	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		unsigned int xs_i,
		CarpRobotTensor& dEdxi)
	{
		dEdxi.tvec() += fx.tvec().binaryExpr(dEdf.tvec(), ScalarLogisticSigmoidBackwardOp());
	}
};

// y = x_1 \odot x_1
class CarpRobotSquareNode : public CarpRobotNode
{
public:
	CarpRobotSquareNode(const std::vector<int>& a) : CarpRobotNode(a) {}
	~CarpRobotSquareNode() {}

public:
	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"CarpRobotSquareNode 必须是一个输入");
		fx.SetDim(xs[0]->GetDim());
		fx.tvec() = xs[0]->tvec().square();
	}
	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		unsigned int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		dEdxi.tvec() += dEdf.tvec() * xs[0]->tvec() * 2.f;
	}
};

// y = ty * log(x_1) + (1 - ty) * log(x_1)
class CarpRobotBinaryLogLossNode : public CarpRobotNode
{
public:
	CarpRobotBinaryLogLossNode(const std::vector<int>& a) : CarpRobotNode(a) {}
	~CarpRobotBinaryLogLossNode() {}

protected:
	struct FBinaryLogLoss
	{
		inline cr_real operator()(cr_real x, cr_real x_true) const
		{
			const cr_real READ_MIN_VALUE = 1.175494351e-38f;
			if (x_true == 1.f)
			{
				if (x == 0.f) return -1.f * std::log(READ_MIN_VALUE);
				return -1.f * std::log(x);
			}
			else if (x_true == 0.f)
			{
				if (x == 1.f) return -1.f * std::log(READ_MIN_VALUE);
				else return (x_true - 1.f) * std::log1pf(-x);
			}
			else
			{
				if (x == 0.f) return -1.f * std::log(READ_MIN_VALUE);
				else if (x == 1.f) return -1.f * std::log(READ_MIN_VALUE);
				else return -1.f * (x_true * std::log(x) + (1.f - x_true) * std::log1pf(-x));
			}
		}
	};

	struct FBinaryLogLossBackward
	{
		FBinaryLogLossBackward(cr_real d) : d(d) {}
		inline cr_real operator()(cr_real x, cr_real x_true) const
		{
			const cr_real READ_MIN_VALUE = 1.175494351e-38f;

			if (x == x_true) return 0;
			if (x == 0.f) x = READ_MIN_VALUE;
			if (x == 1.f) x = 0.9999999f;
			if (x_true == 1.f)
				return d * -x_true / x;
			else if (x_true == 0.f)
				return d * (1.f - x_true) / (1.f - x);
			return d * ((1.f - x_true) / (1.f - x) + (-x_true / x));
		}
		cr_real d;
	};

	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 2, u8"CarpRobotBinaryLogLossNode 必须是两个输入");
		CARP_ROBOT_ASSERT(xs[0]->GetDim().Rows() == 2 || xs[0]->GetDim().Count() == 1, u8"输入维度信息错误");
		CARP_ROBOT_ASSERT(xs[1]->GetDim().Rows() == 2 || xs[1]->GetDim().Count() == 1, u8"输入维度信息错误");

		fx.SetDim(CarpRobotDim({ 1 }));
		fx.t<0>() = xs[0]->tvec().binaryExpr(xs[1]->tvec(), FBinaryLogLoss()).sum();
	}
	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		unsigned int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		dEdxi.tvec() += xs[xs_i]->tvec().binaryExpr(xs[1 - xs_i]->tvec(), FBinaryLogLossBackward(dEdf.AsScalar()));
	}
};

class ICarpRobotComputationGraph
{
public:
	virtual ~ICarpRobotComputationGraph() {}
	virtual const CarpRobotTensor& GetValue(int i) = 0;
	virtual const CarpRobotTensor& GetGradient(int i) const = 0;
	virtual int AddNode(CarpRobotNode* node) = 0;
};

class CarpRobotExpression
{
public:
	CarpRobotExpression() {}
	CarpRobotExpression(ICarpRobotComputationGraph* graph, int index) : m_graph(graph), m_index(index) {}

public:
	const CarpRobotTensor& GetValue() const { return m_graph->GetValue(m_index); }
	const CarpRobotTensor& GetGradient() const { return m_graph->GetGradient(m_index); }
	
public:
	ICarpRobotComputationGraph* GetGraph() const { return m_graph; }
	int GetIndex() const { return m_index; }

public:
	CarpRobotExpression Square() { std::vector<int> args; args.push_back(m_index); return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotSquareNode(args))); }
	CarpRobotExpression Sigmoid() { std::vector<int> args; args.push_back(m_index); return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotLogisticSigmoidNode(args))); }

private:
	ICarpRobotComputationGraph* m_graph = nullptr;
	int m_index = 0;
};

// 1. 计算图，保存所有节点的拓扑结构
// 2. 前向计算，反向传播
class CarpRobotComputationGraph : public ICarpRobotComputationGraph
{
public:
	// 构造函数，初始化
	CarpRobotComputationGraph() { }
	~CarpRobotComputationGraph() { Clear(); }

public:
	// 添加一个标量输入节点
	CarpRobotExpression AddInput(cr_real scalar) { return CarpRobotExpression(this, AddNode(new CarpRobotScalarInputNode(scalar))); }
	CarpRobotExpression AddInput(cr_real* pscalar) { return CarpRobotExpression(this, AddNode(new CarpRobotScalarInputNode(pscalar))); }

	// 添加指定结构的输入
	CarpRobotExpression AddInput(const CarpRobotDim& dim, const std::vector<cr_real>& data) { return CarpRobotExpression(this, AddNode(new CarpRobotInputNode(dim, data))); }
	CarpRobotExpression AddInput(const CarpRobotDim& dim, std::vector<cr_real>* pdata) { return CarpRobotExpression(this, AddNode(new CarpRobotInputNode(dim, pdata))); }

public:
	// 添加一个Parameter参数节点
	CarpRobotExpression AddParameters(CarpRobotParameter* p) { return CarpRobotExpression(this, AddParameterNode(new CarpRobotParameterNode(p))); }
	// 添加一个Parameter常量参数节点
	CarpRobotExpression AddConstParameters(CarpRobotParameter* p) { return CarpRobotExpression(this, AddNode(new CarpRobotConstParameterNode(p))); }

public:
	// 添加一个计算节点
	int AddNode(CarpRobotNode* node) override
	{
		// 添加到节点列表
		m_nodes.push_back(node);
		// 初始化维度，并且申请内存
		m_fx_list.emplace_back(CarpRobotTensor());

		// 如果需要世界计算，那么进行向前计算
		if (m_immediate_compute) Forward();
		// 返回索引
		return (int)m_nodes.size() - 1;
	}

	// 添加一个参数节点
	int AddParameterNode(ICarpRobotParameterNode * node)
	{
		// 添加到图
		int index = AddNode(node);
		// 标记参数节点
		m_parameter_map[index] = node;
		// 返回索引
		return index;
	}

public:
	const CarpRobotTensor& GetValue(int i) override
	{
		Forward();
		CARP_ROBOT_ASSERT(i < (int)m_fx_list.size(), u8"访问越界");
		return m_fx_list[i];
	}
	const CarpRobotTensor& GetGradient(int i) const  override
	{
		CARP_ROBOT_ASSERT(i < (int)m_dEdf_list.size(), u8"访问越界");
		return m_dEdf_list[i];
	}

public:
	// 清理所有信息
	void Clear()
	{
		for (unsigned int i = 0; i < m_nodes.size(); ++i)
			delete m_nodes[i];
		m_nodes.resize(0);
		m_parameter_map.clear();
		m_fx_list.resize(0);
		m_dEdf_list.resize(0);
		m_evaluated_index = 0;
	}

	// 拓扑结构不变，从头开始算
	void Invalidate() { m_evaluated_index = 0; }

	// 设置是否直接计算节点
	void SetImmediateCompute(bool immediate) { m_immediate_compute = immediate; }

public:
	// 向前传播
	const CarpRobotTensor& Forward()
	{
		CARP_ROBOT_ASSERT(m_nodes.size() > 0, u8"当前没有节点，无法向前计算");

		// 定义在外面，避免频繁构造
		std::vector<const CarpRobotTensor*> xs;

		// 从当前节点向前计算，直到最后
		while (m_evaluated_index < (int)m_nodes.size())
		{
			// 获取当前计算节点
			auto* node = m_nodes[m_evaluated_index];

			// 定义输入数组，并且获取输入参数
			const auto& args = node->GetArgs();
			xs.resize(args.size());
			for (size_t i = 0; i < args.size(); ++i)
				xs[i] = &m_fx_list[args[i]];

			// 执行节点的前向计算
			node->Forward(xs, m_fx_list[m_evaluated_index]);

			// 向前走一步
			++m_evaluated_index;
		}

		return m_fx_list[m_evaluated_index - 1];
	}

	// 反向传播
	void Backward()
	{
		// 执行向前计算，保证所有节点全部计算完毕
		Forward();

		// 获取当前节点个数
		int num_nodes = (int)m_nodes.size();

		// 设置误差项大小
		m_dEdf_list.resize(num_nodes);

		// 初始化维度信息和内存，然后清零
		for (size_t i = 0; i < m_dEdf_list.size(); ++i)
			m_dEdf_list[i].SetDim(m_fx_list[i].GetDim(), true);
		m_dEdf_list[num_nodes - 1].Constant(1);

		// 参数节点需要反向传播
		std::vector<bool> needs_derivative(num_nodes, false);
		for (auto& pair : m_parameter_map)
			needs_derivative[pair.first] = true;

		// 把拥有参数节点的节点，设置为需要反向传播
		for (int i = 0; i < num_nodes; ++i)
		{
			// 如果这个节点已经被定位需要求导，那么就直接跳过
			bool need = needs_derivative[i];
			if (need) continue;

			// 如果输入节点的某个节点需要求导，那么本节点也需要求导
			const auto* node = m_nodes[i];
			const auto& args = node->GetArgs();
			for (size_t j = 0; j < args.size(); ++j)
			{
				if (needs_derivative[args[j]])
				{
					need = true;
					break;
				}
			}

			// 设置为是否需要求导
			needs_derivative[i] = need;
		}

		// 只考虑参与计算的节点，默认不参与计算
		std::vector<bool> in_computation(num_nodes, false);
		in_computation[num_nodes - 1] = true;	// 标记最后一个节点是要参与计算的

		// 保存输入节点
		std::vector<const CarpRobotTensor*> xs;
		for (int i = num_nodes - 1; i >= 0; --i)
		{
			// 如果这个节点不参与计算，直接跳过
			if (!in_computation[i]) continue;

			// 获取节点
			auto* node = m_nodes[i];

			// 获取该节点的输入
			const auto& args = node->GetArgs();
			xs.resize(args.size());
			for (size_t j = 0; j < args.size(); ++j)
			{
				const int arg = args[j];

				// 标记这个节点为参与计算的节点
				in_computation[arg] = true;

				// 设置输入节点
				xs[j] = &m_fx_list[arg];
			}

			// 遍历所有输入节点，需要求导的输入都要进行反向传播
			for (size_t j = 0; j < args.size(); ++j)
			{
				const int arg = args[j];
				if (needs_derivative[arg])
					node->Backward(xs, m_fx_list[i], m_dEdf_list[i], (unsigned int)j, m_dEdf_list[arg]);
			}
		}

		// 把最后的结果设置到节点上去
		for (auto& pair : m_parameter_map)
			pair.second->AccumulateGrad(m_dEdf_list[pair.first]);
	}

private:
	std::vector<CarpRobotNode*> m_nodes;			// 用于存放计算图的拓扑结构
	std::map<int, ICarpRobotParameterNode*> m_parameter_map;	// 记录参数节点的索引

private:
	std::vector<CarpRobotTensor> m_fx_list;			// 保存前向计算的结果
	std::vector<CarpRobotTensor> m_dEdf_list;		// 保存反向计算的结果
	int m_evaluated_index = 0;				// 记录当前计算到那个节点
	bool m_immediate_compute = false;			// 是否立即计算
};

CarpRobotExpression operator-(const CarpRobotExpression& x) { return CarpRobotExpression(x.GetGraph(), x.GetGraph()->AddNode( new CarpRobotNegateNode({ x.GetIndex() }))); }
CarpRobotExpression operator+(const CarpRobotExpression& x, const CarpRobotExpression& y) { return CarpRobotExpression(x.GetGraph(), x.GetGraph()->AddNode(new CarpRobotCwiseSumNode({ x.GetIndex(), y.GetIndex() }))); }
CarpRobotExpression operator+(cr_real x, const CarpRobotExpression& y) { return CarpRobotExpression(y.GetGraph(), y.GetGraph()->AddNode(new CarpRobotConstantPlusXNode({ y.GetIndex() }, x))); }
CarpRobotExpression operator+(const CarpRobotExpression& x, cr_real y) { return y + x; }
CarpRobotExpression operator-(const CarpRobotExpression& x, const CarpRobotExpression& y) { return x + (-y); }
CarpRobotExpression operator-(cr_real x, const CarpRobotExpression& y) { return CarpRobotExpression(y.GetGraph(), y.GetGraph()->AddNode(new CarpRobotConstantMinusXNode({ y.GetIndex() }, x))); }
CarpRobotExpression operator-(const CarpRobotExpression& x, cr_real y) { return -(y - x); }
CarpRobotExpression operator*(const CarpRobotExpression& x, const CarpRobotExpression& y) { return CarpRobotExpression(x.GetGraph(), x.GetGraph()->AddNode(new CarpRobotMatrixMultiplyNode({ x.GetIndex(), y.GetIndex() }))); }
CarpRobotExpression operator*(const CarpRobotExpression& x, cr_real y) { return CarpRobotExpression(x.GetGraph(), x.GetGraph()->AddNode(new CarpRobotConstScalarMultiplyNode({ x.GetIndex() }, y))); }
CarpRobotExpression operator/(const CarpRobotExpression& x, const CarpRobotExpression& y) { return CarpRobotExpression(x.GetGraph(), x.GetGraph()->AddNode(new CarpRobotCwiseQuotientNode({ x.GetIndex(), y.GetIndex() }))); }

class CarpRobotTrainer
{
public:
	CarpRobotTrainer(CarpRobotParameterCollection& model, cr_real learning_rate)
		: m_model(&model)
		, m_learning_rate(learning_rate)
	{}
	virtual ~CarpRobotTrainer() {}

public:
	// 根据训练器规则，将地图应用到参数中
	virtual void Update()
	{
		// 获取parameters列表
		const std::vector<CarpRobotParameter*>& parameters = m_model->GetParameters();
		// 遍历parameters列表
		for (size_t i = 0; i < parameters.size(); ++i)
		{
			// 按照gscale更新这个参数
			UpdateParameter(parameters[i]);
			// 将梯度值清零
			parameters[i]->Clear();
		}
	}

	// 重置训练器的缓存数据
	virtual void Restart() {}

protected:
	cr_real m_learning_rate = 0.1f;	// 学习率
	CarpRobotParameterCollection* m_model = nullptr;		// ParameterCollection对象

protected:
	// 基类不允许构造
	CarpRobotTrainer() {}

	// 更新参数
	virtual void UpdateParameter(CarpRobotParameter* parameter) = 0;
};

class CarpRobotSGDTrainer : public CarpRobotTrainer
{
public:
	CarpRobotSGDTrainer(CarpRobotParameterCollection& model, cr_real learning_rate) : CarpRobotTrainer(model, learning_rate) {}
protected:
	void UpdateParameter(CarpRobotParameter* parameter) override { parameter->GetValue().tvec() -= parameter->GetGradient().tvec() * m_learning_rate; }
};

#endif