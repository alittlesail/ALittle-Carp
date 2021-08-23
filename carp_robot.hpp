#ifndef CARP_ROBOT_INCLUDED
#define CARP_ROBOT_INCLUDED

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <random>

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

// 随机数
class CarpRandom
{
public:
	static std::mt19937& GetGen()
	{
		static std::random_device rd;
		static std::mt19937 gen(rd());
		return gen;
	}
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

public:
	// 删除某个维度
	void Delete(int index)
	{
		int nd = (int)m_d.size();
		if (index >= nd) return;

		if (index == nd - 1)
		{
			if (nd == 1)
				m_d[0] = 1;
			else
				m_d.resize(nd - 1);
		}
		else
		{
			for (; index + 1 < nd; ++index)
				m_d[index] = m_d[index + 1];
			m_d.resize(nd - 1);
		}

		m_total = m_d.empty() ? 0 : 1; for (size_t i = 0; i < m_d.size(); ++i) m_total *= m_d[i];
	}

public:
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

#include <eigen/Eigen/Eigen>
#include <eigen/unsupported/Eigen/CXX11/Tensor>
#include <eigen_third_party/eigen_spatial_convolutions.h>
#include <eigen_third_party/eigen_backward_spatial_convolutions.h>
#include <eigen_third_party/eigen_pooling.h>

class CarpRobotTensor
{
public:
	// 构造一个张量
	CarpRobotTensor() { }
	CarpRobotTensor(const CarpRobotDim& d, bool init_zero=false) { SetDim(d, init_zero); }
	CarpRobotTensor(const CarpRobotDim& d, cr_real* value) { m_dim = d; m_value = value; m_shared = true; }
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

	// 获取内存
	cr_real* GetValue() const { return m_value; }

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
	void RandomizeUniform(cr_real left=0.0f, cr_real right=1.0f)
	{
		std::uniform_real_distribution<cr_real> distribution(left, right);
		auto b = [&] { return distribution(CarpRandom::GetGen()); };
		std::generate(m_value, m_value + m_dim.GetTotalSize(), b);
	}
	// 随机数
	void RandomizeBernoulli(cr_real p, cr_real scale)
	{
		std::bernoulli_distribution distribution(p);
		auto b = [&] {return distribution(CarpRandom::GetGen()) * scale; };
		std::generate(m_value, m_value + m_dim.GetTotalSize(), b);
	}

	void Logsumexp(CarpRobotTensor& m, CarpRobotTensor& z, int axis=0) const
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= 2, u8"TensorTools::logsumexp 目前只支持向量或者矩阵");

		int other_axis = axis ^ 1;
		if (m_dim[other_axis] == 1)
		{
			m.t<0>() = tvec().maximum();
			cr_real mval = m.AsScalar();
			// This needs to be split into two lines to prevent memory allocation
			z.t<0>() = (tvec() - mval).exp().sum();
			z.t<0>() = z.t<0>().log() + mval;
		}
		else
		{
			Eigen::array<int, 1> red_axis; red_axis[0] = axis;
			m.tb<1>() = tb<2>().maximum(red_axis);

			auto miter = m.GetValue();
			for (size_t i = 0; i < m_dim[1]; ++i, ++miter)
			{
				z.tb<1>().chip<1>(0).chip<0>(i) = (tb<2>().chip<2>(0).chip(i, other_axis) - *miter).exp().sum();
				z.tb<1>().chip<1>(0).chip<0>(i) = z.tb<1>().chip<1>(0).chip<0>(i).log() + *miter;
			}
		}
	}
