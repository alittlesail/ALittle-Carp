#ifndef CARP_ROBOT_INCLUDED
#define CARP_ROBOT_INCLUDED

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <random>

// �����쳣��
#define CARP_ROBOT_ASSERT(e, text) \
do { \
	if (!(e)) { std::ostringstream oss; oss << __FUNCTION__ << "() " << text; throw std::runtime_error(oss.str()); } \
} while (0)

// ���帡��������
typedef float cr_real;

// ���л�
class CarpRobotModelSerializer
{
public:
	~CarpRobotModelSerializer() { Close(); }

public:
	// �򿪺͹ر��ļ�
	bool Open(const std::string& model_path) { m_out_file.open(model_path, std::ios::binary | std::ios::trunc); return m_out_file.is_open(); }
	void Close() { if (m_out_file.is_open()) m_out_file.close(); }

public:
	// д���ַ���
	void WriteString(const std::string& str)
	{
		CARP_ROBOT_ASSERT(m_out_file.is_open(), u8"WriteString Failed: out_file open failed");
		int length = (int)str.size();
		m_out_file.write((char*)&length, sizeof(length));
		m_out_file.write(str.c_str(), length);
	}
	// д��һ��������
	void WriteReal(cr_real value)
	{
		CARP_ROBOT_ASSERT(m_out_file.is_open(), u8"WriteReal Failed: out_file open failed");
		m_out_file.write((char*)&value, sizeof(cr_real));
	}
	// д��һ������������
	void WriteRealArray(cr_real* value, size_t count)
	{
		CARP_ROBOT_ASSERT(m_out_file.is_open(), u8"WriteRealArray Failed: out_file open failed");
		int length = (int)count;
		m_out_file.write((char*)&length, sizeof(length));
		m_out_file.write((char*)value, sizeof(cr_real) * count);
	}
	// д��һ��������ֵ
	void WriteInt(int value)
	{
		CARP_ROBOT_ASSERT(m_out_file.is_open(), u8"WriteInt Failed: out_file open failed");
		m_out_file.write((char*)&value, sizeof(int));
	}

private:
	std::ofstream m_out_file;
};

// �����л�
class CarpRobotModelDeserializer
{
public:
	~CarpRobotModelDeserializer() { Close(); }

public:
	// �򿪺͹ر��ļ�
	bool Open(const std::string& model_path) { m_in_file.open(model_path, std::ios::binary); return m_in_file.is_open(); }
	void Close() { if (m_in_file.is_open()) m_in_file.close(); }

public:
	// ��ȡ�ַ���
	const char* ReadString()
	{
		CARP_ROBOT_ASSERT(m_in_file.is_open(), "ReadString Failed: in_file open failed");

		// �ȶ�ȡ��С
		int length = 0;
		m_in_file.read((char*)&length, sizeof(int));
		CARP_ROBOT_ASSERT(m_in_file.gcount() == sizeof(int), u8"ReadString Failed: length error 1");

		// ���ô�С
		m_string.resize(length + 1);
		m_string[length] = 0;
		m_in_file.read(m_string.data(), length);
		CARP_ROBOT_ASSERT(m_in_file.gcount() == length, u8"ReadString Failed: length error 2");

		return m_string.data();
	}
	// ��ȡһ��������
	cr_real ReadReal()
	{
		CARP_ROBOT_ASSERT(m_in_file.is_open(), u8"ReadReal Failed: in_file open failed");

		cr_real value = 0;
		m_in_file.read((char*)&value, sizeof(cr_real));
		CARP_ROBOT_ASSERT(m_in_file.gcount() == sizeof(cr_real), u8"ReadReal Failed: length error 1");

		return value;
	}
	// ��ȡ����������
	void ReadRealArray(cr_real* value, size_t count)
	{
		CARP_ROBOT_ASSERT(m_in_file.is_open(), u8"ReadRealArray Failed: in_file open failed");

		// �ȶ�ȡ��С�������
		int length = 0;
		m_in_file.read((char*)&length, sizeof(int));
		CARP_ROBOT_ASSERT(m_in_file.gcount() == sizeof(int), u8"ReadRealArray Failed: length error 1");
		CARP_ROBOT_ASSERT(length == count, u8"ReadRealArray Failed: length error 2");

		// ��ȡ����
		m_in_file.read((char*)value, sizeof(cr_real) * count);
		CARP_ROBOT_ASSERT(m_in_file.gcount() == sizeof(cr_real) * count, u8"ReadRealArray Failed: length error 1");
	}
	// ��ȡһ��������
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

// �����
class CarpRandom
{
public:
	static std::mt19937& GetGen()
	{
		thread_local static std::random_device rd;
		thread_local static std::mt19937 gen(rd());
		return gen;
	}
};

// �����洢����ʽ������ά�������Ϣ
class CarpRobotDim
{
public:
	// ���캯������ʼ��
	CarpRobotDim() { }
	CarpRobotDim(const std::vector<int>& d) : m_d(d) { m_total = m_d.empty() ? 0 : 1; for (size_t i = 0; i < m_d.size(); ++i) m_total *= m_d[i]; }

public:
	// ��ȡ�ܴ�С
	int GetTotalSize() const { return m_total; }

public:
	// ǿ�����
	bool Equal(const CarpRobotDim& right) const { return m_total == right.m_total && m_d == right.m_d; }

public:
	// ȷ�ϵ�ǰ�Ƕ�άʱ����ȡ��һ��ά�ȵĴ�С
	int Rows() const { return m_d.empty() ? 0 : m_d[0]; }
	// ȷ�ϵ�ǰ�Ƕ�άʱ����ȡ�ڶ���ά�ȵĴ�С�����û�оͷ���1
	int Cols() const { return m_d.size() > 1 ? m_d[1] : 1; }
	// ��ȡ��ά
	int Count() const { return (int)m_d.size(); }
	// �����ȡ
	int operator[](size_t index) const { return index >= 0 && index < m_d.size() ? m_d[index] : 1; }
	// ��ȡ
	int Get(int index) const { return index >= 0 && index < m_d.size() ? m_d[index] : 1; }
	// ����
	void Set(int index, int dim) { if (index < 0 || index >= (int)m_d.size()) return; m_d[index] = dim; }
	// ���ô�С
	void Resize(int size) { m_d.resize(size); m_total = m_d.empty() ? 0 : 1; for (size_t i = 0; i < m_d.size(); ++i) m_total *= m_d[i]; }

public:
	// ɾ��ĳ��ά��
	void Delete(int index)
	{
		// ����±곬������ô�Ͳ�ɾ��
		int nd = (int)m_d.size();
		if (index >= nd) return;

		// ���ɾ���������һ��ά��
		if (index == nd - 1)
		{
			// �����ǰֻ��һ��ά���ˣ���ô��ά������Ϊ1
			// ��������һ��ɾ��
			if (nd == 1)
				m_d[0] = 1;
			else
				m_d.resize(nd - 1);
		}
		else
		{
			// ��ǰ����
			for (; index + 1 < nd; ++index)
				m_d[index] = m_d[index + 1];
			// Ȼ��ɾ�����һ��
			m_d.resize(nd - 1);
		}

		// ���¼���
		m_total = m_d.empty() ? 0 : 1; for (size_t i = 0; i < m_d.size(); ++i) m_total *= m_d[i];
	}

public:
	// ���л�
	void Serialize(CarpRobotModelSerializer& file)
	{
		file.WriteInt((int)m_d.size());
		for (size_t i = 0; i < m_d.size(); ++i)
			file.WriteInt(m_d[i]);
	}
	// �����л�
	void Deserialize(CarpRobotModelDeserializer& file)
	{
		m_d.resize(file.ReadInt());
		for (size_t i = 0; i < m_d.size(); ++i)
			m_d[i] = file.ReadInt();
		m_total = m_d.empty() ? 0 : 1;
		for (size_t i = 0; i < m_d.size(); ++i) m_total *= m_d[i];
	}
	// ת�ַ���
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
	// �������ά�ȵ����飬������2��3��4����ôd����ĳ���Ϊ3����0��Ԫ����2����2��Ԫ����3��������Ԫ����4
	std::vector<int> m_d;
	// �ܴ�С
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
	// ����һ������
	CarpRobotTensor() { }
	CarpRobotTensor(const CarpRobotDim& d, bool init_zero=false) { SetDim(d, init_zero); }
	CarpRobotTensor(const CarpRobotDim& d, cr_real* value) { m_dim = d; m_value = value; m_shared = true; }
	CarpRobotTensor(const CarpRobotTensor& t) { Copy(t); }
	~CarpRobotTensor() { ReleaseMemory(); }
	void operator = (const CarpRobotTensor& t) { Copy(t); }

public:
	// ����������Tensor�ڴ�
	void RefrenceTensor(const CarpRobotTensor& t)
	{
		if (&t == this) return;
		ReleaseMemory();
		m_dim = t.m_dim;
		m_value = t.m_value;
		m_shared = true;
	}
	// ֱ�������ڴ�
	void RefrenceMemory(const CarpRobotDim& dim, cr_real* value)
	{
		ReleaseMemory();
		m_dim = dim;
		m_value = value;
		m_shared = true;
	}

	// ��ȡ�ڴ�
	cr_real* GetValue() const { return m_value; }

	// ����
	void Copy(const CarpRobotTensor& t)
	{
		if (&t == this) return;

		ReleaseMemory();
		m_dim = t.m_dim;
		// �����ڴ棬������ֵ
		auto total_bytes = m_dim.GetTotalSize() * sizeof(cr_real);
		if (total_bytes > 0 && t.m_value)
		{
			m_value = (cr_real*)malloc(total_bytes);
			memcpy(m_value, t.m_value, total_bytes);
		}
	}

	// ����ά����Ϣ��
	void SetDim(const CarpRobotDim& dim, bool init_zero = false)
	{
		// ���ά�ȸ߶�һ�£�ֻ��Ҫ�ж��Ƿ���Ҫ����
		if (m_dim.Equal(dim))
		{
			if (init_zero && m_dim.GetTotalSize() > 0) Zero();
			return;
		}

		// �ͷ��ڴ�
		ReleaseMemory();
		m_dim = dim;

		// ��ȡ�ܴ�С
		auto real_count = m_dim.GetTotalSize();
		if (real_count <= 0) return;

		// �����ڴ�
		m_value = (cr_real*)malloc(real_count * sizeof(cr_real));
		if (init_zero) Zero();
	}

	// ��ȡά��
	const CarpRobotDim& GetDim() const { return m_dim; }

public:
	// תΪ�ַ���չʾ
	std::string ToString() const
	{
		std::string result = m_dim.ToString() + "\n";
		int total_size = m_dim.GetTotalSize();
		// ����Ƕ�ά����ô�ͽ��ж�άչʾ
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
		// ����ֱ��ʹ��һάչʾ
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
	// �ͷ��ڴ�
	void ReleaseMemory()
	{
		if (m_value && m_shared == false) free(m_value);
		m_value = nullptr;
		m_shared = false;
	}

private:
	bool m_shared = false;	// �Ƿ��ǹ�������Tensor���ڴ�
	cr_real* m_value = nullptr;		// �ڴ�
	CarpRobotDim m_dim;		// ��ǰ������ά����Ϣ

public:
	// ����Ϊ����
	void Constant(cr_real c) { tvec() = tvec().constant(c); }
	// ȫ������Ϊ0
	void Zero() { Constant(0); }
	// �����
	void RandomizeUniform(cr_real left=0.0f, cr_real right=1.0f)
	{
		std::uniform_real_distribution<cr_real> distribution(left, right);
		auto b = [&] { return distribution(CarpRandom::GetGen()); };
		std::generate(m_value, m_value + m_dim.GetTotalSize(), b);
	}
	// �����
	void RandomizeBernoulli(cr_real p, cr_real scale)
	{
		std::bernoulli_distribution distribution(p);
		auto b = [&] {return distribution(CarpRandom::GetGen()) * scale; };
		std::generate(m_value, m_value + m_dim.GetTotalSize(), b);
	}

	void Logsumexp(CarpRobotTensor& m, CarpRobotTensor& z, int axis=0) const
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= 2, u8"TensorTools::logsumexp Ŀǰֻ֧���������߾���");

		// ���axisΪ0����ôother_axis = 1
		// ���axisΪ1����ôother_axis = 0
		int other_axis = axis ^ 1;

		// �����һ��ά����1��˵����ǰ������һά�ģ����Ե���������
		if (m_dim[other_axis] == 1)
		{
			// maximum ��ʾ��ȡĳ��ά�ȵ����ֵ
			// tvec().maximum() ��ʾ��ǰ�����������Ǹ��������ʱ���õ���һ������
			m.t<0>() = tvec().maximum();
			// ȡ���Ǹ����ֵ
			cr_real mval = m.AsScalar();
			// ������������ֻ��ȥ���ֵ��Ȼ����eΪ�����Ԫ����ָ����Ȼ�������
			z.t<0>() = (tvec() - mval).exp().sum();
			// ����������������ԭ�������ֵ
			z.t<0>() = z.t<0>().log() + mval;
		}
		// ��һ��ά�Ȳ���1�������Ȼ������ά��
		else
		{
			Eigen::array<int, 1> red_axis; red_axis[0] = axis;
			// �ѵ�ǰ����תΪ(d[0], d[1], bd)��Ȼ������axisά������ȡ���ֵ�����ʱ���õ���һ��(d[other_axis], bd)
			m.tb<1>() = tb<2>().maximum(red_axis);
			// ��ȡ����ָ��
			auto* miter = m.GetValue();

			// ��Ϊ��ʱ��֧�ֶ����δ�������������д��Ϊ1������
			for (int b = 0; b < 1; ++b)
			{
				for (int i = 0; i < m_dim[other_axis]; ++i, ++miter)
				{
					// tb<2>() ά��Ϊ (d[0], d[1], bd)
					// tb<2>().chip<2>(0) ȡ���Ϊ2��ά��ƫ��Ϊ0���õ�ά��Ϊ(d[0], d[1])
					// tb<2>().chip<2>(0).chip(i, other_axis) ȡ���Ϊother_axis��ά�ȣ�ƫ��Ϊi���õ�ά��Ϊ(d[axis])
					// (tb<2>().chip<2>(b).chip(i, other_axis) - *miter) ͳһ��ȥ���ֵ���õ�ά��Ϊ(d[axis])

					// z.tb<1>() ά��Ϊ (d[0], bd)
					// z.tb<1>().chip<1>(0) ȡ���Ϊ1��ά��ƫ��Ϊ0���õ�ά��Ϊ(d[0])
					// z.tb<1>().chip<1>(0).chip<0>(i) ȡ���Ϊ0��ά��ƫ��Ϊi���õ�һ������
					z.tb<1>().chip<1>(b).chip<0>(i) = (tb<2>().chip<2>(b).chip(i, other_axis) - *miter).exp().sum();
					z.tb<1>().chip<1>(b).chip<0>(i) = z.tb<1>().chip<1>(b).chip<0>(i).log() + *miter;
				}
			}
		}
	}
public:
	// ��ȡ������ά�������
	Eigen::Map<Eigen::MatrixXf> m() { return Eigen::Map<Eigen::MatrixXf>(m_value, m_dim.Rows(), m_dim.Cols()); }
	const Eigen::Map<Eigen::MatrixXf> m() const { return Eigen::Map<Eigen::MatrixXf>(m_value, m_dim.Rows(), m_dim.Cols()); }

	Eigen::Map<Eigen::VectorXf> vec() { return Eigen::Map<Eigen::VectorXf>(m_value, m_dim.GetTotalSize()); }
	const Eigen::Map<Eigen::VectorXf> vec() const { return Eigen::Map<Eigen::VectorXf>(m_value, m_dim.GetTotalSize()); }

	// ʹ�õ�ǰ���ݣ�����һ��ֻ��һά��Tensor
	Eigen::TensorMap<Eigen::Tensor<cr_real, 1>> tvec() { return Eigen::TensorMap<Eigen::Tensor<cr_real, 1>>(m_value, m_dim.GetTotalSize()); }
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 1>> tvec() const { return Eigen::TensorMap<Eigen::Tensor<cr_real, 1>>(m_value, m_dim.GetTotalSize()); }

	Eigen::TensorMap<Eigen::Tensor<cr_real, 2>> tbvec() { return Eigen::TensorMap<Eigen::Tensor<cr_real, 2>>(m_value, m_dim.GetTotalSize(), 1); }
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 2>> tbvec() const { return Eigen::TensorMap<Eigen::Tensor<cr_real, 2>>(m_value, m_dim.GetTotalSize(), 1); }

	// ʹ�õ�ǰ���ݣ�����һ��Orderά�ȵ�Tensor����
	template <int Order>
	Eigen::TensorMap<Eigen::Tensor<cr_real, Order>> t()
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= Order, u8"�Ƿ�ά����Ϣ t<" << Order << ">(): dim=" << m_dim.ToString());
		std::array<int, Order> dim; for (int i = 0; i < Order; ++i) dim[i] = m_dim[i];
		return Eigen::TensorMap<Eigen::Tensor<cr_real, Order>>(m_value, dim);
	}
	template <int Order>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, Order>> t() const
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= Order, u8"�Ƿ�ά����Ϣ t<" << Order << ">(): dim=" << m_dim.ToString());
		std::array<int, Order> dim; for (int i = 0; i < Order; ++i) dim[i] = m_dim[i];
		return Eigen::TensorMap<Eigen::Tensor<cr_real, Order>>(m_value, dim);
	}

	template <>
	Eigen::TensorMap<Eigen::Tensor<cr_real, 0>> t<0>()
	{
		CARP_ROBOT_ASSERT(m_dim.Count() == 1 && m_dim.GetTotalSize() == 1, u8"�Ƿ�ά����Ϣ t<0>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 0>>(m_value);
	}
	template <>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 0>> t<0>() const
	{
		CARP_ROBOT_ASSERT(m_dim.Count() == 1 && m_dim.GetTotalSize() == 1, u8"�Ƿ�ά����Ϣ t<0>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 0>>(m_value);
	}

	template <>
	Eigen::TensorMap<Eigen::Tensor<cr_real, 1>> t<1>()
	{
		CARP_ROBOT_ASSERT(m_dim.GetTotalSize() == m_dim.Rows(), u8"�Ƿ�ά����Ϣ t<1>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 1>>(m_value, m_dim[0]);
	}
	template <>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 1>> t<1>() const
	{
		CARP_ROBOT_ASSERT(m_dim.GetTotalSize() == m_dim.Rows(), u8"�Ƿ�ά����Ϣ t<1>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 1>>(m_value, m_dim[0]);
	}

	template <>
	Eigen::TensorMap<Eigen::Tensor<cr_real, 2>> t<2>()
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= 2, u8"�Ƿ�ά����Ϣ t<2>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 2>>(m_value, m_dim[0], m_dim[1]);
	}
	template <>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 2>> t<2>() const
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= 2, u8"�Ƿ�ά����Ϣ t<2>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 2>>(m_value, m_dim[0], m_dim[1]);
	}

	template <>
	Eigen::TensorMap<Eigen::Tensor<cr_real, 3>> t<3>()
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= 3, u8"�Ƿ�ά����Ϣ t<3>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 3>>(m_value, m_dim[0], m_dim[1], m_dim[2]);
	}
	template <>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 3>> t<3>() const
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= 3, u8"�Ƿ�ά����Ϣ t<3>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 3>>(m_value, m_dim[0], m_dim[1], m_dim[2]);
	}

	template <>
	Eigen::TensorMap<Eigen::Tensor<cr_real, 4>> t<4>()
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= 4, u8"�Ƿ�ά����Ϣ t<4>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 4>>(m_value, m_dim[0], m_dim[1], m_dim[2], m_dim[3]);
	}
	template <>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 4>> t<4>() const
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= 4, u8"�Ƿ�ά����Ϣ t<4>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 4>>(m_value, m_dim[0], m_dim[1], m_dim[2], m_dim[3]);
	}

	// ʹ�õ�ǰ���ݣ�����һ�����������ݵ�Orderά�ȵ�Tensor����
	template <int Order>
	Eigen::TensorMap<Eigen::Tensor<cr_real, Order + 1>> tb()
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= Order, u8"�Ƿ�ά����Ϣ tb<" << Order << ">(): dim=" << m_dim.ToString());
		std::array<int, Order + 1> dim; for (int i = 0; i < Order; ++i) dim[i] = m_dim[i];
		m_dim[Order] = 1;
		return Eigen::TensorMap<Eigen::Tensor<cr_real, Order + 1>>(m_value, dim);
	}
	template <int Order>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, Order + 1>> tb() const
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= Order, u8"�Ƿ�ά����Ϣ tb<" << Order << ">(): dim=" << m_dim.ToString());
		std::array<int, Order + 1> dim; for (int i = 0; i < Order; ++i) dim[i] = m_dim[i];
		m_dim[Order] = 1;
		return Eigen::TensorMap<Eigen::Tensor<cr_real, Order>>(m_value, dim);
	}

	template <>
	Eigen::TensorMap<Eigen::Tensor<cr_real, 1>> tb<0>()
	{
		CARP_ROBOT_ASSERT(m_dim.Count() == 1 && m_dim.GetTotalSize() == 1, u8"�Ƿ�ά����Ϣ tb<0>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 1>>(m_value, 1);
	}
	template <>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 1>> tb<0>() const
	{
		CARP_ROBOT_ASSERT(m_dim.Count() == 1 && m_dim.GetTotalSize() == 1, u8"�Ƿ�ά����Ϣ tb<0>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 1>>(m_value, 1);
	}

	template <>
	Eigen::TensorMap<Eigen::Tensor<cr_real, 2>> tb<1>()
	{
		CARP_ROBOT_ASSERT(m_dim.GetTotalSize() == m_dim.Rows(), u8"�Ƿ�ά����Ϣ tb<1>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 2>>(m_value, m_dim[0], 1);
	}
	template <>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 2>> tb<1>() const
	{
		CARP_ROBOT_ASSERT(m_dim.GetTotalSize() == m_dim.Rows(), u8"�Ƿ�ά����Ϣ tb<1>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 2>>(m_value, m_dim[0], 1);
	}

	template <>
	Eigen::TensorMap<Eigen::Tensor<cr_real, 3>> tb<2>()
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= 2, u8"�Ƿ�ά����Ϣ tb<2>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 3>>(m_value, m_dim[0], m_dim[1], 1);
	}
	template <>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 3>> tb<2>() const
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= 2, u8"�Ƿ�ά����Ϣ tb<2>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 3>>(m_value, m_dim[0], m_dim[1], 1);
	}

	template <>
	Eigen::TensorMap<Eigen::Tensor<cr_real, 4>> tb<3>()
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= 3, u8"�Ƿ�ά����Ϣ tb<3>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 4>>(m_value, m_dim[0], m_dim[1], m_dim[2], 1);
	}
	template <>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 4>> tb<3>() const
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= 3, u8"�Ƿ�ά����Ϣ tb<3>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 4>>(m_value, m_dim[0], m_dim[1], m_dim[2], 1);
	}

	template <>
	Eigen::TensorMap<Eigen::Tensor<cr_real, 5>> tb<4>()
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= 4, u8"�Ƿ�ά����Ϣ tb<4>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 5>>(m_value, m_dim[0], m_dim[1], m_dim[2], m_dim[3], 1);
	}
	template <>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 5>> tb<4>() const
	{
		CARP_ROBOT_ASSERT(m_dim.Count() <= 4, u8"�Ƿ�ά����Ϣ tb<4>(): dim=" << m_dim.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 5>>(m_value, m_dim[0], m_dim[1], m_dim[2], m_dim[3], 1);
	}

public:
	// ������������
	cr_real AsScalar() const { CARP_ROBOT_ASSERT(m_dim.GetTotalSize() == 1, u8"��ǰTensor���Ǳ���"); return m_value[0]; }
	// ������������ 
	std::vector<cr_real> AsVector() const
	{
		std::vector<cr_real> result;
		result.resize(m_dim.GetTotalSize());
		memcpy(result.data(), m_value, sizeof(cr_real) * result.size());
		return result;
	}
	// �������������ҷ�����������ֵ�����Ǹ��±�
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

	// �������������ҷ�����������ֵ�����Ǹ�ֵ
	cr_real AsVectorAndMaxValue() const { return m_value[AsVectorAndArgmax()]; }

	// �������������ҷ�����������ֵ�����Ǹ�ֵ
	cr_real AsVectorAndGetValue(int index) const { return index >= 0 && index < m_dim.GetTotalSize() ? m_value[index] : 0; }

public:
	void Serialize(CarpRobotModelSerializer& file) { m_dim.Serialize(file); file.WriteRealArray(m_value, m_dim.GetTotalSize()); }
	void Deserialize(CarpRobotModelDeserializer& file) { CarpRobotDim tmp; tmp.Deserialize(file); SetDim(tmp); file.ReadRealArray(m_value, m_dim.GetTotalSize()); }
};

class ICarpRobotParameter
{
public:
	virtual ~ICarpRobotParameter() {}

public:
	// ��ȡ������
	virtual const std::string& GetFullName() const = 0;
};

// ���������
class CarpRobotParameter : public ICarpRobotParameter
{
public:
	CarpRobotParameter(const CarpRobotDim& dim, const std::string& name)
		: m_name(name)
	{
		// ����ֵ
		m_values.SetDim(dim, true);
		// �ݶ�ֵ
		m_grad.SetDim(dim, true);
	}
	virtual ~CarpRobotParameter() {}

public:
	// �ۼ�Ȩ�� grad <- grad + d
	void AccumulateGrad(const CarpRobotTensor& d)
	{
		// ����ݶȲ�Ϊ0
		m_has_grad = true;

		// �ۼ��ݶ�
		m_grad.tvec() += d.tvec();
	}

	// ���������Ϊ0
	void Clear()
	{
		// ������ñ��Ϊ��0
		m_has_grad = false;

		// ���m_grad��ֵ���ھ���0
		if (m_grad.GetDim().GetTotalSize() > 0) m_grad.Zero();
	}

	// ��ȡ����
	CarpRobotTensor& GetValue() { return m_values; }
	// ��ȡ�ݶ�
	CarpRobotTensor& GetGradient() { return m_grad; }

public:
	// ��ȡ������
	virtual const std::string& GetFullName() const override { return m_name; }

public:
	// ���л�
	void Serialize(CarpRobotModelSerializer& file, const char* name)
	{
		file.WriteString(name ? name : "Parameter");
		m_values.Serialize(file);
	}
	// �����л�
	void Deserialize(CarpRobotModelDeserializer& file, const char* name)
	{
		std::string name_copy = "Parameter";
		if (name) name_copy = name;
		std::string read_name = file.ReadString();
		CARP_ROBOT_ASSERT(read_name == name_copy, u8"�����л����ƴ���, ��ǰ��:" << read_name << "Ӧ����:" << name_copy);
		m_values.Deserialize(file);
	}

private:
	std::string m_name = "Parameter";				// ��ǰ���������֣����������õ�
	CarpRobotTensor m_values;				// ��ǰ������ֵ
	CarpRobotTensor m_grad;					// ��ǰ�����������
	bool m_has_grad = false;			// �Ƿ��������
};