public:
	// 获取创建二维矩阵对象
	Eigen::Map<Eigen::MatrixXf> m() { return Eigen::Map<Eigen::MatrixXf>(m_value, m_dim.Rows(), m_dim.Cols()); }
	const Eigen::Map<Eigen::MatrixXf> m() const { return Eigen::Map<Eigen::MatrixXf>(m_value, m_dim.Rows(), m_dim.Cols()); }

	Eigen::Map<Eigen::VectorXf> vec() { return Eigen::Map<Eigen::VectorXf>(m_value, m_dim.GetTotalSize()); }
	const Eigen::Map<Eigen::VectorXf> vec() const { return Eigen::Map<Eigen::VectorXf>(m_value, m_dim.GetTotalSize()); }

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
	Eigen::TensorMap<Eigen::Tensor<cr_real, 0>> t<0>()
	{
		CARP_ROBOT_ASSERT(m_dim.Count() == 1 && m_dim.GetTotalSize() == 1, u8"非法维度信息 t<0>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 0>>(m_value);
	}
	template <>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 0>> t<0>() const
	{
		CARP_ROBOT_ASSERT(m_dim.Count() == 1 && m_dim.GetTotalSize() == 1, u8"非法维度信息 t<0>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 0>>(m_value);
	}

	template <>
	Eigen::TensorMap<Eigen::Tensor<cr_real, 1>> t<1>()
	{
		CARP_ROBOT_ASSERT(m_dim.GetTotalSize() == m_dim.Rows(), u8"非法维度信息 t<1>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 1>>(m_value, m_dim[0]);
	}
	template <>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 1>> t<1>() const
	{
		CARP_ROBOT_ASSERT(m_dim.GetTotalSize() == m_dim.Rows(), u8"非法维度信息 t<1>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 1>>(m_value, m_dim[0]);
	}

	template <>
	Eigen::TensorMap<Eigen::Tensor<cr_real, 2>> t<2>()
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= 2, u8"非法维度信息 t<2>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 2>>(m_value, m_dim[0], m_dim[1]);
	}
	template <>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 2>> t<2>() const
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= 2, u8"非法维度信息 t<2>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 2>>(m_value, m_dim[0], m_dim[1]);
	}

	template <>
	Eigen::TensorMap<Eigen::Tensor<cr_real, 3>> t<3>()
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= 3, u8"非法维度信息 t<3>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 3>>(m_value, m_dim[0], m_dim[1], m_dim[2]);
	}
	template <>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 3>> t<3>() const
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= 3, u8"非法维度信息 t<3>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 3>>(m_value, m_dim[0], m_dim[1], m_dim[2]);
	}

	template <>
	Eigen::TensorMap<Eigen::Tensor<cr_real, 4>> t<4>()
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= 4, u8"非法维度信息 t<4>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 4>>(m_value, m_dim[0], m_dim[1], m_dim[2], m_dim[3]);
	}
	template <>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 4>> t<4>() const
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= 4, u8"非法维度信息 t<4>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 4>>(m_value, m_dim[0], m_dim[1], m_dim[2], m_dim[3]);
	}

	// 使用当前数据，创建一个带批次数据的Order维度的Tensor对象
	template <int Order>
	Eigen::TensorMap<Eigen::Tensor<cr_real, Order + 1>> tb()
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= Order, u8"非法维度信息 tb<" << Order << ">(): dim=" << m_dim.ToString());
		std::array<int, Order + 1> dim; for (int i = 0; i < Order; ++i) dim[i] = m_dim[i];
		m_dim[Order] = 1;
		return Eigen::TensorMap<Eigen::Tensor<cr_real, Order + 1>>(m_value, dim);
	}
	template <int Order>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, Order + 1>> tb() const
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= Order, u8"非法维度信息 tb<" << Order << ">(): dim=" << m_dim.ToString());
		std::array<int, Order + 1> dim; for (int i = 0; i < Order; ++i) dim[i] = m_dim[i];
		m_dim[Order] = 1;
		return Eigen::TensorMap<Eigen::Tensor<cr_real, Order>>(m_value, dim);
	}

	template <>
	Eigen::TensorMap<Eigen::Tensor<cr_real, 1>> tb<0>()
	{
		CARP_ROBOT_ASSERT(m_dim.Count() == 1 && m_dim.GetTotalSize() == 1, u8"非法维度信息 tb<0>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 1>>(m_value, 1);
	}
	template <>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 1>> tb<0>() const
	{
		CARP_ROBOT_ASSERT(m_dim.Count() == 1 && m_dim.GetTotalSize() == 1, u8"非法维度信息 tb<0>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 1>>(m_value, 1);
	}

	template <>
	Eigen::TensorMap<Eigen::Tensor<cr_real, 2>> tb<1>()
	{
		CARP_ROBOT_ASSERT(m_dim.GetTotalSize() == m_dim.Rows(), u8"非法维度信息 tb<1>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 2>>(m_value, m_dim[0], 1);
	}
	template <>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 2>> tb<1>() const
	{
		CARP_ROBOT_ASSERT(m_dim.GetTotalSize() == m_dim.Rows(), u8"非法维度信息 tb<1>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 2>>(m_value, m_dim[0], 1);
	}

	template <>
	Eigen::TensorMap<Eigen::Tensor<cr_real, 3>> tb<2>()
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= 2, u8"非法维度信息 tb<2>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 3>>(m_value, m_dim[0], m_dim[1], 1);
	}
	template <>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 3>> tb<2>() const
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= 2, u8"非法维度信息 tb<2>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 3>>(m_value, m_dim[0], m_dim[1], 1);
	}

	template <>
	Eigen::TensorMap<Eigen::Tensor<cr_real, 4>> tb<3>()
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= 3, u8"非法维度信息 tb<3>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 4>>(m_value, m_dim[0], m_dim[1], m_dim[2], 1);
	}
	template <>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 4>> tb<3>() const
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= 3, u8"非法维度信息 tb<3>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 4>>(m_value, m_dim[0], m_dim[1], m_dim[2], 1);
	}

	template <>
	Eigen::TensorMap<Eigen::Tensor<cr_real, 5>> tb<4>()
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= 4, u8"非法维度信息 tb<4>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 5>>(m_value, m_dim[0], m_dim[1], m_dim[2], m_dim[3], 1);
	}
	template <>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 5>> tb<4>() const
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= 4, u8"非法维度信息 tb<4>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 5>>(m_value, m_dim[0], m_dim[1], m_dim[2], m_dim[3], 1);
	}

public:
	// 当作标量返回
	cr_real AsScalar() const { CARP_ROBOT_ASSERT(m_dim.GetTotalSize() == 1, u8"当前Tensor不是标量"); return m_value[0]; }
	// 当做向量返回 
	std::vector<cr_real> AsVector() const
	{
		std::vector<cr_real> result;
		result.resize(m_dim.GetTotalSize());
		memcpy(result.data(), m_value, sizeof(cr_real) * result.size());
		return result;
	}
	// 当做向量，并且返回向量中数值最大的那个下标
	int AsVectorAndArgmax() const
	{
		int vector_size = m_dim.GetTotalSize();
		int index = 0;
		cr_real max_value = m_value[index];
		for (int i = 0; i < vector_size; ++i)
		{
			if (max_value < m_value[i])
			{
				index = i;
				max_value = m_value[i];
			}
		}

		return index;
	}

	// 当做向量，并且返回向量中数值最大的那个值
	cr_real AsVectorAndMaxValue() const { return m_value[AsVectorAndArgmax()]; }

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
	CarpRobotParameterCollection() { }
	virtual ~CarpRobotParameterCollection()
	{
		for (size_t i = 0; i < m_params.size(); ++i)
			delete m_params[i];
	}