// ����������ռ�
class CarpRobotParameterCollection
{
public:
	CarpRobotParameterCollection() { }
	virtual ~CarpRobotParameterCollection()
	{
		// �ͷ����в����ڴ�
		for (size_t i = 0; i < m_params.size(); ++i)
			delete m_params[i];
	}

public:
	// ���Ӳ���
	virtual CarpRobotParameter* AddParameters(const CarpRobotDim& d, const std::string& name = "")
	{
		// ������������
		auto* p = new CarpRobotParameter(d, name);
		// ��ʼ��Ϊ���ֵ
		p->GetValue().RandomizeUniform();
		// ���ӵ��б�
		m_params.push_back(p);
		return p;
	}

	// ���ز����б�
	virtual const std::vector<CarpRobotParameter*>& GetParameters() const { return m_params; }

public:
	// ��ȡ�����ռ�������
	const std::string& GetFullName() const { return m_name; }

	// �����ַ�����Ϣ
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

	// ���л�
	void Serialize(CarpRobotModelSerializer& file)
	{
		for (size_t i = 0; i < m_params.size(); ++i)
			m_params[i]->Serialize(file, nullptr);
	}
	// �����л�
	void Deserialize(CarpRobotModelDeserializer& file)
	{
		for (size_t i = 0; i < m_params.size(); ++i)
			m_params[i]->Deserialize(file, nullptr);
	}

	void Load(const char* file_path)
	{
		CarpRobotModelDeserializer file;
		if (!file.Open(file_path)) return;
		Deserialize(file);
	}

	void Save(const char* file_path)
	{
		CarpRobotModelSerializer file;
		if (!file.Open(file_path)) return;
		Serialize(file);
	}

private:
	std::vector<CarpRobotParameter*> m_params;					// Parameter�����ļ���
	
private:
	std::string m_name;									// �ռ�������
};

// ����ڵ�
class CarpRobotNode
{
public:
	CarpRobotNode() {};
	// ������ͼ�е��±�
	CarpRobotNode(const std::vector<int>& args) : m_args(args) {}
	virtual ~CarpRobotNode() {}

public:
	// ��ȡ�����±��б�
	const std::vector<int>& GetArgs() const { return m_args; }
	// ��ȡ�����ά����Ϣ
	const CarpRobotDim& GetDim() const { return m_dim_out; }

public:
	virtual void Dim(const std::vector<const CarpRobotDim*>& xs) = 0;
	// ִ����ǰ����
	virtual void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) = 0;
	// ִ�з������
	virtual void Backward(const std::vector<const CarpRobotTensor*>& xs,// �ýڵ������ֵ
		const CarpRobotTensor& fx,										// �ýڵ�����ֵ
		const CarpRobotTensor& dEdf,									// �ýڵ������ڵ�������
		int xs_i,														// �ýڵ������ڵ�����
		CarpRobotTensor& dEdxi) = 0;									// ����ڵ�������
	
protected:
	// ��������ڵ���±�
	std::vector<int> m_args;
	// �ڵ������ά����Ϣ
	CarpRobotDim m_dim_out;
};

// ��������ڵ�
class CarpRobotInputNode : public CarpRobotNode
{
public:
	// ֵ����
	CarpRobotInputNode(const CarpRobotDim& dim, const std::vector<cr_real>& data) : m_dim(dim), m_data(data), m_pdata(&m_data) {}
	// ��ַ����
	CarpRobotInputNode(const CarpRobotDim& dim, const std::vector<cr_real>* pdata) : m_dim(dim), m_pdata(pdata) {}

protected:
	void Dim(const std::vector<const CarpRobotDim*>& xs) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 0, u8"����ڵ㲻���в���");
		m_dim_out = m_dim;
	}

	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		// ֱ�������ڴ�
		fx.RefrenceMemory(m_dim, (cr_real*)m_pdata->data());
	}

	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		// ����ڵ㣬���ܷ��򴫲�
		CARP_ROBOT_ASSERT(0, u8"����ڵ㲻�ܷ��򴫲�: i = " << xs_i);
	}

public:
	CarpRobotDim m_dim;
	std::vector<cr_real> m_data;
	const std::vector<cr_real>* m_pdata;
};

// ��������ڵ�
class CarpRobotScalarInputNode : public CarpRobotNode
{
public:
	// ֵ����
	CarpRobotScalarInputNode(cr_real scalar) : m_data(scalar), m_pdata(&m_data), m_dim({ 1 }) {}
	// ��ַ����
	CarpRobotScalarInputNode(const cr_real* pscalar) : m_pdata(pscalar), m_dim({ 1 }) {}

protected:
	void Dim(const std::vector<const CarpRobotDim*>& xs) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 0, u8"����ڵ㲻���в���");
		m_dim_out = m_dim;
	}

	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		// ֱ�������ڴ�
		fx.RefrenceMemory(m_dim, (cr_real*)m_pdata);
	}
	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		// ����ڵ㣬���ܷ��򴫲�
		CARP_ROBOT_ASSERT(0, u8"����ڵ㲻�ܷ��򴫲�: i = " << xs_i);
	}

public:
	CarpRobotDim m_dim;
	cr_real m_data = 0;
	const cr_real* m_pdata = nullptr;
};

// �����ڵ�����
class ICarpRobotParameterNode : public CarpRobotNode
{
public:
	virtual ~ICarpRobotParameterNode() {}

public:
	// ���򴫲�
	virtual void AccumulateGrad(const CarpRobotTensor& graph) {}
};

// ��ͨ��������ڵ�
class CarpRobotParameterNode : public ICarpRobotParameterNode
{
public:
	CarpRobotParameterNode(CarpRobotParameter* parameter) : m_parameter(parameter) {}

public:
	// ִ�з��򴫵��ݶ�
	void AccumulateGrad(const CarpRobotTensor& grad) override { m_parameter->AccumulateGrad(grad); }

protected:
	void Dim(const std::vector<const CarpRobotDim*>& xs) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 0, u8"�����ڵ㲻���в���");
		m_dim_out = m_parameter->GetValue().GetDim();
	}

	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		fx.tvec() = m_parameter->GetValue().tvec();
	}

	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		CARP_ROBOT_ASSERT(0, u8"�����ڵ㲻�ܷ��򴫲�: i = " << xs_i);
	}

private:
	CarpRobotParameter* m_parameter = nullptr;
};

// ������������ڵ�
class CarpRobotConstParameterNode : public CarpRobotNode
{
public:
	CarpRobotConstParameterNode(CarpRobotParameter* parameter) : m_parameter(parameter) { }

protected:
	void Dim(const std::vector<const CarpRobotDim*>& xs) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 0, u8"�����ڵ㲻���в���");
		m_dim_out = m_parameter->GetValue().GetDim();
	}

	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		fx.tvec() = m_parameter->GetValue().tvec();
	}
	
	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		CARP_ROBOT_ASSERT(0, u8"�����ڵ㲻�ܷ��򴫲�: i = " << xs_i);
	}

private:
	CarpRobotParameter* m_parameter = nullptr;
};

// һ����������
// ������ y = -x_1
// �󵼺��� dx_1 = -1
class CarpRobotNegateNode : public CarpRobotNode
{
public:
	CarpRobotNegateNode(const std::vector<int>& a) : CarpRobotNode(a) {}

protected:
	void Dim(const std::vector<const CarpRobotDim*>& xs) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"CarpRobotNegateNode ������һ������");
		m_dim_out = *xs[0];
	}

	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		// v��ÿ��Ԫ��ȡ����
		fx.tvec() = -xs[0]->tvec();
	}

	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		// ����Ҫ��ôдdEdxi.tvec() += dEdf.tvec() * -1; ����Ϊ�˼���һ���˷����㣬����д����������
		dEdxi.tvec() -= dEdf.tvec();
	}
};

// ���������ӷ�
// ������ y = x_1 + x_2
// �󵼺��� dx_1 = 1
// �󵼺��� dx_2 = 1
class CarpRobotCwiseSumNode : public CarpRobotNode
{
public:
	CarpRobotCwiseSumNode(const std::vector<int>& a) : CarpRobotNode(a) {}

protected:
	void Dim(const std::vector<const CarpRobotDim*>& xs) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 2, "CarpRobotCwiseSumNode ��������������");
		
		for (int i = 0; i < std::min(xs[0]->Count(), xs[1]->Count()); ++i)
			CARP_ROBOT_ASSERT((*xs[0])[i] == (*xs[1])[i] || std::min((*xs[0])[i], (*xs[1])[i]) == 1
				, u8"CwiseSum: ����������ά����Ȼ�����1");
		
		std::vector<int> dims;
		// ����ά�ȱȽϴ��
		for (int i = 0; i < std::max(xs[0]->Count(), xs[1]->Count()); ++i)
		{
			// �����ǰ�±궼С����������ά����������ô��ȡ�����Ǹ�
			if (i < std::min(xs[0]->Count(), xs[1]->Count()))
				dims.push_back(std::max((*xs[0])[i], (*xs[1])[i]));
			// ����±�С��xs[0]����ô��ȡxs[0]��Ӧ��ά��
			else if (i < xs[0]->Count())
				dims.push_back((*xs[0])[i]);
			// ����±�С��xs[1]����ô��ȡxs[1]��Ӧ��ά��
			else
				dims.push_back((*xs[1])[i]);
		}

		// ����ά��
		m_dim_out = CarpRobotDim(dims);
	}

	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 2, u8"CarpRobotCwiseSumNode ��������������");
		
		auto& dim_left = xs[0]->GetDim();
		auto& dim_right = xs[1]->GetDim();

		// ��iƫ�Ƶ�����ȵ�λ��
		int i = 0;
		while (i < fx.GetDim().Count() && dim_left[i] == dim_right[i])
			i++;

		// ��������˵����ȫ�����
		if (i == fx.GetDim().Count())
		{
			// ֱ�Ӱ�������Ӽ���
			fx.tvec() = xs[0]->tvec() + xs[1]->tvec();
		}
		else
		{
			// ������ɢ����
			Eigen::array<ptrdiff_t, 5> bcast_left = { 1,1,1,1,1 }, bcast_right = { 1,1,1,1,1 };

			// �������Լ��ұ��Ƿ���Ҫ��ɢ
			bool has_left = false, has_right = false;

			// ����ʣ�µ�ά��
			for (; i < fx.GetDim().Count(); ++i)
			{
				// �����ߵ�ά�ȴ����ұߣ���ô�ұ���Ҫ��ɢ
				if (dim_left[i] > dim_right[i])
				{
					has_right = true;
					bcast_right[i] = dim_left[i];
				}
				// �����ߵ�ά��С���ұߣ���ô�����Ҫ��ɢ
				else if (dim_left[i] < dim_right[i])
				{
					has_left = true;
					bcast_left[i] = dim_right[i];
				}
			}

			// ������ɢ���������
			if (has_right && has_left)
				fx.tb<4>() = xs[0]->tb<4>().broadcast(bcast_left) + xs[1]->tb<4>().broadcast(bcast_right);
			else if (has_right)
				fx.tb<4>() = xs[0]->tb<4>() + xs[1]->tb<4>().broadcast(bcast_right);
			else
				fx.tb<4>() = xs[0]->tb<4>().broadcast(bcast_left) + xs[1]->tb<4>();
		}
	}

	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		// ����������������ȵ�ά�ȵ�����
		int n_red = 0;
		for (int j = 0; j < fx.GetDim().Count(); ++j)
			n_red += xs[xs_i]->GetDim()[j] != fx.GetDim()[j] ? 1 : 0;

		// �����ȫ��ȣ���ô��ֱ�����
		if (n_red == 0)
		{
			dEdxi.tvec() += dEdf.tvec();
		}
		else
		{
			CARP_ROBOT_ASSERT(n_red < 5, u8"����ֻ֧�ֵ�4��ά��");
			// ���ݲ���ȵ�ά��������ѡ��ͬ�ķ��򴫲�����
			if (n_red == 1) BackwardImpl<1>(xs, fx, dEdf, xs_i, dEdxi);
			else if (n_red == 2) BackwardImpl<2>(xs, fx, dEdf, xs_i, dEdxi);
			else if (n_red == 3) BackwardImpl<3>(xs, fx, dEdf, xs_i, dEdxi);
			else if (n_red == 4) BackwardImpl<4>(xs, fx, dEdf, xs_i, dEdxi);
		}
	}

	template <int ReductionOrder>
	void BackwardImpl(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		int xs_i,
		CarpRobotTensor& dEdxi) const {

		Eigen::array<ptrdiff_t, ReductionOrder> red_axis;
		if (ReductionOrder > 0) red_axis[ReductionOrder - 1] = 4;
		int curr_red_axis = 0;

		// morph���һ��1���������������������
		// ��Ϊ��ʱ��֧�ֶ����Σ�����д��1
		Eigen::array<ptrdiff_t, 5> morph = { 1,1,1,1,1 };
		for (int di = 0; di < fx.GetDim().Count(); ++di)
		{
			// �����ǰ��ά�ȳ������룬���ߺ������һ��
			// ��ô��ά���±��¼��������Щά��Ҫ����sum����
			if ((di >= xs[xs_i]->GetDim().Count() && fx.GetDim()[di] > 1) || xs[xs_i]->GetDim()[di] != fx.GetDim()[di])
			{
				red_axis[curr_red_axis] = di;
				curr_red_axis++;
			}

			// �����������ά������һ��
			morph[di] = xs[xs_i]->GetDim()[di];
		}

		// dEdf.tb<4>() ά��Ϊ (d[0], d[1], d[2], d[3], d[4], bd)
		// dEdf.tb<4>().sum(red_axis) ��red_axis�б�ǵ�ά����ȫ�����
		// dEdf.tb<4>().sum(red_axis).reshape(morph) ά��Ϊ����Ϊ(d[0], d[1], d[2], d[3], d[4], bd)

		// �����sum�������൱�ڰѵ�ʱ��ɢ��ȥ�Ķ�Ӧֵ����������������ɢ���Ǹ�ά��
		// ���ʹ��reshape��ֵ��Ϊ��������ľ����ʽ��Ȼ���򴫲�
		dEdxi.tb<4>() += dEdf.tb<4>().sum(red_axis).reshape(morph);
	}
};

// ���� ���� ����
// ������ y = c + x_1
// �󵼺��� dx_1 = 1;
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

	void Dim(const std::vector<const CarpRobotDim*>& xs) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"CarpRobotConstantPlusXNode ������һ������");
		m_dim_out = *xs[0];
	}

	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		// unaryExpr��ʾ��xs[0]->tvec()����Ԫ�ؽ��б�����ÿ��Ԫ�ض�����ConstAddOp����
		fx.tvec() = xs[0]->tvec().unaryExpr(ConstAddOp(m_value));
	}

	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		// ����Ҫ��ôдdEdxi.tvec() += dEdf.tvec() * 1; ����Ϊ�˼���һ���˷����㣬����д����������
		dEdxi.tvec() += dEdf.tvec();
	}

private:
	cr_real m_value;
};

// ���� �� ����
// ������ y = c - x_1
// �󵼺��� dx_1 = -1;
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

	void Dim(const std::vector<const CarpRobotDim*>& xs) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"CarpRobotConstantMinusXNode ������һ������");
		m_dim_out = *xs[0];
	}

	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		// unaryExpr��ʾ��xs[0]->tvec()����Ԫ�ؽ��б�����ÿ��Ԫ�ض�����ConstAddOp����
		fx.tvec() = xs[0]->tvec().unaryExpr(ConstMinusOp(m_value));
	}
	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		// ����Ҫ��ôдdEdxi.tvec() += dEdf.tvec() * -1; ����Ϊ�˼���һ���˷����㣬����д����������
		dEdxi.tvec() -= dEdf.tvec();
	}

private:
	cr_real m_value;
};

// �����������
// ������ y = x_1 * x_2
// �󵼺��� dx_1 = dy * x_2^T
// �󵼺��� dx_2 = x_1^T * dy
class CarpRobotMatrixMultiplyNode : public CarpRobotNode
{
public:
	CarpRobotMatrixMultiplyNode(const std::vector<int>& a) : CarpRobotNode(a) {}

protected:
	void Dim(const std::vector<const CarpRobotDim*>& xs) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 2, "CarpRobotMatrixMultiplyNode ��������������");
		CARP_ROBOT_ASSERT(xs[0]->Cols() == xs[1]->Rows(), u8"CarpRobotMatrixMultiplyNode ǰ����б�����ں������:" << xs[0]->Cols() << "!=" << xs[1]->Rows());
		CARP_ROBOT_ASSERT(xs[0]->Count() <= 2 && xs[1]->Count() <= 2, "CarpRobotMatrixMultiplyNode ������������2ά��");

		m_dim_out = CarpRobotDim({ xs[0]->Rows(), xs[1]->Cols() });
	}

	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		fx.m().noalias() = xs[0]->m() * xs[1]->m();
	}

	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		CARP_ROBOT_ASSERT(xs_i < 2, "CarpRobotMatrixMultiplyNode ��������������");
		if (xs_i == 0)
			dEdxi.m().noalias() += dEdf.m() * xs[1]->m().transpose();
		else
			dEdxi.m().noalias() += xs[0]->m().transpose() * dEdf.m();
	}
};

// ���� �� ����
// ������ y = c * x_1
// �󵼺��� dx_1 = c
class CarpRobotConstScalarMultiplyNode : public CarpRobotNode
{
public:
	CarpRobotConstScalarMultiplyNode(const std::vector<int>& a, cr_real v) : CarpRobotNode(a), m_value(v) {}

protected:
	void Dim(const std::vector<const CarpRobotDim*>& xs) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, "CarpRobotConstScalarMultiplyNode ������һ������");
		m_dim_out = *xs[0];
	}

	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		fx.tvec() = xs[0]->tvec() * m_value;
	}

	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		CARP_ROBOT_ASSERT(xs_i == 0, "CarpRobotConstScalarMultiplyNode ������һ������");
		dEdxi.tvec() += dEdf.tvec() * m_value;
	}

private:
	cr_real m_value;
};

// ���� �� ����
// ������ y = x_1 dot x_2
class CarpRobotCwiseMultiplyNode : public CarpRobotNode
{
public:
	CarpRobotCwiseMultiplyNode(const std::vector<int>& a) : CarpRobotNode(a) {}

protected:
	void Dim(const std::vector<const CarpRobotDim*>& xs) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 2, "CarpRobotCwiseMultiplyNode ��������������");

		for (int i = 0; i < std::min(xs[0]->Count(), xs[1]->Count()); ++i)
			CARP_ROBOT_ASSERT((*xs[0])[i] == (*xs[1])[i] || std::min((*xs[0])[i], (*xs[1])[i]) == 1
				, u8"CwiseMultiply: ����������ά����Ȼ�����1");

		std::vector<int> dims;
		// ����ά�ȱȽϴ��
		for (int i = 0; i < std::max(xs[0]->Count(), xs[1]->Count()); ++i)
		{
			// �����ǰ�±궼С����������ά����������ô��ȡ�����Ǹ�
			if (i < std::min(xs[0]->Count(), xs[1]->Count()))
				dims.push_back(std::max((*xs[0])[i], (*xs[1])[i]));
			// ����±�С��xs[0]����ô��ȡxs[0]��Ӧ��ά��
			else if (i < xs[0]->Count())
				dims.push_back((*xs[0])[i]);
			// ����±�С��xs[1]����ô��ȡxs[1]��Ӧ��ά��
			else
				dims.push_back((*xs[1])[i]);
		}

		// ����ά��
		m_dim_out = CarpRobotDim(dims);
	}

	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 2, u8"CarpRobotCwiseSumNode ��������������");

		auto& dim_left = xs[0]->GetDim();
		auto& dim_right = xs[1]->GetDim();

		// ��iƫ�Ƶ�����ȵ�λ��
		int i = 0;
		while (i < fx.GetDim().Count() && dim_left[i] == dim_right[i])
			i++;

		// ��������˵����ȫ�����
		if (i == fx.GetDim().Count())
		{
			// ֱ�Ӱ�������Ӽ���
			fx.tvec() = xs[0]->tvec() + xs[1]->tvec();
		}
		else
		{
			// ������ɢ����
			Eigen::array<ptrdiff_t, 5> bcast_left = { 1,1,1,1,1 }, bcast_right = { 1,1,1,1,1 };

			// �������Լ��ұ��Ƿ���Ҫ��ɢ
			bool has_left = false, has_right = false;

			// ����ʣ�µ�ά��
			for (; i < fx.GetDim().Count(); ++i)
			{
				// �����ߵ�ά�ȴ����ұߣ���ô�ұ���Ҫ��ɢ
				if (dim_left[i] > dim_right[i])
				{
					has_right = true;
					bcast_right[i] = dim_left[i];
				}
				// �����ߵ�ά��С���ұߣ���ô�����Ҫ��ɢ
				else if (dim_left[i] < dim_right[i])
				{
					has_left = true;
					bcast_left[i] = dim_right[i];
				}
			}

			// ������ɢ���������
			if (has_right && has_left)
				fx.tb<4>() = xs[0]->tb<4>().broadcast(bcast_left) * xs[1]->tb<4>().broadcast(bcast_right);
			else if (has_right)
				fx.tb<4>() = xs[0]->tb<4>() * xs[1]->tb<4>().broadcast(bcast_right);
			else
				fx.tb<4>() = xs[0]->tb<4>().broadcast(bcast_left) * xs[1]->tb<4>();
		}
	}

	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		// ����������������ȵ�ά�ȵ�����
		int n_red = 0;
		bool must_red = false;
		for (int j = 0; j < fx.GetDim().Count(); ++j)
		{
			n_red += xs[xs_i]->GetDim()[j] != fx.GetDim()[j] ? 1 : 0;
			must_red = must_red || xs[0]->GetDim()[j] != xs[1]->GetDim()[j];
		}

		// �����ȫ��ȣ���ô��ֱ�����
		if (!must_red)
		{
			dEdxi.tvec() += dEdf.tvec();
		}
		else
		{
			CARP_ROBOT_ASSERT(n_red < 5, u8"����ֻ֧�ֵ�4��ά��");
			// ���ݲ���ȵ�ά��������ѡ��ͬ�ķ��򴫲�����
			if (n_red == 0) BackwardImpl<0>(xs, fx, dEdf, xs_i, dEdxi);
			else if (n_red == 1) BackwardImpl<1>(xs, fx, dEdf, xs_i, dEdxi);
			else if (n_red == 2) BackwardImpl<2>(xs, fx, dEdf, xs_i, dEdxi);
			else if (n_red == 3) BackwardImpl<3>(xs, fx, dEdf, xs_i, dEdxi);
			else if (n_red == 4) BackwardImpl<4>(xs, fx, dEdf, xs_i, dEdxi);
		}
	}

	template <int ReductionOrder>
	void BackwardImpl(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		int xs_i,
		CarpRobotTensor& dEdxi) const {

		Eigen::array<ptrdiff_t, ReductionOrder> red_axis;
		if (ReductionOrder > 0) red_axis[ReductionOrder - 1] = 4;
		int curr_red_axis = 0;

		// morph���һ��1���������������������
		// ��Ϊ��ʱ��֧�ֶ����Σ�����д��1
		Eigen::array<ptrdiff_t, 5> morph = { 1,1,1,1,1 };
		Eigen::array<ptrdiff_t, 5> bcast_other = { 1,1,1,1,1 };
		for (int di = 0; di < fx.GetDim().Count(); ++di)
		{
			// �����ǰ��ά�ȳ������룬���ߺ������һ��
			// ��ô��ά���±��¼��������Щά��Ҫ����sum����
			if (xs[xs_i]->GetDim()[di] != fx.GetDim()[di])
			{
				red_axis[curr_red_axis] = di;
				curr_red_axis++;
			}

			// �����������ά������һ��
			morph[di] = xs[xs_i]->GetDim()[di];
			if (xs[1 - xs_i]->GetDim()[di] == 1)
				bcast_other[di] = fx.GetDim()[di];
		}

		// dEdf.tb<4>() ά��Ϊ (d[0], d[1], d[2], d[3], d[4], bd)
		// dEdf.tb<4>().sum(red_axis) ��red_axis�б�ǵ�ά����ȫ�����
		// dEdf.tb<4>().sum(red_axis).reshape(morph) ά��Ϊ����Ϊ(d[0], d[1], d[2], d[3], d[4], bd)

		// �����sum�������൱�ڰѵ�ʱ��ɢ��ȥ�Ķ�Ӧֵ����������������ɢ���Ǹ�ά��
		// ���ʹ��reshape��ֵ��Ϊ��������ľ����ʽ��Ȼ���򴫲�
		dEdxi.tb<4>() += (dEdf.tb<4>() * xs[1 - xs_i]->tb<4>().broadcast(bcast_other)).sum(red_axis).reshape(morph);
	}
};