public:
	virtual CarpRobotParameter* AddParameters(const CarpRobotDim& d, const std::string& name = "")
	{
		auto* p = new CarpRobotParameter(d, name);
		p->GetValue().RandomizeUniform();
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

	// 序列化
	void Serialize(CarpRobotModelSerializer& file)
	{
		for (size_t i = 0; i < m_params.size(); ++i)
			m_params[i]->Serialize(file, nullptr);
	}
	// 反序列化
	void Deserialize(CarpRobotModelDeserializer& file)
	{
		for (size_t i = 0; i < m_params.size(); ++i)
			m_params[i]->Deserialize(file, nullptr);
	}

private:
	std::vector<CarpRobotParameter*> m_params;					// Parameter参数的集合
	
private:
	std::string m_name;									// 收集器名字
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
	CarpRobotParameterNode(CarpRobotParameter* parameter) : m_parameter(parameter) {}

public:
	// 执行反向传递梯度
	void AccumulateGrad(const CarpRobotTensor& grad) override { m_parameter->AccumulateGrad(grad); }

protected:
	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		fx.SetDim(m_parameter->GetValue().GetDim());
		fx.tvec() = m_parameter->GetValue().tvec();
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
	CarpRobotParameter* m_parameter = nullptr;
};

// 常量参数输入节点
class CarpRobotConstParameterNode : public CarpRobotNode
{
public:
	CarpRobotConstParameterNode(CarpRobotParameter* parameter) : m_parameter(parameter) { }

protected:
	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		fx.SetDim(m_parameter->GetValue().GetDim());
		fx.tvec() = m_parameter->GetValue().tvec();
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
	CarpRobotParameter* m_parameter = nullptr;
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
		fx.tb<4>() = xs[0]->tb<4>() / xs[1]->tb<4>();
	}
	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		unsigned int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		if (xs_i == 0)
			dEdxi.tb<4>() += dEdf.tb<4>() / xs[1]->tb<4>();
		else
			dEdxi.tb<4>() -= (dEdf.tb<4>() / xs[1]->tb<4>().square() * xs[0]->tb<4>());
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

// y = max(0,x)
class CarpRobotRectifyNode : public CarpRobotNode
{
public:
	CarpRobotRectifyNode(const std::vector<int>& a) : CarpRobotNode(a) {}
	~CarpRobotRectifyNode() {}

protected:
	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx)
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"CarpRobotRectifyNode 必须是一个输入");
		fx.SetDim(xs[0]->GetDim());
		fx.tvec() = xs[0]->tvec().cwiseMax(0.f);
	}
	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		unsigned int xs_i,
		CarpRobotTensor& dEdxi)
	{
		dEdxi.tvec() += fx.tvec().cast<bool>().cast<float>() * dEdf.tvec();
	}
};

// z = \sum_j \exp (x_i)_j
// y_i = (x_1)_i / z
class CarpRobotSoftmaxNode : public CarpRobotNode
{
public:
	CarpRobotSoftmaxNode(const std::vector<int>& a) : CarpRobotNode(a) {}
	~CarpRobotSoftmaxNode() {}

protected:
	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx)
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"CarpRobotRectifyNode 必须是一个输入");
		fx.SetDim(xs[0]->GetDim());

		CarpRobotTensor z(CarpRobotDim({ 1 }));
		CarpRobotTensor m(CarpRobotDim({ 1 }));

		int size = xs[0]->GetDim()[0];
		int num_cols = xs[0]->GetDim()[1];

		cr_real* col_x_value = xs[0]->GetValue();
		cr_real* col_fx_value = fx.GetValue();
		CarpRobotDim col_dim = CarpRobotDim({ xs[0]->GetDim()[0] });
		for (size_t col = 0; col < num_cols; ++col)
		{
			CarpRobotTensor col_x(col_dim, col_x_value);
			CarpRobotTensor col_fx(col_dim, col_fx_value);

			// 取得col_x向量中最大的那个值
			m.t<0>() = col_x.tvec().maximum();
			// 将col_x向量的每个值都减去最大值，然后再求e的col_x每个值次方
			// 这里之所有要减去最大值，是防止运算过程中溢出。并且同时减去一个最大值也不会影响结果
			col_fx.tvec() = (col_x.tvec() - m.GetValue()[0]).exp();
			// 计算e的col_x每个值次方的总和
			z.t<0>() = col_fx.tvec().sum();
			// 将e的col_x每个值次方都除以总和，从而得到softmax
			col_fx.tvec() = col_fx.tvec() / z.GetValue()[0];

			col_x_value += size;
			col_fx_value += size;
		}
	}
	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		unsigned int xs_i,
		CarpRobotTensor& dEdxi)
	{
		CarpRobotTensor z(CarpRobotDim({ fx.GetDim().Cols() }));
		Eigen::array<ptrdiff_t, 1> red_axis = { 0 };
		z.tb<1>() = (fx.tb<2>() * dEdf.tb<2>()).sum(red_axis);

		int size = xs[0]->GetDim()[0];
		int num_cols = xs[0]->GetDim()[1];

		cr_real* col_fx_value = fx.GetValue();
		cr_real* col_dEdf_value = dEdf.GetValue();
		cr_real* col_dEdxi_value = dEdxi.GetValue();
		CarpRobotDim col_dim = CarpRobotDim({ xs[0]->GetDim()[0] });

		for (size_t col = 0; col < num_cols; ++col)
		{
			CarpRobotTensor col_fx(col_dim, col_fx_value);
			CarpRobotTensor col_dEdf(col_dim, col_dEdf_value);
			CarpRobotTensor col_dEdxi(col_dim, col_dEdxi_value);
			col_dEdxi.tvec() += (col_dEdf.tvec() - z.GetValue()[col]) * col_fx.tvec();
			col_fx_value += size;
			col_dEdf_value += size;
			col_dEdxi_value += size;
		}
	}
};