// ���� �� ����
// ������ y = x_1 / x_2
// �󵼺��� dx_1 = 1 / x_2
// �󵼺��� dx_2 = - (dy / x_2^2 * x_1)
class CarpRobotCwiseQuotientNode : public CarpRobotNode
{
public:
	CarpRobotCwiseQuotientNode(const std::vector<int>& a) : CarpRobotNode(a) {}

protected:
	void Dim(const std::vector<const CarpRobotDim*>& xs) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 2, "CarpRobotCwiseQuotientNode ��������������");

		for (int i = 0; i < std::min(xs[0]->Count(), xs[1]->Count()); ++i)
			CARP_ROBOT_ASSERT((*xs[0])[i] == (*xs[1])[i] || (*xs[1])[i] == 1
				, u8"CwiseQuotient: ����������ά����Ȼ����Ҳ���ά����1");

		std::vector<int> dims;
		// ����ά�ȱȽϴ��
		for (int i = 0; i < std::max(xs[0]->Count(), xs[1]->Count()); ++i)
		{
			// �����ǰ�±궼С����������ά����������ô��ȡ�����Ǹ�
			if (i < std::min(xs[0]->Count(), xs[1]->Count()))
				dims.push_back(std::max((*xs[0])[i], (*xs[1])[i]));
			// ����±�С��xs[0]����ô��ȡxs[0]��Ӧ��ά��
			else if (i < xs[0]->Count())
				dims.push_back((*xs[0])[i]);
			// ����±�С��xs[1]����ô��ȡxs[1]��Ӧ��ά��
			else
				dims.push_back((*xs[1])[i]);
		}

		// ����ά��
		m_dim_out = CarpRobotDim(dims);
	}

	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		// ������������ܺ�һ�£���ôֱ�ӳ�
		if (xs[0]->GetDim().GetTotalSize() == xs[1]->GetDim().GetTotalSize())
		{
			fx.tb<4>() = xs[0]->tb<4>() / xs[1]->tb<4>();
		}
		else
		{
			// ���ұ�С��ά�ȱ��Ϊ��ɢ
			Eigen::array<ptrdiff_t, 5> bcast = { 1,1,1,1,1 };
			for (int di = 0; di < xs[0]->GetDim().Count(); ++di)
			{
				if (xs[1]->GetDim()[di] == 1)
					bcast[di] = xs[0]->GetDim()[di];
			}
			// ���ұ߲�����ɢ֮��Ȼ����г���
			fx.tb<4>() = xs[0]->tb<4>() / xs[1]->tb<4>().broadcast(bcast);
		}
	}

	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		CARP_ROBOT_ASSERT(xs_i < 2, "CarpRobotCwiseQuotientNode ��������������");

		// ��ߵĲ���
		if (xs_i == 0)
		{
			// ������������ܺ�һ�£���ôֱ�ӳ�
			if (xs[0]->GetDim().GetTotalSize() == xs[1]->GetDim().GetTotalSize())
			{
				dEdxi.tb<4>() += dEdf.tb<4>() / xs[1]->tb<4>();
			}
			else
			{
				// ����ұ߲�����ά�Ⱥ���߲�һ�£���ô�ͱ��Ϊ��ɢ
				Eigen::array<ptrdiff_t, 5> bcast = { 1,1,1,1,1 };
				for (int di = 0; di < xs[0]->GetDim().Count(); ++di)
				{
					if (xs[0]->GetDim()[di] != xs[1]->GetDim()[di])
						bcast[di] = xs[0]->GetDim()[di];
				}

				// ���ұ߲�����ɢ֮��Ȼ����г���
				dEdxi.tb<4>() += dEdf.tb<4>() / xs[1]->tb<4>().broadcast(bcast);
			}
		}
		// �ұߵĲ���
		else
		{
			if (xs[0]->GetDim().GetTotalSize() == xs[1]->GetDim().GetTotalSize())
			{
				dEdxi.tb<4>() -= (dEdf.tb<4>() / xs[1]->tb<4>().square() * xs[0]->tb<4>());
			}
			else
			{
				// ͳ��ά�Ȳ���ȵ�����
				int n_red = 0;
				for (int di = 0; di < xs[0]->GetDim().Count(); ++di)
					if (xs[0]->GetDim()[di] != xs[1]->GetDim()[di]) n_red++;
				CARP_ROBOT_ASSERT(n_red < 5, u8"����ֻ֧�ֵ�4��ά��");
				if (n_red == 0)      BackwardImpl<0>(xs, fx, dEdf, xs_i, dEdxi);
				else if (n_red == 1) BackwardImpl<1>(xs, fx, dEdf, xs_i, dEdxi);
				else if (n_red == 2) BackwardImpl<2>(xs, fx, dEdf, xs_i, dEdxi);
				else if (n_red == 3) BackwardImpl<3>(xs, fx, dEdf, xs_i, dEdxi);
				else if (n_red == 4) BackwardImpl<4>(xs, fx, dEdf, xs_i, dEdxi);
			}
		}
	}

	template <int ReductionOrder>
	void BackwardImpl(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		int xs_i,
		CarpRobotTensor& dEdxi) const {

		Eigen::array<ptrdiff_t, ReductionOrder> red_axis;
		if (ReductionOrder > 0) red_axis[ReductionOrder - 1] = 4;

		// �Ѳ���ȵ�ά���±�ȫ����������
		int curr_red_axis = 0;
		for (int di = 0; di < xs[0]->GetDim().Count(); ++di)
		{
			if (xs[0]->GetDim()[di] != xs[1]->GetDim()[di])
			{
				red_axis[curr_red_axis] = di;
				curr_red_axis++;
			}
		}

		// morph���һ��1���������������������
		// ��Ϊ��ʱ��֧�ֶ����Σ�����д��1
		Eigen::array<ptrdiff_t, 5> morph = { 1,1,1,1,1 };
		for (int di = 0; di < xs[0]->GetDim().Count(); ++di)
		{
			morph[di] = xs[xs_i]->GetDim()[di];
		}

		// ������ɢ��Ϣ
		Eigen::array<ptrdiff_t, 5> bcast = { 1,1,1,1,1 };
		for (int di = 0; di < xs[0]->GetDim().Count(); ++di)
		{
			if (xs[0]->GetDim()[di] != xs[1]->GetDim()[di])
				bcast[di] = xs[0]->GetDim()[di];
		}

		CarpRobotTensor xs1_squared(xs[1]->GetDim());
		// ����xs1��ƽ��
		xs1_squared.tb<4>() = xs[1]->tb<4>().square();
		// ���򴫲�
		dEdxi.tb<4>() -= (dEdf.tb<4>() / xs1_squared.tb<4>().broadcast(bcast) * xs[0]->tb<4>()).sum(red_axis).reshape(morph);
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

	void Dim(const std::vector<const CarpRobotDim*>& xs) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"LogisticSigmoidNode ������һ������");
		m_dim_out = *xs[0];
	}

	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		fx.tvec() = xs[0]->tvec().unaryExpr(ScalarLogisticSigmoidOp());
	}

	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		dEdxi.tvec() += fx.tvec().binaryExpr(dEdf.tvec(), ScalarLogisticSigmoidBackwardOp());
	}
};

// y = tanh x_1
class CarpRobotTanhNode : public CarpRobotNode
{
public:
	CarpRobotTanhNode(const std::vector<int>& a) : CarpRobotNode(a) {}
	~CarpRobotTanhNode() {}

protected:
	struct ScalarTanhBackwardOp
	{
		inline const cr_real operator() (const cr_real& t, const cr_real& d) const { return (1 - t * t) * d; }
	};

	void Dim(const std::vector<const CarpRobotDim*>& xs) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"CarpRobotTanhNode ������һ������");
		m_dim_out = *xs[0];
	}

	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		fx.tvec() = xs[0]->tvec().tanh();
	}

	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		dEdxi.tvec() += fx.tvec().binaryExpr(dEdf.tvec(), ScalarTanhBackwardOp());
	}
};

// y = max(0,x)
class CarpRobotRectifyNode : public CarpRobotNode
{
public:
	CarpRobotRectifyNode(const std::vector<int>& a) : CarpRobotNode(a) {}
	~CarpRobotRectifyNode() {}

protected:
	void Dim(const std::vector<const CarpRobotDim*>& xs) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"CarpRobotRectifyNode ������һ������");
		m_dim_out = *xs[0];
	}

	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		fx.tvec() = xs[0]->tvec().cwiseMax(0.f);
	}

	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		int xs_i,
		CarpRobotTensor& dEdxi) override
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
	void Dim(const std::vector<const CarpRobotDim*>& xs) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"CarpRobotRectifyNode ������һ������");
		m_dim_out = *xs[0];
	}

	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
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

			// ȡ��col_x�����������Ǹ�ֵ
			m.t<0>() = col_x.tvec().maximum();
			// ��col_x������ÿ��ֵ����ȥ���ֵ��Ȼ������e��col_xÿ��ֵ�η�
			// ����֮����Ҫ��ȥ���ֵ���Ƿ�ֹ������������������ͬʱ��ȥһ�����ֵҲ����Ӱ����
			col_fx.tvec() = (col_x.tvec() - m.GetValue()[0]).exp();
			// ����e��col_xÿ��ֵ�η����ܺ�
			z.t<0>() = col_fx.tvec().sum();
			// ��e��col_xÿ��ֵ�η��������ܺͣ��Ӷ��õ�softmax
			col_fx.tvec() = col_fx.tvec() / z.GetValue()[0];

			col_x_value += size;
			col_fx_value += size;
		}
	}

	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		int xs_i,
		CarpRobotTensor& dEdxi) override
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
	void Dim(const std::vector<const CarpRobotDim*>& xs) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"CarpRobotLogSoftmaxNode ������һ������");
		m_dim_out = *xs[0];
	}

	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
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
		int xs_i,
		CarpRobotTensor& dEdxi) override
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
	CarpRobotDropoutNode(const std::vector<int>& a, cr_real v, int training) : CarpRobotNode(a), m_value(v), m_training(training), m_ptraining(&m_training) {}
	CarpRobotDropoutNode(const std::vector<int>& a, cr_real v, const int* ptraining) : CarpRobotNode(a), m_value(v), m_training(0), m_ptraining(ptraining) {}
	~CarpRobotDropoutNode() {}

protected:
	void Dim(const std::vector<const CarpRobotDim*>& xs) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"CarpRobotDropoutNode ������һ������");
		m_dim_out = *xs[0];
	}

	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		if (*m_ptraining)
		{
			m_aux_mem.SetDim(fx.GetDim());
			m_aux_mem.RandomizeBernoulli((1.f - m_value), 1.f / (1.f - m_value));
			fx.tvec() = xs[0]->tvec() * m_aux_mem.tvec();
		}
		else
		{
			fx.tvec() = xs[0]->tvec();
		}
	}

	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		if (*m_ptraining)
			dEdxi.tvec() += dEdf.tvec() * m_aux_mem.tvec();
		else
			dEdxi.tvec() += dEdf.tvec();
	}

private:
	int m_training = 0;
	const int* m_ptraining = nullptr;
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
	CarpRobotConv2DNode(const std::vector<int>& a, int stride_width, int stride_height = 1, bool padding_type = true) : CarpRobotNode(a), m_stride({stride_width, stride_height}), m_padding_type(padding_type) {}
	~CarpRobotConv2DNode() {}

protected:
	void Dim(const std::vector<const CarpRobotDim*>& xs) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 2 || xs.size() == 3, u8"Conv2D requires either two or three inputs");
		CARP_ROBOT_ASSERT((xs[0]->Count() == 2 || xs[0]->Count() == 3) && xs[1]->Count() == 4 && (*xs[1])[2] == (*xs[0])[2], u8"Conv2D requires either two or three inputs");
		CARP_ROBOT_ASSERT(!m_padding_type || ((*xs[0])[0] >= (*xs[1])[0] && (*xs[0])[1] >= (*xs[1])[1]), u8"Bad input dimensions in Conv2D: in VALID convolution, the filter size must not be greater than the feature map size");
		if (xs.size() == 3) //has bias term
			CARP_ROBOT_ASSERT((*xs[2])[0] == (*xs[1])[3] && xs[2]->Count() == 1, u8"Bad input dimensions in Conv2D");

		std::vector<int> output_shape(3);
		output_shape[2] = (*xs[1])[3];
		for (unsigned i = 0; i < 2; ++i)
		{
			float input_dim = static_cast<float>((*xs[0])[i]);
			float kernel_dim = static_cast<float>((*xs[1])[i]);
			float s = static_cast<float>(m_stride[i]);
			if (m_padding_type)
				output_shape[i] = static_cast<int>(std::ceil((input_dim - kernel_dim + 1) / s));
			else
				output_shape[i] = static_cast<int>(std::ceil(input_dim / s));
		}
		
		m_dim_out = CarpRobotDim(output_shape);
	}

	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
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
		int xs_i,
		CarpRobotTensor& dEdxi) override
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
	CarpRobotMaxPooling2DNode(const std::vector<int>& a, int kernel_width, int kernel_height, int stride_width = 1, int stride_height = 1, bool padding_type = true) : CarpRobotNode(a), m_ksize({kernel_width, kernel_height}), m_stride({ stride_width, stride_height }), m_padding_type(padding_type) {}
	~CarpRobotMaxPooling2DNode() {}

protected:
	void Dim(const std::vector<const CarpRobotDim*>& xs) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"CarpRobotMaxPooling2DNode ������һ������");

		CARP_ROBOT_ASSERT(xs[0]->Count() == 2 || xs[0]->Count() == 3, u8"Bad input dimensions in MaxPooling2D, expected 2 or 3 dimensions");
		CARP_ROBOT_ASSERT(!m_padding_type || ((*xs[0])[0] >= m_ksize[0] && (*xs[0])[1] >= m_ksize[1]), u8"Bad input dimensions in MaxPooling2D: in VALID mode, the kernel size cannot be greater than the feature map size");

		std::vector<int> output_shape(xs[0]->Count());
		if (xs[0]->Count() == 3)
			output_shape[2] = (*xs[0])[2];

		for (unsigned i = 0; i < 2; ++i)
		{
			float input_dim = static_cast<float>((*xs[0])[i]);
			float kernel_dim = static_cast<float>(m_ksize[i]);
			float s = static_cast<float>(m_stride[i]);
			if (m_padding_type)
				output_shape[i] = static_cast<int>(std::ceil((input_dim - kernel_dim + 1) / s));
			else
				output_shape[i] = static_cast<int>(std::ceil(input_dim / s));
		}
		m_dim_out = CarpRobotDim(output_shape);
	}

	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
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
		int xs_i,
		CarpRobotTensor& dEdxi) override
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
	void Dim(const std::vector<const CarpRobotDim*>& xs) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"CarpRobotSquareNode ������һ������");
		m_dim_out = *xs[0];
	}

	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		fx.tvec() = xs[0]->tvec().square();
	}

	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		int xs_i,
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

	void Dim(const std::vector<const CarpRobotDim*>& xs) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 2, u8"CarpRobotBinaryLogLossNode ��������������");
		CARP_ROBOT_ASSERT(xs[0]->Rows() == 2 || xs[0]->Count() == 1, u8"����ά����Ϣ����");
		CARP_ROBOT_ASSERT(xs[1]->Rows() == 2 || xs[1]->Count() == 1, u8"����ά����Ϣ����");

		m_dim_out = CarpRobotDim({ 1 });
	}

	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		fx.t<0>() = xs[0]->tvec().binaryExpr(xs[1]->tvec(), FBinaryLogLoss()).sum();
	}

	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		int xs_i,
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
	CarpRobotPickNegLogSoftmaxNode(const std::vector<int>& a, int label) : CarpRobotNode(a), m_label(label), m_plabel(&m_label) {}
	CarpRobotPickNegLogSoftmaxNode(const std::vector<int>& a, const int* plabel) : CarpRobotNode(a), m_label(0), m_plabel(plabel) {}
	~CarpRobotPickNegLogSoftmaxNode() {}

protected:
	void Dim(const std::vector<const CarpRobotDim*>& xs) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"CarpRobotPickNegLogSoftmaxNode ������һ������");
		CARP_ROBOT_ASSERT(xs[0]->GetTotalSize() == xs[0]->Rows(), u8"�����ά����Ϣ����");

		m_dim_out = CarpRobotDim({ 1 });
	}

	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		m_z.SetDim(CarpRobotDim({ 1 }), true);
		m_m.SetDim(CarpRobotDim({ 1 }), true);

		if (xs[0]->GetDim().Cols() == 1)
		{
			xs[0]->Logsumexp(m_m, m_z);
			fx.GetValue()[0] = xs[0]->GetValue()[*m_plabel];
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
		int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		if (xs[0]->GetDim().Cols() == 1)
		{
			dEdxi.tb<1>().chip<1>(0) += (xs[0]->tb<1>().chip<1>(0) - m_z.GetValue()[0]).exp() * dEdf.GetValue()[0];
			dEdxi.GetValue()[*m_plabel] -= dEdf.GetValue()[0];
		}
		else
		{
			CARP_ROBOT_ASSERT(0, "PickNegLogSoftmax::backward not yet implemented for multiple columns");
		}
	}
public:
	int m_label = 0;
	const int* m_plabel = nullptr;
	
private:
	CarpRobotTensor m_z;
	CarpRobotTensor m_m;
};

// x_1 is a vector
// y = (x_1)_{*pval}
class CarpRobotPickElementNode : public CarpRobotNode
{
public:
	CarpRobotPickElementNode(const std::vector<int>& a, int label, int d = 0) : CarpRobotNode(a), m_label(label), m_plabel(&m_label), m_dim(d) {}
	CarpRobotPickElementNode(const std::vector<int>& a, const int* plabel, int d = 0) : CarpRobotNode(a), m_label(0), m_plabel(plabel), m_dim(d) {}
	~CarpRobotPickElementNode() {}

protected:
	void Dim(const std::vector<const CarpRobotDim*>& xs) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"CarpRobotPickElementNode ������һ������");
		CARP_ROBOT_ASSERT(*m_plabel < (*xs[0])[m_dim], u8"�����ά����Ϣ����");

		m_dim_out = *xs[0];
		m_dim_out.Delete(m_dim);
	}

	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		fx.tb<3>() = xs[0]->tb<4>().chip(*m_plabel, m_dim);
	}

	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		dEdxi.tb<3>().chip(*m_plabel, m_dim) += dEdf.tb<2>();
	}

public:
	int m_label = 0;
	const int* m_plabel = nullptr;
	int m_dim = 0;
};

// y = reshape(x_1, --> to)
class CarpRobotReshapeNode : public CarpRobotNode
{
public:
	CarpRobotReshapeNode(const std::vector<int>& a, const CarpRobotDim& dim) : CarpRobotNode(a), m_dim(dim) {}
	~CarpRobotReshapeNode() {}

protected:
	void Dim(const std::vector<const CarpRobotDim*>& xs) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"CarpRobotReshapeNode ������һ������");
		CARP_ROBOT_ASSERT(xs[0]->GetTotalSize() == m_dim.GetTotalSize(), u8"CarpRobotReshapeNode ������ܴ�С��������һ��");

		// just point to the input memory and change dimensions
		// dimensions are handled by forward_dim
		m_dim_out = m_dim;
	}

	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		fx.tvec() = xs[0]->tvec();
	}

	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		CarpRobotTensor reshaped(dEdxi.GetDim(), dEdf.GetValue());
		dEdxi.tvec() += reshaped.tvec();
	}

private:
	CarpRobotDim m_dim;
};

//y = \sum_i x_i
class CarpRobotMeanElementsNode : public CarpRobotNode
{
public:
	CarpRobotMeanElementsNode(const std::vector<int>& a, int dim) : CarpRobotNode(a), m_dim(dim) {}
	~CarpRobotMeanElementsNode() {}

protected:
	void Dim(const std::vector<const CarpRobotDim*>& xs) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"CarpMeanElementsNode ������һ������");
		CARP_ROBOT_ASSERT(xs[0]->Count() <= 3, u8"CarpMeanElementsNode ���ֻ֧�ֵ�3��ά��");

		m_dim_out = *xs[0];
		m_dim_out.Delete(m_dim);
	}

	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		float n = (float)fx.GetDim().GetTotalSize();
		Eigen::array<ptrdiff_t, 1> reduction_axis = { m_dim };
		fx.tb<2>() = xs[0]->tb<3>().sum(reduction_axis) / n;
	}

	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		CarpRobotDim dim = xs[0]->GetDim();

		float n = (float)dim.GetTotalSize();
		Eigen::array<ptrdiff_t, 4> bcast = { 1,1,1,1 }; bcast[m_dim] = dim[m_dim];
		Eigen::array<ptrdiff_t, 4> morph = { dim[0], dim[1], dim[2], 1 }; morph[m_dim] = 1;
		dEdxi.tb<3>() += dEdf.tb<2>().reshape(morph).broadcast(bcast) / n;
	}

private:
	int m_dim = 0;
};

// y = \sum_i,j,... x[i,j,...]
class CarpRobotStdElementsNode : public CarpRobotNode
{
public:
	CarpRobotStdElementsNode(const std::vector<int>& a) : CarpRobotNode(a) {}
	~CarpRobotStdElementsNode() {}

protected:
	struct FSqrtBackward
	{
		inline cr_real operator()(cr_real t, cr_real d) const
		{
			return d / (2.f * t);
		}
	};

	void Dim(const std::vector<const CarpRobotDim*>& xs) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"CarpRobotStdElementsNode ������һ������");

		m_dim_out = CarpRobotDim({ 1 });
	}

	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		Eigen::array<ptrdiff_t, 1> red_axis = { 0 };
		Eigen::array<ptrdiff_t, 2> bcast = { (ptrdiff_t)xs[0]->GetDim().GetTotalSize(), 1 };
		Eigen::array<ptrdiff_t, 2> newaxis = { 1, 1 };
		cr_real n = (cr_real)xs[0]->GetDim().GetTotalSize();
		fx.tb<0>() = ((xs[0]->tbvec() - (xs[0]->tbvec().sum(red_axis).reshape(newaxis) / n).broadcast(bcast)).square().sum(red_axis) / n).sqrt();
	}

	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		Eigen::array<ptrdiff_t, 2> bcast = { (ptrdiff_t)xs[0]->GetDim().GetTotalSize(), 1 };
		Eigen::array<ptrdiff_t, 2> newaxis = { 1, 1 };
		Eigen::array<ptrdiff_t, 1> red_axis = { 0 };
		cr_real n = (cr_real)xs[0]->GetDim().GetTotalSize();
		dEdxi.tbvec() += (2 / n) * (xs[0]->tbvec() - (xs[0]->tbvec().sum(red_axis).reshape(newaxis) / n).broadcast(bcast)) * (fx.tbvec().binaryExpr(dEdf.tbvec(), FSqrtBackward())).broadcast(bcast);

	}
};

// y = \sum_i,j,... x[i,j,...]
class CarpRobotPickRangeNode : public CarpRobotNode
{
public:
	CarpRobotPickRangeNode(const std::vector<int>& a, int start, int end, int dim=0) : CarpRobotNode(a), m_start(start), m_end(end), m_dim(dim) {}
	~CarpRobotPickRangeNode() {}

protected:
	void Dim(const std::vector<const CarpRobotDim*>& xs) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"CarpRobotPickRangeNode ������һ������");
		CARP_ROBOT_ASSERT(m_dim < xs[0]->Count() && m_start < m_end && ((*xs[0]))[m_dim] >= m_end, u8"ʰȡ����Խ��");

		m_dim_out = *(xs[0]);
		m_dim_out.Set(m_dim, m_end - m_start);
	}

	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		Eigen::DSizes<ptrdiff_t, 5> indices(0, 0, 0, 0, 0);
		indices[m_dim] = m_start;
		Eigen::DSizes<ptrdiff_t, 5> sizes(static_cast<ptrdiff_t>(fx.GetDim()[0]),
			static_cast<ptrdiff_t>(fx.GetDim()[1]),
			static_cast<ptrdiff_t>(fx.GetDim()[2]),
			static_cast<ptrdiff_t>(fx.GetDim()[3]),
			static_cast<ptrdiff_t>(1));
		sizes[m_dim] = m_end - m_start;
		fx.tb<4>() = xs[0]->tb<4>().slice(indices, sizes);
	}

	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		Eigen::DSizes<ptrdiff_t, 5> indices(0, 0, 0, 0, 0);
		indices[m_dim] = m_start;
		Eigen::DSizes<ptrdiff_t, 5> sizes(static_cast<ptrdiff_t>(fx.GetDim()[0]),
			static_cast<ptrdiff_t>(fx.GetDim()[1]),
			static_cast<ptrdiff_t>(fx.GetDim()[2]),
			static_cast<ptrdiff_t>(fx.GetDim()[3]),
			static_cast<ptrdiff_t>(1));
		sizes[m_dim] = m_end - m_start;
		dEdxi.tb<4>().slice(indices, sizes) += dEdf.tb<4>();
	}

private:
	int m_start = 0;
	int m_end = 0;
	int m_dim = 0;
};

// y = x_1 \sum_{i=2, 4 ...} A_i * x_{i+1}
class CarpRobotAffineTransformNode : public CarpRobotNode
{
public:
	CarpRobotAffineTransformNode(const std::vector<int>& a) : CarpRobotNode(a) {}
	~CarpRobotAffineTransformNode() {}

protected:
	void Dim(const std::vector<const CarpRobotDim*>& xs) override
	{
		CARP_ROBOT_ASSERT(xs.size() % 2 == 1, u8"CarpRobotAffineTransformNode �����������������");
		if (xs.size() == 1)
		{
			m_dim_out = *xs[0];
			return;
		}

		CARP_ROBOT_ASSERT(xs[0]->Rows() == xs[1]->Rows() && xs[1]->Cols() == xs[2]->Rows(), u8"Bad dimensions for AffineTransform: ");
		CarpRobotDim d = (xs[2]->Cols() != 1 ? CarpRobotDim({ xs[0]->Rows(), xs[2]->Cols() }) : CarpRobotDim({ xs[0]->Rows() }));
		for (size_t i = 3; i < xs.size(); i += 2) {
			CARP_ROBOT_ASSERT(xs[i]->Cols() == xs[i + 1]->Rows() && d.Rows() == xs[i]->Rows() && d.Cols() == xs[i + 1]->Cols(),
				"Bad dimensions for AffineTransform: ");
		}

		m_dim_out = d;
	}

	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		if (xs.size() == 1)
		{
			fx.tvec() = xs[0]->tvec();
			return;
		}

		// Add the first matrix
		size_t b_size = xs[0]->GetDim().GetTotalSize(), fx_size = fx.GetDim().GetTotalSize();
		if (fx_size == b_size)
		{
			fx.tvec() = xs[0]->tvec();
		}
		else
		{
			cr_real* curr_ptr = fx.GetValue(), * end_ptr = curr_ptr + fx.GetDim().GetTotalSize(), * in_ptr = xs[0]->GetValue();
			do
			{
				memcpy(curr_ptr, in_ptr, sizeof(float) * b_size);
				curr_ptr += b_size;
			} while (curr_ptr != end_ptr);
		}

		// Multiply
		for (size_t i = 1; i < xs.size(); i += 2)
		{
			fx.m().noalias() += xs[i]->m() * xs[i + 1]->m();
		}
	}

	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		// Bias term
		if (xs_i == 0)
		{
			// bias term
			int dx_size = dEdxi.GetDim().GetTotalSize();
			int df_size = dEdf.GetDim().GetTotalSize();
			if (dx_size == df_size)
			{
				dEdxi.tvec() += dEdf.tvec();
			}
			else
			{
				if (dEdxi.GetDim()[1] == dEdf.GetDim()[1])
				{
					dEdxi.m().noalias() += dEdf.m();
				}
				else
				{
					CarpRobotTensor mychip;
					mychip.RefrenceMemory(dEdxi.GetDim(), dEdf.GetValue());
					int len = dEdf.GetDim()[1];
					for (int b = 0; b < len; ++b)
					{
						dEdxi.m().noalias() += mychip.m();
						mychip.RefrenceMemory(dEdxi.GetDim(), mychip.GetValue() + dx_size);
					}
				}
			}
		}
		// Left argument of matrix multiply
		else if (xs_i % 2 == 1)
			dEdxi.m().noalias() += dEdf.m() * xs[xs_i + 1]->m().transpose();
		else
			dEdxi.m().noalias() += xs[xs_i - 1]->m().transpose() * dEdf.m();
	}
};