// z = \sum_j \exp (x_i)_j
// y_i = (x_1)_i - \log z
class CarpRobotLogSoftmaxNode : public CarpRobotNode
{
public:
	CarpRobotLogSoftmaxNode(const std::vector<int>& a) : CarpRobotNode(a) {}
	~CarpRobotLogSoftmaxNode() {}

protected:
	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx)
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"CarpRobotRectifyNode 必须是一个输入");
		fx.SetDim(xs[0]->GetDim());

		CarpRobotTensor z(CarpRobotDim({ xs[0]->GetDim().Cols() }));
		CarpRobotTensor m(CarpRobotDim({ xs[0]->GetDim().Cols() }));
		xs[0]->Logsumexp(m, z);
		if (fx.GetDim().GetTotalSize() == fx.GetDim().Rows())
		{
			fx.tvec() = xs[0]->tvec() - z.AsScalar();
		}
		else
		{
			int size = xs[0]->GetDim()[0];
			int num_cols = xs[0]->GetDim()[1];

			auto col_fx_value = fx.GetValue();
			auto col_x_value = xs[0]->GetValue();
			auto col_dim = CarpRobotDim({ xs[0]->GetDim()[0] });
			for (size_t col = 0; col < num_cols; ++col)
			{
				CarpRobotTensor col_fx(col_dim, col_fx_value);
				CarpRobotTensor col_x(col_dim, col_x_value);
				col_fx.tvec() = col_x.tvec() - z.GetValue()[col];
				col_x_value += size;
				col_fx_value += size;
			}
		}
	}
	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		unsigned int xs_i,
		CarpRobotTensor& dEdxi)
	{
		CarpRobotTensor z(CarpRobotDim({ xs[0]->GetDim().Cols() }));
		Eigen::array<ptrdiff_t, 1> red_axis = { 0 };
		z.tb<1>() = dEdf.tb<2>().sum(red_axis);

		int size = xs[0]->GetDim()[0];
		int num_cols = xs[0]->GetDim()[1];

		cr_real* col_fx_value = fx.GetValue();
		cr_real* col_dEdf_value = dEdf.GetValue();
		cr_real* col_dEdxi_value = dEdxi.GetValue();
		CarpRobotDim col_dim = CarpRobotDim({ xs[0]->GetDim()[0] });

		for (size_t col = 0; col < num_cols; ++col)
		{
			CarpRobotTensor col_fx(col_dim, col_fx_value);
			CarpRobotTensor col_dEdf(col_dim, col_dEdf_value);
			CarpRobotTensor col_dEdxi(col_dim, col_dEdxi_value);

			col_dEdxi.tvec() += (col_fx.tvec().exp() * -z.GetValue()[col]) + col_dEdf.tvec();
			col_fx_value += size;
			col_dEdf_value += size;
			col_dEdxi_value += size;
		}
	}
};

// y = dropout(x,p) where p specifies the dropout probability
class CarpRobotDropoutNode : public CarpRobotNode
{
public:
	CarpRobotDropoutNode(const std::vector<int>& a, cr_real v) : CarpRobotNode(a), m_value(v) {}
	~CarpRobotDropoutNode() {}

protected:
	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx)
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"CarpRobotDropoutNode 必须是一个输入");
		fx.SetDim(xs[0]->GetDim());
		m_aux_mem.SetDim(fx.GetDim());
		m_aux_mem.RandomizeBernoulli((1.f - m_value), 1.f / (1.f - m_value));
		fx.tvec() = xs[0]->tvec() * m_aux_mem.tvec();
	}
	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		unsigned int xs_i,
		CarpRobotTensor& dEdxi)
	{
		dEdxi.tvec() += dEdf.tvec() * m_aux_mem.tvec();
	}

private:
	cr_real m_value;
	CarpRobotTensor m_aux_mem;
};

// conv2d 
// y = x_1 *conv2d x_2
// x_1 \in R^{H x W x Ci x N} (input)
// x_2 \in R^{H x W x Ci x Co} (filter)
// stride[0] corresponds to H
// stride[1] corresponds to W
// padding_type: true for 'VALID' and false for 'SAME'
class CarpRobotConv2DNode : public CarpRobotNode
{
public:
	CarpRobotConv2DNode(const std::vector<int>& a, const std::vector<int>& stride= { 1, 1 }, bool padding_type = true) : CarpRobotNode(a), m_stride(stride), m_padding_type(padding_type) {}
	~CarpRobotConv2DNode() {}

protected:
	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx)
	{
		{
			CARP_ROBOT_ASSERT(xs.size() == 2 || xs.size() == 3, u8"Conv2D requires either two or three inputs");
			CARP_ROBOT_ASSERT((xs[0]->GetDim().Count() == 2 || xs[0]->GetDim().Count() == 3) && xs[1]->GetDim().Count() == 4 && xs[1]->GetDim()[2] == xs[0]->GetDim()[2], u8"Conv2D requires either two or three inputs");
			CARP_ROBOT_ASSERT(!m_padding_type || (xs[0]->GetDim()[0] >= xs[1]->GetDim()[0] && xs[0]->GetDim()[1] >= xs[1]->GetDim()[1]), u8"Bad input dimensions in Conv2D: in VALID convolution, the filter size must not be greater than the feature map size");
			if (xs.size() == 3) //has bias term
				CARP_ROBOT_ASSERT(xs[2]->GetDim()[0] == xs[1]->GetDim()[3] && xs[2]->GetDim().Count() == 1, u8"Bad input dimensions in Conv2D");

			std::vector<int> output_shape(3);
			output_shape[2] = xs[1]->GetDim()[3];
			for (unsigned i = 0; i < 2; ++i)
			{
				float input_dim = static_cast<float>(xs[0]->GetDim()[i]);
				float kernel_dim = static_cast<float>(xs[1]->GetDim()[i]);
				float s = static_cast<float>(m_stride[i]);
				if (m_padding_type)
					output_shape[i] = static_cast<int>(std::ceil((input_dim - kernel_dim + 1) / s));
				else
					output_shape[i] = static_cast<int>(std::ceil(input_dim / s));
			}
			fx.SetDim(CarpRobotDim(output_shape));
		}

		Eigen::PaddingType padding_type = m_padding_type ? Eigen::PADDING_VALID : Eigen::PADDING_SAME;
		
		CarpRobotTensor CHWN_x(CarpRobotDim({ xs[0]->GetDim()[2], xs[0]->GetDim()[0], xs[0]->GetDim()[1] }));
		Eigen::array<ptrdiff_t, 4> shuffles;
		shuffles[0] = 2; shuffles[1] = 0; shuffles[2] = 1; shuffles[3] = 3;
		CHWN_x.tb<3>() = xs[0]->tb<3>().shuffle(shuffles);
		
		CarpRobotTensor NCHW_f(CarpRobotDim({ xs[1]->GetDim()[3], xs[1]->GetDim()[2], xs[1]->GetDim()[0], xs[1]->GetDim()[1] }));
		shuffles[0] = 3; shuffles[1] = 2; shuffles[2] = 0; shuffles[3] = 1;
		NCHW_f.t<4>() = xs[1]->t<4>().shuffle(shuffles);

		CarpRobotTensor CHWN_y(CarpRobotDim({ fx.GetDim()[2], fx.GetDim()[0], fx.GetDim()[1] }));
		CHWN_y.tb<3>() = Eigen::SpatialConvolution(CHWN_x.tb<3>(), NCHW_f.t<4>(), m_stride[0], m_stride[1], padding_type);
		shuffles[0] = 1; shuffles[1] = 2; shuffles[2] = 0; shuffles[3] = 3;
		fx.tb<3>() = CHWN_y.tb<3>().shuffle(shuffles);
		if (xs.size() == 3)
		{
			CarpRobotTensor bias(CarpRobotDim({ fx.GetDim()[0], fx.GetDim()[1], 1 }), CHWN_x.GetValue());
			for (int i = 0; i < fx.GetDim()[2]; ++i)
			{
				bias.Constant(xs[2]->vec()(i));
				fx.tb<3>().chip<2>(i) += bias.t<3>();
			}
		}
	}
	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		unsigned int xs_i,
		CarpRobotTensor& dEdxi)
	{
		CarpRobotTensor CHWN_dy(CarpRobotDim({ dEdf.GetDim()[2], dEdf.GetDim()[0], dEdf.GetDim()[1] }));
		Eigen::array<ptrdiff_t, 4> shuffles;
		shuffles[0] = 2; shuffles[1] = 0; shuffles[2] = 1; shuffles[3] = 3;
		CHWN_dy.tb<3>() = dEdf.tb<3>().shuffle(shuffles);
		
		if (xs_i == 0) // backward w.r.t the input
		{
			CarpRobotTensor NCHW_f(CarpRobotDim({ xs[1]->GetDim()[3], xs[1]->GetDim()[2], xs[1]->GetDim()[0], xs[1]->GetDim()[1] }));
			shuffles[0] = 3; shuffles[1] = 2; shuffles[2] = 0; shuffles[3] = 1;
			NCHW_f.t<4>() = xs[1]->t<4>().shuffle(shuffles);
			
			CarpRobotTensor CHWN_dEdxi(CarpRobotDim({ xs[0]->GetDim()[2], xs[0]->GetDim()[0], xs[0]->GetDim()[1] }));
			CHWN_dEdxi.tb<3>() = Eigen::SpatialConvolutionBackwardInput(NCHW_f.t<4>(), CHWN_dy.tb<3>(), xs[0]->GetDim()[0], xs[0]->GetDim()[1], m_stride[0], m_stride[1]);
			
			CarpRobotTensor HWCN_dEdxi(xs[0]->GetDim());
			shuffles[0] = 1; shuffles[1] = 2; shuffles[2] = 0; shuffles[3] = 3;
			HWCN_dEdxi.tb<3>() = CHWN_dEdxi.tb<3>().shuffle(shuffles);
			dEdxi.tb<3>() += HWCN_dEdxi.tb<3>();
		}
		else if (xs_i == 1) //backward w.r.t the kernel
		{
			CarpRobotTensor CHWN_x(CarpRobotDim({ xs[0]->GetDim()[2], xs[0]->GetDim()[0], xs[0]->GetDim()[1] }));
			shuffles[0] = 2; shuffles[1] = 0; shuffles[2] = 1; shuffles[3] = 3;
			CHWN_x.tb<3>() = xs[0]->tb<3>().shuffle(shuffles);
			
			CarpRobotTensor NCHW_dEdxi(CarpRobotDim({ xs[1]->GetDim()[3], xs[1]->GetDim()[2], xs[1]->GetDim()[0], xs[1]->GetDim()[1] }));
			NCHW_dEdxi.t<4>() = Eigen::SpatialConvolutionBackwardKernel(CHWN_x.tb<3>(), CHWN_dy.tb<3>(), xs[1]->GetDim()[0], xs[1]->GetDim()[1], m_stride[0], m_stride[1], m_padding_type);
			
			CarpRobotTensor HWCN_dEdxi(xs[1]->GetDim());
			shuffles[0] = 2; shuffles[1] = 3; shuffles[2] = 1; shuffles[3] = 0;
			HWCN_dEdxi.t<4>() = NCHW_dEdxi.t<4>().shuffle(shuffles);
			dEdxi.t<4>() += HWCN_dEdxi.t<4>();
		}
		else //backward w.r.t the bias
		{ 
			Eigen::array<ptrdiff_t, 3> red_axis = { 0, 1, 3 };
			dEdxi.t<1>() += dEdf.tb<3>().sum(red_axis);
		}
	}