// draw from Bernoulli(p)
class CarpRobotRandomBernoulliNode : public CarpRobotNode
{
public:
	CarpRobotRandomBernoulliNode(const std::vector<int>& a, const CarpRobotDim& dim, cr_real p, cr_real scale=1.0f) : CarpRobotNode(a), m_dim(dim), m_p(p), m_scale(scale) {}
	~CarpRobotRandomBernoulliNode() {}

public:
	void Dim(const std::vector<const CarpRobotDim*>& xs) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 0, u8"CarpRobotRandomBernoulliNode û������");
		m_dim_out = m_dim;
	}

	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		fx.RandomizeBernoulli(m_p, m_scale);
	}

	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		int xs_i,
		CarpRobotTensor& dEdxi) override
	{
	}

private:
	CarpRobotDim m_dim;
	cr_real m_p = 0.0f;
	cr_real m_scale = 1.0f;
};

// concatenate along a particular dimension
class CarpRobotConcatenateNode : public CarpRobotNode
{
public:
	CarpRobotConcatenateNode(const std::vector<int>& a, int dim) : CarpRobotNode(a), m_dim(dim) {}
	~CarpRobotConcatenateNode() {}

public:
	void Dim(const std::vector<const CarpRobotDim*>& xs) override
	{
		CARP_ROBOT_ASSERT(xs.size() > 0, u8"CarpRobotRandomBernoulliNode ����һ������");
		int new_rows = 0;
		CarpRobotDim dr = *xs[0];
		for (size_t i = 0; i < xs.size(); ++i)
		{
			CarpRobotDim c = *xs[i];
			if (dr.Count() < c.Count()) dr.Resize(c.Count());
			if (c.Count() < dr.Count()) c.Resize(dr.Count());
			// ����������ڵ�ָ��ά�ȵĴ�С�ۼ�����
			new_rows += c[m_dim];
			dr.Set(m_dim, c[m_dim]);
		}
		dr.Resize(std::max(xs[0]->Count(), m_dim + 1));
		// ���ýڵ�ָ��ά�ȵĴ�С
		dr.Set(m_dim, new_rows);
		m_dim_out = dr;
	}

	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		unsigned int curr_row = 0;
		m_src_indices.resize(xs.size());
		Eigen::DSizes<ptrdiff_t, 5> indices(0, 0, 0, 0, 0);
		Eigen::DSizes<ptrdiff_t, 5> sizes(static_cast<ptrdiff_t>(fx.GetDim()[0])
			, static_cast<ptrdiff_t>(fx.GetDim()[1])
			, static_cast<ptrdiff_t>(fx.GetDim()[2])
			, static_cast<ptrdiff_t>(fx.GetDim()[3])
			, static_cast<ptrdiff_t>(1));
		for (size_t i = 0; i < xs.size(); ++i)
		{
			indices[m_dim] = m_src_indices[i] = curr_row;
			const unsigned int row_size = xs[i]->GetDim()[m_dim];
			sizes[m_dim] = row_size;
			fx.tb<4>().slice(indices, sizes) = xs[i]->tb<4>();
			curr_row += row_size;
		}
	}

	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		Eigen::DSizes<ptrdiff_t, 5> indices(0, 0, 0, 0, 0); indices[m_dim] = m_src_indices[xs_i];
		Eigen::DSizes<ptrdiff_t, 5> sizes(static_cast<ptrdiff_t>(dEdxi.GetDim()[0]),
			static_cast<ptrdiff_t>(dEdxi.GetDim()[1]),
			static_cast<ptrdiff_t>(dEdxi.GetDim()[2]),
			static_cast<ptrdiff_t>(dEdxi.GetDim()[3]),
			static_cast<ptrdiff_t>(1));

		dEdxi.tb<4>() += dEdf.tb<4>().slice(indices, sizes);
	}

private:
	std::vector<int> m_src_indices;
	int m_dim = 0;
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
	CarpRobotExpression AffineTransform(const std::vector<CarpRobotExpression>& inputs) { std::vector<int> args; args.reserve(inputs.size() + 1); args.push_back(m_index); for (auto& input : inputs) args.push_back(input.GetIndex()); return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotAffineTransformNode(args))); }
	CarpRobotExpression Concatenate(const std::vector<CarpRobotExpression>& inputs, int dim=0) { std::vector<int> args; args.reserve(inputs.size() + 1); args.push_back(m_index); for (auto& input : inputs) args.push_back(input.GetIndex()); return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotConcatenateNode(args, dim))); }

public:
	// ��������
	CarpRobotExpression Negate() const { return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotNegateNode({ m_index }))); }
	CarpRobotExpression CwiseMultiply(const CarpRobotExpression& x) const { return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotCwiseMultiplyNode({m_index, x.GetIndex()}))); }
	CarpRobotExpression CwiseSum(const CarpRobotExpression& x) const { return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotCwiseSumNode({ m_index, x.GetIndex() }))); }
	CarpRobotExpression ConstantPlusX(cr_real x) const { return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotConstantPlusXNode({ m_index }, x))); }
	CarpRobotExpression ConstantMinusX(cr_real x) const { return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotConstantMinusXNode({ m_index }, x))); }
	CarpRobotExpression CwiseQuotient(const CarpRobotExpression& x) const { return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotCwiseQuotientNode({ m_index, x.GetIndex() }))); }
	CarpRobotExpression MatrixMultiply(const CarpRobotExpression& x) const { return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotMatrixMultiplyNode({ m_index, x.GetIndex() }))); }
	CarpRobotExpression ConstScalarMultiply(cr_real x) const { return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotConstScalarMultiplyNode({ m_index }, x))); }

	// ��ʧ����
	CarpRobotExpression Square() const { std::vector<int> args; args.push_back(m_index); return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotSquareNode(args))); }
	CarpRobotExpression PickNegLogSoftmax(const int* plabel) const { std::vector<int> args; args.push_back(m_index); return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotPickNegLogSoftmaxNode(args, plabel))); }
	CarpRobotExpression PickNegLogSoftmax(int label) const { std::vector<int> args; args.push_back(m_index); return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotPickNegLogSoftmaxNode(args, label))); }
	CarpRobotExpression BinaryLogLoss() const { std::vector<int> args; args.push_back(m_index); return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotBinaryLogLossNode(args))); }

	// �����
	CarpRobotExpression Tanh() const { std::vector<int> args; args.push_back(m_index); return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotTanhNode(args))); }
	CarpRobotExpression Sigmoid() const { std::vector<int> args; args.push_back(m_index); return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotLogisticSigmoidNode(args))); }
	CarpRobotExpression Rectify() const { std::vector<int> args; args.push_back(m_index); return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotRectifyNode(args))); }
	CarpRobotExpression Softmax() const { std::vector<int> args; args.push_back(m_index); return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotSoftmaxNode(args))); }
	CarpRobotExpression LogSoftmax() const { std::vector<int> args; args.push_back(m_index); return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotLogSoftmaxNode(args))); }

	// ���ܺ���
	CarpRobotExpression Dropout(cr_real rate, bool training=false) const { std::vector<int> args; args.push_back(m_index); return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotDropoutNode(args, rate, training ? 1 : 0))); }
	CarpRobotExpression Dropout(cr_real rate, const int* ptraining) const { std::vector<int> args; args.push_back(m_index); return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotDropoutNode(args, rate, ptraining))); }
	CarpRobotExpression Conv2D(const CarpRobotExpression& kernel, int stride_width = 1, int stride_height = 1, bool padding_type = true) const { std::vector<int> args; args.push_back(m_index); args.push_back(kernel.GetIndex()); return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotConv2DNode(args, stride_width, stride_height, padding_type))); }
	CarpRobotExpression MaxPooling2D(int kernel_width, int kernel_height, int stride_width = 1, int stride_height = 1, bool padding_type = true) const { std::vector<int> args; args.push_back(m_index); return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotMaxPooling2DNode(args, kernel_width, kernel_height, stride_width, stride_height, padding_type))); }
	CarpRobotExpression Reshape(const CarpRobotDim& dim) const { std::vector<int> args; args.push_back(m_index); return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotReshapeNode(args, dim))); }
	CarpRobotExpression PickElement(int value, int dim = 0) const { std::vector<int> args; args.push_back(m_index); return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotPickElementNode(args, value, dim))); }
	CarpRobotExpression PickElement(const int* value, int dim = 0) const { std::vector<int> args; args.push_back(m_index); return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotPickElementNode(args, value, dim))); }
	CarpRobotExpression MeanElements(int dim) const { std::vector<int> args; args.push_back(m_index); return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotMeanElementsNode(args, dim))); }
	CarpRobotExpression StdElements() const { std::vector<int> args; args.push_back(m_index); return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotStdElementsNode(args))); }
	CarpRobotExpression PickRange(int start, int end, int dim=0) const { std::vector<int> args; args.push_back(m_index); return CarpRobotExpression(m_graph, m_graph->AddNode(new CarpRobotPickRangeNode(args, start, end, dim))); }
	CarpRobotExpression LayerNorm(const CarpRobotExpression& g, const CarpRobotExpression& b) const
	{
		// Expression mu = mean_elems(x);
		auto mu = MeanElements(1);
		// Expression x_centered = x - mu;
		auto x_centered = CwiseSum(mu.Negate());
		// Expression sigma = std_elems(x);
		auto sigma = StdElements();
		// cmult(g, cdiv(x_centered, sigma + (real)1e-8)) + b;
		return g.CwiseMultiply(x_centered.CwiseQuotient(sigma.ConstantPlusX((cr_real)1e-8))).CwiseSum(b);
	}

private:
	ICarpRobotComputationGraph* m_graph = nullptr;
	int m_index = 0;
};

// ����ʽ����������
CarpRobotExpression operator-(const CarpRobotExpression& x) { return x.Negate(); }
CarpRobotExpression operator+(const CarpRobotExpression& x, const CarpRobotExpression& y) { return x.CwiseSum(y); }
CarpRobotExpression operator+(cr_real x, const CarpRobotExpression& y) { return y.ConstantPlusX(x); }
CarpRobotExpression operator+(const CarpRobotExpression& x, cr_real y) { return y + x; }
CarpRobotExpression operator-(const CarpRobotExpression& x, const CarpRobotExpression& y) { return x + (-y); }
CarpRobotExpression operator-(cr_real x, const CarpRobotExpression& y) { return y.ConstantMinusX(x); }
CarpRobotExpression operator-(const CarpRobotExpression& x, cr_real y) { return -(y - x); }
CarpRobotExpression operator*(const CarpRobotExpression& x, const CarpRobotExpression& y) { return x.MatrixMultiply(y); }
CarpRobotExpression operator*(const CarpRobotExpression& x, cr_real y) { return x.ConstScalarMultiply(y); }
CarpRobotExpression operator/(const CarpRobotExpression& x, const CarpRobotExpression& y) { return x.CwiseQuotient(y); }
CarpRobotExpression operator/(const CarpRobotExpression& x, cr_real y) { return x * (1.f / y); }

class CarpRobotLabel
{
public:
	void Update(int label) { m_label = label; }

	const int* GetLabel() const { return &m_label; }

private:
	int m_label = 0;
};

// 1. ����ͼ���������нڵ�����˽ṹ
// 2. ǰ����㣬���򴫲�
class CarpRobotComputationGraph : public ICarpRobotComputationGraph
{
public:
	// ���캯������ʼ��
	CarpRobotComputationGraph() { }
	~CarpRobotComputationGraph() { Clear(); }

private:
	CarpRobotComputationGraph(const CarpRobotComputationGraph&) {}
	CarpRobotComputationGraph& operator=(const CarpRobotComputationGraph&) {}

public:
	// ����һ����������ڵ�
	CarpRobotExpression AddInput(cr_real scalar) { return CarpRobotExpression(this, AddNode(new CarpRobotScalarInputNode(scalar))); }
	CarpRobotExpression AddInput(const cr_real* pscalar) { return CarpRobotExpression(this, AddNode(new CarpRobotScalarInputNode(pscalar))); }