private:
	std::vector<int> m_stride;
	bool m_padding_type = true;
};

// maxpooling2d
// y = x_1 * maxpooling2d
// x_1 \in R^{H x W x Ci x N} (input)
// ksize[0] corresponds to H
// ksize[1] corresponds to W
// stride[0] corresponds to H
// stride[1] corresponds to W
// padding_type: true for 'VALID' and false for 'SAME'
class CarpRobotMaxPooling2DNode : public CarpRobotNode
{
public:
	CarpRobotMaxPooling2DNode(const std::vector<int>& a, const std::vector<int>& ksize, const std::vector<int>& stride= { 1, 1 }, bool padding_type = true) : CarpRobotNode(a), m_ksize(ksize), m_stride(stride), m_padding_type(padding_type) {}
	~CarpRobotMaxPooling2DNode() {}

protected:
	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx)
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"CarpRobotMaxPooling2DNode 必须是一个输入");
		{
			CARP_ROBOT_ASSERT(xs[0]->GetDim().Count() == 2 || xs[0]->GetDim().Count() == 3, u8"Bad input dimensions in MaxPooling2D, expected 2 or 3 dimensions");
			CARP_ROBOT_ASSERT(!m_padding_type || (xs[0]->GetDim()[0] >= m_ksize[0] && xs[0]->GetDim()[1] >= m_ksize[1]), u8"Bad input dimensions in MaxPooling2D: in VALID mode, the kernel size cannot be greater than the feature map size");

			std::vector<int> output_shape(xs[0]->GetDim().Count());
			if (xs[0]->GetDim().Count() == 3)
				output_shape[2] = xs[0]->GetDim()[2];
			
			for (unsigned i = 0; i < 2; ++i)
			{
				float input_dim = static_cast<float>(xs[0]->GetDim()[i]);
				float kernel_dim = static_cast<float>(m_ksize[i]);
				float s = static_cast<float>(m_stride[i]);
				if (m_padding_type)
					output_shape[i] = static_cast<int>(std::ceil((input_dim - kernel_dim + 1) / s));
				else
					output_shape[i] = static_cast<int>(std::ceil(input_dim / s));
			}
			fx.SetDim(CarpRobotDim(output_shape));
		}

		Eigen::PaddingType padding_type = m_padding_type ? Eigen::PADDING_VALID : Eigen::PADDING_SAME;

		// convert x from HWCN to CHWN
		CarpRobotTensor CHWN_x(CarpRobotDim({ xs[0]->GetDim()[2], xs[0]->GetDim()[0], xs[0]->GetDim()[1] }));
		Eigen::array<ptrdiff_t, 4> shuffles;
		shuffles[0] = 2; shuffles[1] = 0; shuffles[2] = 1; shuffles[3] = 3;
		CHWN_x.tb<3>() = xs[0]->tb<3>().shuffle(shuffles);

		// allocate temp memory and compute
		CarpRobotTensor CHWN_y(CarpRobotDim({ fx.GetDim()[2], fx.GetDim()[0], fx.GetDim()[1] }));
		CHWN_y.tb<3>() = Eigen::SpatialMaxPooling(CHWN_x.tb<3>(), m_ksize[0], m_ksize[1], m_stride[0], m_stride[1], padding_type);
		// convert y from CHWN to HWCN
		shuffles[0] = 1; shuffles[1] = 2; shuffles[2] = 0; shuffles[3] = 3;
		fx.tb<3>() = CHWN_y.tb<3>().shuffle(shuffles);
	}
	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		unsigned int xs_i,
		CarpRobotTensor& dEdxi)
	{
		int pad_along_height = (fx.GetDim()[0] - 1) * m_stride[0] + m_ksize[0] - xs[0]->GetDim()[0];
		int pad_along_width = ((fx.GetDim()[1] - 1) * m_stride[1] + m_ksize[1] - xs[0]->GetDim()[1]);
		int pad_top = m_padding_type ? 0 : pad_along_height / 2;
		int pad_left = m_padding_type ? 0 : pad_along_width / 2;

		for (int i = 0; i < fx.GetDim()[0]; ++i)
		{
			for (int j = 0; j < fx.GetDim()[1]; ++j)
			{
				for (int ch = 0; ch < fx.GetDim()[2]; ++ch)
				{
					int max_r = 0, max_c = 0;
					float max_val;
					bool is_feasible = false;
					for (int r = 0; r < m_ksize[0]; ++r)
					{
						for (int c = 0; c < m_ksize[1]; ++c)
						{
							int row = m_stride[0] * i + r - pad_top;
							int col = m_stride[1] * j + c - pad_left;
							if (((col < xs[0]->GetDim()[1]) && (row < xs[0]->GetDim()[0])))
							{
								if (!is_feasible)
								{
									max_val = xs[0]->tb<3>()(row, col, ch, 0);
									max_r = row; max_c = col; is_feasible = true;
								}
								else if (xs[0]->tb<3>()(row, col, ch, 0) > max_val)
								{
									max_val = xs[0]->tb<3>()(row, col, ch, 0);
									max_r = row; max_c = col;
								}
							}
						}
					}
					(dEdxi.tb<3>())(max_r, max_c, ch, 0) += (dEdf.tb<3>())(i, j, ch, 0);
				}
			}
		}
	}

private:
	std::vector<int> m_ksize;
	std::vector<int> m_stride;
	bool m_padding_type = true;;
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