	// ����ָ���ṹ������
	CarpRobotExpression AddInput(const CarpRobotDim& dim, const std::vector<cr_real>& data) { return CarpRobotExpression(this, AddNode(new CarpRobotInputNode(dim, data))); }
	CarpRobotExpression AddInput(const CarpRobotDim& dim, const std::vector<cr_real>* pdata) { return CarpRobotExpression(this, AddNode(new CarpRobotInputNode(dim, pdata))); }

public:
	// ����һ��Parameter�����ڵ�
	CarpRobotExpression AddParameters(CarpRobotParameter* p) { return CarpRobotExpression(this, AddParameterNode(new CarpRobotParameterNode(p))); }
	// ����һ��Parameter���������ڵ�
	CarpRobotExpression AddConstParameters(CarpRobotParameter* p) { return CarpRobotExpression(this, AddNode(new CarpRobotConstParameterNode(p))); }
	
public:
	// ��ʼ��
	CarpRobotExpression RandomBernoulli(const CarpRobotDim& d, cr_real p, cr_real scale) { std::vector<int> args; return CarpRobotExpression(this, AddNode(new CarpRobotRandomBernoulliNode(args, d, p, scale))); }

public:
	// ��������
	int Negate(int index) { return (-CarpRobotExpression(this, index)).GetIndex(); }
	int Addition(int index1, int index2) { return (CarpRobotExpression(this, index1) + CarpRobotExpression(this, index2)).GetIndex(); }
	int Plus(int index, cr_real value) { return (CarpRobotExpression(this, index) + value).GetIndex(); }
	int Subtraction(int index1, int index2) { return (CarpRobotExpression(this, index1) - CarpRobotExpression(this, index2)).GetIndex(); }
	int Minus(int index, cr_real value) { return (CarpRobotExpression(this, index) - value).GetIndex(); }
	int Multiplication(int index1, int index2) { return (CarpRobotExpression(this, index1) * CarpRobotExpression(this, index2)).GetIndex(); }
	int Multiply(int index, cr_real value) { return (CarpRobotExpression(this, index) * value).GetIndex(); }
	int Division(int index1, int index2) { return (CarpRobotExpression(this, index1) / CarpRobotExpression(this, index2)).GetIndex(); }
	int Divide(int index, cr_real value) { return (CarpRobotExpression(this, index) / value).GetIndex(); }

	// ��ʧ����
	int Square(int index) { return CarpRobotExpression(this, index).Square().GetIndex(); }
	int PickNegLogSoftmax(int index, CarpRobotLabel* v) { return CarpRobotExpression(this, index).PickNegLogSoftmax(v->GetLabel()).GetIndex(); }
	int BinaryLogLoss(int index) { return CarpRobotExpression(this, index).BinaryLogLoss().GetIndex(); }

	// �����
	int Sigmoid(int index) { return CarpRobotExpression(this, index).Sigmoid().GetIndex(); }
	int Rectify(int index) { return CarpRobotExpression(this, index).Rectify().GetIndex(); }
	int Softmax(int index) { return CarpRobotExpression(this, index).Softmax().GetIndex(); }
	int LogSoftmax(int index) { return CarpRobotExpression(this, index).LogSoftmax().GetIndex(); }

	// ���ܺ���
	int Dropout(int index, cr_real rate, CarpRobotLabel* v) { return CarpRobotExpression(this, index).Dropout(rate, v->GetLabel()).GetIndex(); }
	int Conv2D(int index, int kernel_expr, int stride_width = 1, int stride_height = 1, bool padding_type = true) { return CarpRobotExpression(this, index).Conv2D(CarpRobotExpression(this, kernel_expr), stride_width, stride_height, padding_type).GetIndex(); }
	int MaxPooling2D(int index, int kernel_width, int kernel_height, int stride_width = 1, int stride_height = 1, bool padding_type = true) { return CarpRobotExpression(this, index).MaxPooling2D(kernel_width, kernel_height, stride_width, stride_height, padding_type).GetIndex(); }
	int Reshape(int index, int dim_0, int dim_1, int dim_2)
	{
		std::vector<int> dims;
		if (dim_0 > 0)
		{
			dims.push_back(dim_0);
			if (dim_1 > 0) 
			{
				dims.push_back(dim_1);
				if (dim_2 > 0) dims.push_back(dim_2);
			}
		}
		return CarpRobotExpression(this, index).Reshape(CarpRobotDim(dims)).GetIndex();
	}
	int PickElement(int index, CarpRobotLabel* v, int dim = 0) { return CarpRobotExpression(this, index).PickElement(v->GetLabel(), dim).GetIndex(); }
	int MeanElements(int index, int dim) { return CarpRobotExpression(this, index).MeanElements(dim).GetIndex(); }
	int Concatenate(int index, int expr_0, int expr_1, int expr_2)
	{
		std::vector<CarpRobotExpression> expr_list;
		if (expr_0 > 0)
		{
			expr_list.push_back(CarpRobotExpression(this, expr_0));
			if (expr_1 > 0)
			{
				expr_list.push_back(CarpRobotExpression(this, expr_1));
				if (expr_2 > 0)
					expr_list.push_back(CarpRobotExpression(this, expr_2));
			}
		}
		return CarpRobotExpression(this, index).Concatenate(expr_list).GetIndex();
	}

public:
	// ����һ������ڵ�
	int AddNode(CarpRobotNode* node) override
	{
		// ���ӵ��ڵ��б�
		m_nodes.push_back(node);
		// ��ʼ��ά�ȣ����������ڴ�
		m_fx_list.emplace_back(CarpRobotTensor());

		// ��������ڵ�ά��
		std::vector<const CarpRobotDim*> xs(node->GetArgs().size());
		for (size_t i = 0; i < node->GetArgs().size(); ++i)
			xs[i] = &(m_nodes[node->GetArgs()[i]]->GetDim());
		node->Dim(xs);
		m_fx_list.back().SetDim(node->GetDim());

		// �����Ҫ������㣬��ô������ǰ����
		if (m_immediate_compute) Forward((int)m_nodes.size());
		// ��������
		return (int)m_nodes.size() - 1;
	}

	// ����һ�������ڵ�
	int AddParameterNode(ICarpRobotParameterNode * node)
	{
		// ���ӵ�ͼ
		int index = AddNode(node);
		// ��ǲ����ڵ�
		m_parameter_map[index] = node;
		// ��������
		return index;
	}

public:
	const CarpRobotTensor& GetValue(int i) override
	{
		Forward(i);
		CARP_ROBOT_ASSERT(i < (int)m_fx_list.size(), u8"����Խ��");
		return m_fx_list[i];
	}
	const CarpRobotTensor& GetGradient(int i) const  override
	{
		CARP_ROBOT_ASSERT(i < (int)m_dEdf_list.size(), u8"����Խ��");
		return m_dEdf_list[i];
	}

	cr_real AsScalar(int i)
	{
		return GetValue(i).AsScalar();
	}

	int AsVectorAndArgmax(int i)
	{
		return GetValue(i).AsVectorAndArgmax();
	}

	cr_real AsVectorAndMaxValue(int i)
	{
		return GetValue(i).AsVectorAndMaxValue();
	}

	cr_real AsVectorAndGetValue(int i, int index)
	{
		return GetValue(i).AsVectorAndGetValue(index);
	}

	const CarpRobotDim* GetDim(int i) const
	{
		CARP_ROBOT_ASSERT(i < (int)m_fx_list.size(), u8"����Խ��");
		return &m_fx_list[i].GetDim();
	}

public:
	// ����������Ϣ
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

	// ���˽ṹ���䣬��ͷ��ʼ��
	void Invalidate() { m_evaluated_index = 0; }

	// �����Ƿ�ֱ�Ӽ���ڵ�
	void SetImmediateCompute(bool immediate) { m_immediate_compute = immediate; }

public:
	// ��ǰ����
	const CarpRobotTensor& Forward(int index)
	{
		CARP_ROBOT_ASSERT(m_nodes.size() > 0, u8"��ǰû�нڵ㣬�޷���ǰ����");

		// ���������棬����Ƶ������
		std::vector<const CarpRobotTensor*> xs;

		// �ӵ�ǰ�ڵ���ǰ���㣬ֱ�����
		while (m_evaluated_index < (int)m_nodes.size())
		{
			if (m_evaluated_index > index) break;

			// ��ȡ��ǰ����ڵ�
			auto* node = m_nodes[m_evaluated_index];

			// �����������飬���һ�ȡ�������
			const auto& args = node->GetArgs();
			xs.resize(args.size());
			for (size_t i = 0; i < args.size(); ++i)
				xs[i] = &m_fx_list[args[i]];

			// ִ�нڵ��ǰ�����
			node->Forward(xs, m_fx_list[m_evaluated_index]);

			// ��ǰ��һ��
			++m_evaluated_index;
		}

		return m_fx_list[m_evaluated_index - 1];
	}

	// ���򴫲�
	void Backward()
	{
		// ִ����ǰ���㣬��֤���нڵ�ȫ���������
		Forward((int)m_nodes.size());

		// ��ȡ��ǰ�ڵ����
		int num_nodes = (int)m_nodes.size();

		// ����������С
		m_dEdf_list.resize(num_nodes);

		// ��ʼ��ά����Ϣ���ڴ棬Ȼ������
		for (size_t i = 0; i < m_dEdf_list.size(); ++i)
			m_dEdf_list[i].SetDim(m_fx_list[i].GetDim(), true);
		m_dEdf_list[num_nodes - 1].Constant(1);

		// �����ڵ���Ҫ���򴫲�
		std::vector<bool> needs_derivative(num_nodes, false);
		for (auto& pair : m_parameter_map)
			needs_derivative[pair.first] = true;

		// ��ӵ�в����ڵ�Ľڵ㣬����Ϊ��Ҫ���򴫲�
		for (int i = 0; i < num_nodes; ++i)
		{
			// �������ڵ��Ѿ�����λ��Ҫ�󵼣���ô��ֱ������
			bool need = needs_derivative[i];
			if (need) continue;

			// �������ڵ��ĳ���ڵ���Ҫ�󵼣���ô���ڵ�Ҳ��Ҫ��
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

			// ����Ϊ�Ƿ���Ҫ��
			needs_derivative[i] = need;
		}

		// ֻ���ǲ������Ľڵ㣬Ĭ�ϲ��������
		std::vector<bool> in_computation(num_nodes, false);
		in_computation[num_nodes - 1] = true;	// ������һ���ڵ���Ҫ��������

		// ��������ڵ�
		std::vector<const CarpRobotTensor*> xs;
		for (int i = num_nodes - 1; i >= 0; --i)
		{
			// �������ڵ㲻������㣬ֱ������
			if (!in_computation[i]) continue;

			// ��ȡ�ڵ�
			auto* node = m_nodes[i];

			// ��ȡ�ýڵ������
			const auto& args = node->GetArgs();
			xs.resize(args.size());
			for (size_t j = 0; j < args.size(); ++j)
			{
				const int arg = args[j];

				// �������ڵ�Ϊ�������Ľڵ�
				in_computation[arg] = true;

				// ��������ڵ�
				xs[j] = &m_fx_list[arg];
			}

			// ������������ڵ㣬��Ҫ�󵼵����붼Ҫ���з��򴫲�
			for (size_t j = 0; j < args.size(); ++j)
			{
				const int arg = args[j];
				if (needs_derivative[arg])
					node->Backward(xs, m_fx_list[i], m_dEdf_list[i], (unsigned int)j, m_dEdf_list[arg]);
			}
		}

		// �����Ľ�����õ��ڵ���ȥ
		for (auto& pair : m_parameter_map)
			pair.second->AccumulateGrad(m_dEdf_list[pair.first]);
	}

private:
	std::vector<CarpRobotNode*> m_nodes;			// ���ڴ�ż���ͼ�����˽ṹ
	std::map<int, ICarpRobotParameterNode*> m_parameter_map;	// ��¼�����ڵ������

private:
	std::vector<CarpRobotTensor> m_fx_list;			// ����ǰ�����Ľ��
	std::vector<CarpRobotTensor> m_dEdf_list;		// ���淴�����Ľ��
	int m_evaluated_index = 0;				// ��¼��ǰ���㵽�Ǹ��ڵ�
	bool m_immediate_compute = false;			// �Ƿ���������

private:
	std::string m_string;
};

class CarpRobotTrainer
{
public:
	CarpRobotTrainer(CarpRobotParameterCollection* model, cr_real learning_rate)
		: m_model(model)
		, m_learning_rate(learning_rate)
	{}
	virtual ~CarpRobotTrainer() {}

public:
	// ����ѵ�������򣬽���ͼӦ�õ�������
	virtual void Update()
	{
		// ��ȡparameters�б�
		const std::vector<CarpRobotParameter*>& parameters = m_model->GetParameters();
		// ����parameters�б�
		for (size_t i = 0; i < parameters.size(); ++i)
		{
			// ����gscale�����������
			UpdateParameter((int)i, parameters[i]);
			// ���ݶ�ֵ����
			parameters[i]->Clear();
		}

		++m_updates;
	}

	// ����ѵ�����Ļ�������
	virtual void Restart()
	{
		m_updates = 0;
	}

protected:
	cr_real m_learning_rate = 0.1f;	// ѧϰ��
	int m_updates = 0;
	CarpRobotParameterCollection* m_model = nullptr;		// ParameterCollection����

protected:
	// ���಻��������
	CarpRobotTrainer() {}

	// ���²���
	virtual void UpdateParameter(int index, CarpRobotParameter* parameter) = 0;
};

class CarpRobotSGDTrainer : public CarpRobotTrainer
{
public:
	CarpRobotSGDTrainer(CarpRobotParameterCollection* model, cr_real learning_rate) : CarpRobotTrainer(model, learning_rate) {}
protected:
	void UpdateParameter(int index, CarpRobotParameter* parameter) override { parameter->GetValue().tvec() -= parameter->GetGradient().tvec() * m_learning_rate; }
};


class CarpRobotAdamTrainer : public CarpRobotTrainer
{
public:
	CarpRobotAdamTrainer(CarpRobotParameterCollection* model, float learning_rate = 0.001, float beta_1 = 0.9, float beta_2 = 0.999, float eps = 1e-8) :
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