// z = \sum_j \exp (x_i)_j
// y = (x_1)_element - \log z
class CarpRobotPickNegLogSoftmaxNode : public CarpRobotNode
{
public:
	CarpRobotPickNegLogSoftmaxNode(const std::vector<int>& a, int v) : CarpRobotNode(a), m_val(v) {}
	~CarpRobotPickNegLogSoftmaxNode() {}

protected:
	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"CarpRobotPickNegLogSoftmaxNode 必须是两个输入");
		CARP_ROBOT_ASSERT(xs[0]->GetDim().GetTotalSize() == xs[0]->GetDim().Rows(), u8"输入的维度信息错误");
		
		m_z.SetDim(CarpRobotDim({ 1 }), true);
		m_m.SetDim(CarpRobotDim({ 1 }), true);

		fx.SetDim(CarpRobotDim({ 1 }));
		if (xs[0]->GetDim().Cols() == 1)
		{
			xs[0]->Logsumexp(m_m, m_z);
			fx.GetValue()[0] = xs[0]->GetValue()[m_val];
			fx.tvec() = m_z.tvec() - fx.tvec();
		}
		else
		{
			CARP_ROBOT_ASSERT(0, "PickNegLogSoftmax::forward not yet implemented for multiple columns");
		}
	}

	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		unsigned int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		if (xs[0]->GetDim().Cols() == 1)
		{
			dEdxi.tb<1>().chip<1>(0) += (xs[0]->tb<1>().chip<1>(0) - m_z.GetValue()[0]).exp() * dEdf.GetValue()[0];
			dEdxi.GetValue()[m_val] -= dEdf.GetValue()[0];
		}
		else
		{
			CARP_ROBOT_ASSERT(0, "PickNegLogSoftmax::backward not yet implemented for multiple columns");
		}
	}
public:
	int m_val = 0;
	
private:
	CarpRobotTensor m_z;
	CarpRobotTensor m_m;
};

// x_1 is a vector
// y = (x_1)_{*pval}
class CarpRobotPickElementNode : public CarpRobotNode
{
public:
	CarpRobotPickElementNode(const std::vector<int>& a, int v, int d = 0) : CarpRobotNode(a), m_val(v), m_dim(d) {}
	~CarpRobotPickElementNode() {}

protected:
	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"CarpRobotPickElementNode 必须是两个输入");
		CARP_ROBOT_ASSERT(m_val < xs[0]->GetDim()[m_dim], u8"输入的维度信息错误");

		CarpRobotDim dim = xs[0]->GetDim();
		dim.Delete(m_dim);
		fx.SetDim(dim);

		fx.tb<3>() = xs[0]->tb<4>().chip(m_val, m_dim);
	}

	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		unsigned int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		dEdxi.tb<3>().chip(m_val, m_dim) += dEdf.tb<2>();
	}

public:
	int m_val = 0;
	int m_dim = 0;
};

// y = reshape(x_1, --> to)
class CarpRobotReshapeNode : public CarpRobotNode
{
public:
	CarpRobotReshapeNode(const std::vector<int>& a, const CarpRobotDim& dim) : CarpRobotNode(a), m_dim(dim) {}
	~CarpRobotReshapeNode() {}

protected:
	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"CarpRobotPickNegLogSoftmaxNode 必须是两个输入");
		CARP_ROBOT_ASSERT(xs[0]->GetDim().GetTotalSize() == m_dim.GetTotalSize(), u8"CarpRobotReshapeNode 输入的总大小必须和输出一致");

		// just point to the input memory and change dimensions
		// dimensions are handled by forward_dim
		fx.SetDim(m_dim);
		fx.tvec() = xs[0]->tvec();
	}

	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		unsigned int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		CarpRobotTensor reshaped(dEdxi.GetDim(), dEdf.GetValue());
		dEdxi.tvec() += reshaped.tvec();
	}

private:
	CarpRobotDim m_dim;
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
	// 损失函数
	CarpRobotExpression Square() { std::vector<int> args; args.push_back(m_index); return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotSquareNode(args))); }
	CarpRobotExpression PickNegLogSoftmax(int v) { std::vector<int> args; args.push_back(m_index); return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotPickNegLogSoftmaxNode(args, v))); }
	CarpRobotExpression BinaryLogLoss() { std::vector<int> args; args.push_back(m_index); return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotBinaryLogLossNode(args))); }

	// 激活函数
	CarpRobotExpression Sigmoid() { std::vector<int> args; args.push_back(m_index); return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotLogisticSigmoidNode(args))); }
	CarpRobotExpression Rectify() { std::vector<int> args; args.push_back(m_index); return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotRectifyNode(args))); }
	CarpRobotExpression Softmax() { std::vector<int> args; args.push_back(m_index); return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotSoftmaxNode(args))); }
	CarpRobotExpression LogSoftmax() { std::vector<int> args; args.push_back(m_index); return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotLogSoftmaxNode(args))); }

	// 功能函数
	CarpRobotExpression Dropout(cr_real rate) { std::vector<int> args; args.push_back(m_index); return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotDropoutNode(args, rate))); }
	CarpRobotExpression Conv2D(const CarpRobotExpression& kernel, const std::vector<int>& stride= { 1, 1 }, bool padding_type = true) { std::vector<int> args; args.push_back(m_index); args.push_back(kernel.GetIndex()); return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotConv2DNode(args, stride, padding_type))); }
	CarpRobotExpression MaxPooling2D(const std::vector<int>& ksize, const std::vector<int>& stride= { 1, 1 }, bool padding_type = true) { std::vector<int> args; args.push_back(m_index); return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotMaxPooling2DNode(args, ksize, stride, padding_type))); }
	CarpRobotExpression Reshape(const CarpRobotDim& dim) { std::vector<int> args; args.push_back(m_index); return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotReshapeNode(args, dim))); }
	CarpRobotExpression PickElement(int value, int dim = 0) { std::vector<int> args; args.push_back(m_index); return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotPickElementNode(args, value, dim))); }

private:
	ICarpRobotComputationGraph* m_graph = nullptr;
	int m_index = 0;
};

// 表达式的四则运算
CarpRobotExpression operator-(const CarpRobotExpression& x) { return CarpRobotExpression(x.GetGraph(), x.GetGraph()->AddNode(new CarpRobotNegateNode({ x.GetIndex() }))); }
CarpRobotExpression operator+(const CarpRobotExpression& x, const CarpRobotExpression& y) { return CarpRobotExpression(x.GetGraph(), x.GetGraph()->AddNode(new CarpRobotCwiseSumNode({ x.GetIndex(), y.GetIndex() }))); }
CarpRobotExpression operator+(cr_real x, const CarpRobotExpression& y) { return CarpRobotExpression(y.GetGraph(), y.GetGraph()->AddNode(new CarpRobotConstantPlusXNode({ y.GetIndex() }, x))); }
CarpRobotExpression operator+(const CarpRobotExpression& x, cr_real y) { return y + x; }
CarpRobotExpression operator-(const CarpRobotExpression& x, const CarpRobotExpression& y) { return x + (-y); }
CarpRobotExpression operator-(cr_real x, const CarpRobotExpression& y) { return CarpRobotExpression(y.GetGraph(), y.GetGraph()->AddNode(new CarpRobotConstantMinusXNode({ y.GetIndex() }, x))); }
CarpRobotExpression operator-(const CarpRobotExpression& x, cr_real y) { return -(y - x); }
CarpRobotExpression operator*(const CarpRobotExpression& x, const CarpRobotExpression& y) { return CarpRobotExpression(x.GetGraph(), x.GetGraph()->AddNode(new CarpRobotMatrixMultiplyNode({ x.GetIndex(), y.GetIndex() }))); }
CarpRobotExpression operator*(const CarpRobotExpression& x, cr_real y) { return CarpRobotExpression(x.GetGraph(), x.GetGraph()->AddNode(new CarpRobotConstScalarMultiplyNode({ x.GetIndex() }, y))); }
CarpRobotExpression operator/(const CarpRobotExpression& x, const CarpRobotExpression& y) { return CarpRobotExpression(x.GetGraph(), x.GetGraph()->AddNode(new CarpRobotCwiseQuotientNode({ x.GetIndex(), y.GetIndex() }))); }

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
			UpdateParameter((int)i, parameters[i]);
			// 将梯度值清零
			parameters[i]->Clear();
		}

		++m_updates;
	}

	// 重置训练器的缓存数据
	virtual void Restart()
	{
		m_updates = 0;
	}

protected:
	cr_real m_learning_rate = 0.1f;	// 学习率
	int m_updates = 0;
	CarpRobotParameterCollection* m_model = nullptr;		// ParameterCollection对象

protected:
	// 基类不允许构造
	CarpRobotTrainer() {}

	// 更新参数
	virtual void UpdateParameter(int index, CarpRobotParameter* parameter) = 0;
};

class CarpRobotSGDTrainer : public CarpRobotTrainer
{
public:
	CarpRobotSGDTrainer(CarpRobotParameterCollection& model, cr_real learning_rate) : CarpRobotTrainer(model, learning_rate) {}
protected:
	void UpdateParameter(int index, CarpRobotParameter* parameter) override { parameter->GetValue().tvec() -= parameter->GetGradient().tvec() * m_learning_rate; }
};


class CarpRobotAdamTrainer : public CarpRobotTrainer
{
public:
	CarpRobotAdamTrainer(CarpRobotParameterCollection& model, float learning_rate = 0.001, float beta_1 = 0.9, float beta_2 = 0.999, float eps = 1e-8) :
		CarpRobotTrainer(model, learning_rate), m_beta_1(beta_1), m_beta_2(beta_2), m_epsilon(eps) {}
	~CarpRobotAdamTrainer() {}

	void Restart()
	{
		for (size_t i = 0; i < m_m.size(); ++i)
			m_m[i].Zero();

		for (size_t i = 0; i < m_v.size(); ++i)
			m_v[i].Zero();

		m_updates = 0;
	}

protected:
	void AllocateShadowParameters(CarpRobotParameterCollection& model, std::vector<CarpRobotTensor>& list)
	{
		const auto& parameters = model.GetParameters();
		if (list.size() == parameters.size()) return;

		list.resize(parameters.size());
		for (size_t i = 0; i < parameters.size(); ++i)
			list[i].SetDim(parameters[i]->GetValue().GetDim(), true);
	}

	void UpdateParameter(int index, CarpRobotParameter* parameter) override
	{
		AllocateShadowParameters(*m_model, m_m);
		AllocateShadowParameters(*m_model, m_v);

		auto& m = m_m[index];
		auto& v = m_v[index];

		m.tvec() = m.tvec() * m_beta_1 + parameter->GetGradient().tvec() * (1.f - m_beta_1);
		v.tvec() = v.tvec() * m_beta_2 + parameter->GetGradient().tvec().square() * (1.f - m_beta_2);
		auto lr_t = m_learning_rate * sqrt(1 - pow(m_beta_2, m_updates + 1)) / (1 - pow(m_beta_1, m_updates + 1));
		parameter->GetValue().tvec() -= m.tvec() / (v.tvec().sqrt() + m_epsilon) * static_cast<float>(lr_t);
	}

private:
	float m_beta_1 = 0;
	float m_beta_2 = 0;
	float m_epsilon = 0;

	std::vector<CarpRobotTensor> m_m; // History of gradients
	std::vector<CarpRobotTensor> m_v; // History of deltas
};

#endif