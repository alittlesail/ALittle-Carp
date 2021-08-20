#ifndef CARP_ROBOT_INCLUDED
#define CARP_ROBOT_INCLUDED

#include <string>
#include <fstream>
#include <sstream>
#include <vector>

// �����쳣��
#define CARP_ROBOT_ASSERT(e, text) \
do { \
	if (!(e)) { std::ostringstream oss; oss << __FUNCTION__ << "() " << text; throw std::runtime_error(oss.str()); } \
} while (0)

// ���帡��������
typedef float cr_real;

// ��������ά������
const int CARP_ROBOT_MAX_TENSOR_DIM = 7;

// ���л�
class CarpRobotModelSerializer
{
public:
	~CarpRobotModelSerializer() { Close(); }

public:
	bool Open(const std::string& model_path) { m_out_file.open(model_path, std::ios::binary | std::ios::trunc); return m_out_file.is_open(); }
	void Close() { if (m_out_file.is_open()) m_out_file.close(); }

public:
	void WriteString(const std::string& str)
	{
		CARP_ROBOT_ASSERT(m_out_file.is_open(), u8"WriteString Failed: out_file open failed");
		int length = (int)str.size();
		m_out_file.write((char*)&length, sizeof(length));
		m_out_file.write(str.c_str(), length);
	}
	void WriteReal(cr_real value)
	{
		CARP_ROBOT_ASSERT(m_out_file.is_open(), u8"WriteReal Failed: out_file open failed");
		m_out_file.write((char*)&value, sizeof(cr_real));
	}
	void WriteRealArray(cr_real* value, size_t count)
	{
		CARP_ROBOT_ASSERT(m_out_file.is_open(), u8"WriteRealArray Failed: out_file open failed");
		m_out_file.write((char*)value, sizeof(cr_real) * count);
	}
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
	bool Open(const std::string& model_path) { m_in_file.open(model_path, std::ios::binary); return m_in_file.is_open(); }
	void Close() { if (m_in_file.is_open()) m_in_file.close(); }

public:
	const char* ReadString()
	{
		CARP_ROBOT_ASSERT(m_in_file.is_open(), "ReadString Failed: in_file open failed");

		int length = 0;
		m_in_file.read((char*)&length, sizeof(int));
		CARP_ROBOT_ASSERT(m_in_file.gcount() == sizeof(int), u8"ReadString Failed: length error 1");

		m_string.resize(length + 1);
		m_string[length] = 0;
		m_in_file.read(m_string.data(), length);
		CARP_ROBOT_ASSERT(m_in_file.gcount() == length, u8"ReadString Failed: length error 2");

		return m_string.data();
	}
	cr_real ReadReal()
	{
		CARP_ROBOT_ASSERT(m_in_file.is_open(), u8"ReadReal Failed: in_file open failed");

		cr_real value = 0;
		m_in_file.read((char*)&value, sizeof(cr_real));
		CARP_ROBOT_ASSERT(m_in_file.gcount() == sizeof(cr_real), u8"ReadReal Failed: length error 1");

		return value;
	}
	void ReadRealArray(cr_real* value, size_t count)
	{
		CARP_ROBOT_ASSERT(m_in_file.is_open(), u8"ReadRealArray Failed: in_file open failed");

		m_in_file.read((char*)value, sizeof(cr_real) * count);
		CARP_ROBOT_ASSERT(m_in_file.gcount() == sizeof(cr_real) * count, u8"ReadRealArray Failed: length error 1");
	}
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

// �����洢���ʽ������ά�������Ϣ
class CarpRobotDim
{
public:
	// ���캯������ʼ��
	CarpRobotDim() : nd(0) { memset(d, 0, sizeof(d)); }
	CarpRobotDim(unsigned int d0) : nd(1) { memset(d, 0, sizeof(d)); d[0] = d0; }
	CarpRobotDim(unsigned int d0, unsigned int d1) : nd(2) { memset(d, 0, sizeof(d)); d[0] = d0; d[1] = d1; }
	CarpRobotDim(unsigned int d0, unsigned int d1, unsigned int d2) : nd(3) { memset(d, 0, sizeof(d)); d[0] = d0; d[1] = d1; d[2] = d2; }
	CarpRobotDim(unsigned int d0, unsigned int d1, unsigned int d2, unsigned int d3) : nd(4) { memset(d, 0, sizeof(d)); d[0] = d0; d[1] = d1; d[2] = d2; d[3] = d3; }
	CarpRobotDim(unsigned int d0, unsigned int d1, unsigned int d2, unsigned int d3, unsigned int d4) : nd(5) { memset(d, 0, sizeof(d)); d[0] = d0; d[1] = d1; d[2] = d2; d[3] = d3; d[4] = d4; }

public:
	// ��ȡ�ܴ�С
	int GetTotalSize() const { int p = 1; for (unsigned int i = 0; i < nd; ++i) p *= d[i]; return p; }

public:
	// ����==����
	bool operator == (const CarpRobotDim& right) const { if (nd != right.nd) return false; return std::memcmp(d, right.d, nd * sizeof(unsigned int)) == 0; }
	// ����!=����
	bool operator != (const CarpRobotDim& right) const { return !((*this) == right); }

	// ��ǿ�����
	bool SoftEqual(const CarpRobotDim& right) const
	{
		auto max_nd = std::max(nd, right.nd);
		for (unsigned int i = 0; i < max_nd; ++i)
		{
			if (d[i] == right.d[i]) continue;
			if (d[i] == 0 && right.d[i] == 1) continue;
			if (d[i] == 1 && right.d[i] == 0) continue;

			return false;
		}

		return true;
	}

public:
	// ����һ��Dim�����ض�β��Ϊ1��ά��
	CarpRobotDim Truncate() const { CarpRobotDim r = *this; unsigned int m = nd; while (m > 1 && d[m - 1] == 1) --m; r.Resize(m); return r; }
	// �޸�ά�ȵ����������ndС��i����ô������Ĳ���ȫ������Ϊ1
	void Resize(unsigned int i) { while (nd < i) d[nd++] = 1; nd = i; }
	// ȷ�ϵ�ǰ�Ƕ�άʱ����ȡ��һ��ά�ȵĴ�С
	unsigned int Rows() const { return d[0]; }
	// ȷ�ϵ�ǰ�Ƕ�άʱ����ȡ�ڶ���ά�ȵĴ�С�����û�оͷ���1
	unsigned int Cols() const { return nd > 1 ? d[1] : 1; }
	// ָ��ĳ��ά�ȵĴ�С
	void Set(unsigned int i, unsigned int s) { CARP_ROBOT_ASSERT(i < nd || s == 1, u8"Out of bounds exception in Dim::set(" << i << "," << s << ") for node of size " << nd); CARP_ROBOT_ASSERT(s != 0, u8"Attempt to set dimension size to zero in Dim::set(" << i << "," << s << ") for node of size " << nd); d[i] = s; }
	// ���ø���ά�ȴ�С
	void Sets(const std::vector<long>& dims) { CARP_ROBOT_ASSERT(dims.size() <= CARP_ROBOT_MAX_TENSOR_DIM, u8"Out of bounds"); nd = (unsigned int)dims.size(); for (unsigned int i = 0; i < dims.size(); ++i) d[i] = dims[i]; }

	// ��ȡ������ά�ȴ�С
	int GetND() { return nd; }
	void SetND(int n) { nd = n; }

	void Serialize(CarpRobotModelSerializer& file)
	{
		file.WriteInt(nd);
		for (unsigned int i = 0; i < nd; ++i)
			file.WriteInt(d[i]);
	}
	void Deserialize(CarpRobotModelDeserializer& file)
	{
		nd = file.ReadInt();
		for (unsigned int i = 0; i < nd; ++i)
			d[i] = file.ReadInt();
	}

	std::string ToString() const
	{
		std::string result = "{";
		for (unsigned int i = 0; i < nd; ++i) {
			if (i) result += ",";
			result += std::to_string(d[i]);
		}
		result += "}";
		return result;
	}

public:
	// �������ά�ȵ����飬������2��3��4����ôd����ĳ���Ϊ3����0��Ԫ����2����2��Ԫ����3��������Ԫ����4
	unsigned int d[CARP_ROBOT_MAX_TENSOR_DIM] = { 0 };
	// d����ĳ���
	unsigned int nd = 0;
};

#include <Eigen/Eigen/Eigen>
#include <Eigen/unsupported/Eigen/CXX11/Tensor>

class CarpRobotTensor
{
public:
	CarpRobotTensor() : v(nullptr), v_shared(false) { }
	CarpRobotTensor(const CarpRobotDim& d) : v(nullptr), v_shared(false) { SetDim(d, true); }
	CarpRobotTensor(const CarpRobotTensor& t) : v(nullptr), v_shared(false) { Copy(t); }
	~CarpRobotTensor() { ReleaseMemory(); }
	void operator = (const CarpRobotTensor& t) { Copy(t); }

private:
	void ReleaseMemory()
	{
		if (v && v_shared == false)
			free(v);
		v = 0;
		v_shared = false;
	}

public:
	// ����������Tensor�ڴ�
	void RefrenceTensor(const CarpRobotTensor& t)
	{
		if (&t == this) return;

		ReleaseMemory();
		d = t.d;
		v = t.v;
		v_shared = true;
	}
	void RefrenceMemory(const CarpRobotDim& dd, cr_real* vv)
	{
		ReleaseMemory();

		d = dd;
		v = vv;
		v_shared = true;
	}

	// ����
	void Copy(const CarpRobotTensor& t)
	{
		if (&t == this) return;

		ReleaseMemory();
		d = t.d;

		int total_bytes = d.GetTotalSize() * sizeof(cr_real);
		if (total_bytes > 0 && t.v)
		{
			v = (cr_real*)malloc(total_bytes);
			memcpy(v, t.v, total_bytes);
		}
	}

	// ����ά����Ϣ��
	void SetDim(const CarpRobotDim& dim, bool init_zero = false)
	{
		if (d == dim)
		{
			if (init_zero && d.GetTotalSize() > 0)
				Zero();
			return;
		}

		ReleaseMemory();

		d = dim;

		int real_count = d.GetTotalSize();
		if (real_count <= 0) return;

		v = (cr_real*)malloc(real_count * sizeof(cr_real));
		if (init_zero) Zero();
	}

public:
	std::string ToString() const
	{
		std::string result = d.ToString() + "\n";
		int total_size = d.GetTotalSize();
		if (d.nd == 2)
		{
			for (unsigned int row = 0; row < d.d[0]; ++row)
			{
				for (unsigned int col = 0; col < d.d[1]; ++col)
				{
					result += std::to_string(v[row * d.d[1] + col]);
					if (col + 1 != d.d[1]) result += ",";
				}
				if (row + 1 != d.d[0]) result += "\n";
			}
		}
		else
		{
			for (int i = 0; i < total_size; ++i)
			{
				result += std::to_string(v[i]);
				if (i + 1 != total_size) result += ",";
			}
		}

		result += "\n";
		return result;
	}

public:
	bool v_shared;	// �Ƿ��ǹ�������Tensor���ڴ�
	cr_real* v;		// �ڴ�
	CarpRobotDim d;

public:
	void Constant(cr_real c) { tvec() = tvec().constant(c); }
	void Zero() { Constant(0); }
	void Clip(cr_real left, cr_real right) { tvec() = tvec().cwiseMax(left).cwiseMin(right); }
	void Scale(cr_real left, cr_real right) { tvec() = tvec() * left + right; }
	void Identity()
	{
		CARP_ROBOT_ASSERT(d.nd == 2 && d.d[0] == d.d[1], u8"�����Ƿ��ξ���");

		size_t pos = 0;
		for (size_t i = 0; i < d.d[0]; ++i)
		{
			for (size_t j = 0; j < d.d[1]; ++j)
				v[pos++] = (i == j ? 1.0f : 0.0f);
		}
	}
	void RandomizeBernoulli(std::mt19937& gen, cr_real p, cr_real scale=1.0f)
	{
		std::bernoulli_distribution distribution(p);
		auto b = [&] {return distribution(gen) * scale; };
		std::generate(v, v + d.GetTotalSize(), b);
	}
	void RandomizeNormal(std::mt19937& gen, cr_real mean=0.0f, cr_real stddev= 1.0f)
	{
		std::normal_distribution<cr_real> distribution(mean, stddev);
		auto b = [&] { return distribution(gen); };
		std::generate(v, v + d.GetTotalSize(), b);
	}
	void RandomizeUniform(std::mt19937& gen, cr_real left=0.0f, cr_real right=1.0f)
	{
		std::uniform_real_distribution<cr_real> distribution(left, right);
		auto b = [&] { return distribution(gen); };
		std::generate(v, v + d.GetTotalSize(), b);
	}
	void RandomizeOrthonormal(std::mt19937& gen, cr_real scale=1.0f)
	{
		CARP_ROBOT_ASSERT(d.nd == 2 && d.d[0] == d.d[1], u8"�����Ƿ��ξ���");
		RandomizeUniform(gen , -1.0, 1.0);
		Eigen::JacobiSVD<Eigen::MatrixXf> svd(m(), Eigen::ComputeFullU | Eigen::ComputeThinV);
		m() = scale * svd.matrixU();
	}
	cr_real AccessElement(int index) { return v[index]; }
	cr_real AccessElement(const CarpRobotDim& index) { return m()(index.d[0], index.d[1]); }
	void SetElement(int index, cr_real value) { v[index] = value; }
	void CopyElement(const CarpRobotTensor& from, int from_index, int to_index) { v[to_index] = from.v[from_index]; }
	void SetElements(const std::vector<cr_real>& vec) { memcpy(v, vec.data(), sizeof(cr_real) * vec.size()); }
	void CopyElements(const CarpRobotTensor& from) { memcpy(v, from.v, sizeof(cr_real) * from.d.GetTotalSize()); }
	void Accumulate(const CarpRobotTensor& from)
	{
		CARP_ROBOT_ASSERT(d.GetTotalSize() == from.d.GetTotalSize(), u8"����Tensor��С����һ��");
		tvec() += from.tvec();
	}

public:
	// ��ȡ�����������
	Eigen::Map<Eigen::MatrixXf> m() { return Eigen::Map<Eigen::MatrixXf>(v, d.Rows(), d.Cols()); }
	const Eigen::Map<Eigen::MatrixXf> m() const { return Eigen::Map<Eigen::MatrixXf>(v, d.Rows(), d.Cols()); }

	// ʹ�õ�ǰ���ݴ���һ��������Ȼ�󷵻�
	Eigen::Map<Eigen::VectorXf> vec() { return Eigen::Map<Eigen::VectorXf>(v, d.GetTotalSize()); }
	const Eigen::Map<Eigen::VectorXf> vec() const { return Eigen::Map<Eigen::VectorXf>(v, d.GetTotalSize()); }

	// ʹ�õ�ǰ���ݣ�����һ��ֻ��һά��Tensor
	Eigen::TensorMap<Eigen::Tensor<cr_real, 1>> tvec() { return Eigen::TensorMap<Eigen::Tensor<cr_real, 1>>(v, d.GetTotalSize()); }
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 1>> tvec() const { return Eigen::TensorMap<Eigen::Tensor<cr_real, 1>>(v, d.GetTotalSize()); }

	// ʹ�õ�ǰ���ݣ�����һ��Orderά�ȵ�Tensor����
	template <int Order>
	Eigen::TensorMap<Eigen::Tensor<cr_real, Order>> t() { }
	template <int Order>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, Order>> t() const { }
	template <>
	Eigen::TensorMap<Eigen::Tensor<cr_real, 0>> t<0>()
	{
		CARP_ROBOT_ASSERT(d.GetTotalSize() == 1, u8"�Ƿ�ά����Ϣ t<0>(): dim=" << d.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 0>>(v);
	}
	template <>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 0>> t<0>() const
	{
		CARP_ROBOT_ASSERT(d.GetTotalSize() == 1, u8"�Ƿ�ά����Ϣ t<0>(): dim=" << d.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 0>>(v);
	}
	template <>
	Eigen::TensorMap<Eigen::Tensor<cr_real, 1>> t<1>()
	{
		CARP_ROBOT_ASSERT(d.nd <= 1, u8"�Ƿ�ά����Ϣ t<1>(): dim=" << d.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 1>>(v, (int)d.d[0]);
	}
	template <>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 1>> t<1>() const
	{
		CARP_ROBOT_ASSERT(d.nd <= 1, u8"�Ƿ�ά����Ϣ t<1>(): dim=" << d.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 1>>(v, (int)d.d[0]);
	}
	template <>
	Eigen::TensorMap<Eigen::Tensor<cr_real, 2>> t<2>()
	{
		CARP_ROBOT_ASSERT(d.nd <= 2, u8"�Ƿ�ά����Ϣ t<2>(): dim=" << d.ToString());
		if (d.nd == 2) return Eigen::TensorMap<Eigen::Tensor<cr_real, 2>>(v, (int)d.d[0], (int)d.d[1]);
		else                return Eigen::TensorMap<Eigen::Tensor<cr_real, 2>>(v, (int)d.d[0], (int)1);
	}
	template <>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 2>> t<2>() const
	{
		CARP_ROBOT_ASSERT(d.nd <= 2, u8"�Ƿ�ά����Ϣ t<2>(): dim=" << d.ToString());
		if (d.nd == 2) return Eigen::TensorMap<Eigen::Tensor<cr_real, 2>>(v, (int)d.d[0], (int)d.d[1]);
		else                return Eigen::TensorMap<Eigen::Tensor<cr_real, 2>>(v, (int)d.d[0], (int)1);
	}
	template <>
	Eigen::TensorMap<Eigen::Tensor<cr_real, 3>> t<3>()
	{
		CARP_ROBOT_ASSERT(d.nd <= 3, u8"�Ƿ�ά����Ϣ t<3>(): dim=" << d.ToString());
		if (d.nd == 3) return Eigen::TensorMap<Eigen::Tensor<cr_real, 3>>(v, (int)d.d[0], (int)d.d[1], (int)d.d[2]);
		else if (d.nd == 2) return Eigen::TensorMap<Eigen::Tensor<cr_real, 3>>(v, (int)d.d[0], (int)d.d[1], (int)1);
		else                return Eigen::TensorMap<Eigen::Tensor<cr_real, 3>>(v, (int)d.d[0], (int)1, (int)1);
	}
	template <>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 3>> t<3>() const
	{
		CARP_ROBOT_ASSERT(d.nd <= 3, u8"�Ƿ�ά����Ϣ t<3>(): dim=" << d.ToString());
		if (d.nd == 3) return Eigen::TensorMap<Eigen::Tensor<cr_real, 3>>(v, (int)d.d[0], (int)d.d[1], (int)d.d[2]);
		else if (d.nd == 2) return Eigen::TensorMap<Eigen::Tensor<cr_real, 3>>(v, (int)d.d[0], (int)d.d[1], (int)1);
		else                return Eigen::TensorMap<Eigen::Tensor<cr_real, 3>>(v, (int)d.d[0], (int)1, (int)1);
	}
	template <>
	Eigen::TensorMap<Eigen::Tensor<cr_real, 4>> t<4>()
	{
		CARP_ROBOT_ASSERT(d.nd <= 4, u8"�Ƿ�ά����Ϣ t<4>(): dim=" << d.ToString());
		if (d.nd == 4)      return Eigen::TensorMap<Eigen::Tensor<cr_real, 4>>(v, (int)d.d[0], (int)d.d[1], (int)d.d[2], (int)d.d[3]);
		else if (d.nd == 3) return Eigen::TensorMap<Eigen::Tensor<cr_real, 4>>(v, (int)d.d[0], (int)d.d[1], (int)d.d[2], (int)1);
		else if (d.nd == 2) return Eigen::TensorMap<Eigen::Tensor<cr_real, 4>>(v, (int)d.d[0], (int)d.d[1], (int)1, (int)1);
		else                return Eigen::TensorMap<Eigen::Tensor<cr_real, 4>>(v, (int)d.d[0], (int)1, (int)1, (int)1);
	}
	template <>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 4>> t<4>() const
	{
		CARP_ROBOT_ASSERT(d.nd <= 4, u8"�Ƿ�ά����Ϣ t<4>(): dim=" << d.ToString());
		if (d.nd == 4)      return Eigen::TensorMap<Eigen::Tensor<cr_real, 4>>(v, (int)d.d[0], (int)d.d[1], (int)d.d[2], (int)d.d[3]);
		else if (d.nd == 3) return Eigen::TensorMap<Eigen::Tensor<cr_real, 4>>(v, (int)d.d[0], (int)d.d[1], (int)d.d[2], (int)1);
		else if (d.nd == 2) return Eigen::TensorMap<Eigen::Tensor<cr_real, 4>>(v, (int)d.d[0], (int)d.d[1], (int)1, (int)1);
		else                return Eigen::TensorMap<Eigen::Tensor<cr_real, 4>>(v, (int)d.d[0], (int)1, (int)1, (int)1);
	}

public:
	// ������������
	cr_real AsScalar() const { CARP_ROBOT_ASSERT(d.GetTotalSize() == 1, u8"��ǰTensor���Ǳ���"); return v[0]; }
	// ������������
	void AsVector(std::vector<cr_real>& out) const { out.resize(d.GetTotalSize()); memcpy(&out[0], v, sizeof(cr_real) * out.size()); }

public:
	void Serialize(CarpRobotModelSerializer& file) { d.Serialize(file); file.WriteRealArray(v, d.GetTotalSize()); }
	void Deserialize(CarpRobotModelDeserializer& file) { CarpRobotDim tmp; tmp.Deserialize(file); SetDim(tmp); file.ReadRealArray(v, d.GetTotalSize()); }
};

class CarpRobotParameterBase
{
public:
	CarpRobotParameterBase() {}
	virtual ~CarpRobotParameterBase() {}

public:
	// ��ȡ������
	virtual const std::string& GetFullName() const = 0;

	// ��ȡ������Ȩ��
	virtual cr_real CurrentWeightDecay() const = 0;

	// ���Ų���
	virtual void ScaleParameters(cr_real a) = 0;

	// ���������
	virtual void ScaleGradient(cr_real a) = 0;

	// ��������
	virtual void Zero() = 0;

	// �õ�������ƽ��l2����
	virtual void SquaredL2Norm(cr_real* sqnorm) const = 0;

	// �õ�������ƽ��l2����
	virtual void GSquaredL2Norm(cr_real* sqnorm) const = 0;

	// �����Ƿ��Ѿ�������
	virtual bool IsUpdated() const = 0;

	// �ж�������ǲ���0
	virtual bool HasGrad() const = 0;
};

// L2������
class CarpRobotL2WeightDecay
{
public:
	CarpRobotL2WeightDecay(cr_real lambda = 1e-6) : m_weight_decay(1) { SetLambda(lambda); }

public:
	void SetLambda(cr_real lam)
	{
		CARP_ROBOT_ASSERT(lam >= 0, u8"lam ����С��0");
		m_lambda = lam;
	}

public:
	void UpdateWeightDecay(unsigned int num_updates = 1)
	{
		if (num_updates == 0) return;
		if (num_updates == 1)
			m_weight_decay -= m_weight_decay * m_lambda;
		else
			m_weight_decay = (cr_real)(m_weight_decay * std::pow(1 - m_lambda, num_updates));
	}

	cr_real CurrentWeightDecay() const { return m_weight_decay; }

	bool ParametersNeedRescaled() const { return m_weight_decay < 0.25f; }

	void ResetWeightDecay()
	{
		m_weight_decay = 1.0f;
	}

private:
	cr_real m_weight_decay;
	cr_real m_lambda;
};

class CarpRobotParameterCollectionBase
{
public:
	virtual CarpRobotL2WeightDecay& GetWeightDecay() = 0;
};

class CarpRobotParameter : public CarpRobotParameterBase
{
public:
	CarpRobotParameter(const CarpRobotDim& d, const std::string& name)
		: m_name(name)
		, m_dim(d)
	{
		// ���ò�����ά����Ϣ
		m_values.SetDim(d, true);
		m_grad.SetDim(d, true);
		m_grad.Zero();
	}
	virtual ~CarpRobotParameter() {}

public:
	// ����һ��Parameter
	void Copy(const CarpRobotParameter& param)
	{
		// �ж����ݽṹ�Ƿ�һ�£���һ�����޷�����
		CARP_ROBOT_ASSERT(m_dim == param.m_dim, u8"ά�Ȳ�һ��: " << m_dim.ToString() << " != " << param.m_dim.ToString());

		// ��������
		m_values.CopyElements(param.m_values);
	}

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
		if (m_grad.v) m_grad.Zero();
	}

	// ���в�����ֵ��[left,right]��Χ��
	void Clip(cr_real left, cr_real right)
	{
		m_values.Clip(left, right);
	}

	// ����������Ȩ�زü�
	void ClipInplace(cr_real left, cr_real right)
	{
		cr_real my_scale = 1.0f / CurrentWeightDecay();
		Clip(left * my_scale, right * my_scale);
	}

	// ���ò���ֵ
	void SetValue(const std::vector<cr_real>& val)
	{
		m_values.SetElements(val);
	}

	// ��ȡ����
	CarpRobotTensor& GetValue() { return m_values; }

public:
	// ��ȡ������
	virtual const std::string& GetFullName() const override { return m_name; }

	// ��ȡ������Ȩ��
	virtual cr_real CurrentWeightDecay() const override { return m_owner->GetWeightDecay().CurrentWeightDecay(); }

	// ���������Ų���
	virtual void ScaleParameters(cr_real a) override { m_values.tvec() = m_values.tvec() * a; }

	// ���������������
	virtual void ScaleGradient(cr_real a) override { m_grad.tvec() = m_grad.tvec() * a; }

	// �������Ͳ�������Ϊ0
	virtual void Zero() override { m_values.Zero(); Clear(); }

	// ��ȡ������ƽ��L2����
	// L2������ʽΪ ��������Ԫ�ص�ƽ���͵�1/2�η�
	virtual void SquaredL2Norm(cr_real* sqnorm) const override
	{
		CarpRobotTensor sqnorm_t; sqnorm_t.RefrenceMemory(CarpRobotDim(1), sqnorm);
		sqnorm_t.t<0>() = m_values.tvec().square().sum();
	}

	// ��ȡ�ݶȵ�ƽ��L2����
	virtual void GSquaredL2Norm(cr_real* sqnorm) const override
	{
		CARP_ROBOT_ASSERT(m_grad.v != 0, u8"m_grad �ڴ��ǿյ�!");
		CarpRobotTensor sqnorm_t; sqnorm_t.RefrenceMemory(CarpRobotDim(1), sqnorm);
		sqnorm_t.Zero();
		sqnorm_t.t<0>() = m_grad.tvec().square().sum();
	}

	// ��ǰ�����Ƿ���¹���
	virtual bool IsUpdated() const override { return m_updated; }
	void SetUpdated(bool b) { m_updated = b; }

	// �Ƿ��������
	virtual bool HasGrad() const override { return m_has_grad; }

public:
	void Serialize(CarpRobotModelSerializer& file, const char* name)
	{
		if (name)
			file.WriteString(name);
		else
			file.WriteString("Parameter");

		m_values.Serialize(file);
	}
	void Deserialize(CarpRobotModelDeserializer& file, const char* name)
	{
		std::string name_copy = "Parameter";
		if (name) name_copy = name;
		std::string read_name = file.ReadString();
		CARP_ROBOT_ASSERT(read_name == name_copy, u8"�����л����ƴ���, ��ǰ��:" << read_name << "Ӧ����:" << name_copy);
		m_values.Deserialize(file);
	}

public:
	std::string m_name;				// ��ǰ���������֣����������õ�
	CarpRobotDim m_dim;						// ��ǰ������ά����Ϣ
	CarpRobotTensor m_values;				// ��ǰ������ֵ
	CarpRobotTensor m_grad;					// ��ǰ�����������
	bool m_updated = true;					// ��ǰ�����Ƿ���¹���
	bool m_has_grad = false;			// �Ƿ��������
	CarpRobotParameterCollectionBase* m_owner = nullptr;	// �����Ĳ����ռ���
};

class CarpRobotParameterCollection : public CarpRobotParameterCollectionBase
{
public:
	CarpRobotParameterCollection() : m_gen(m_rd()) { }
	virtual ~CarpRobotParameterCollection()
	{
		for (unsigned int i = 0; i < m_all_params.size(); ++i)
			delete m_all_params[i];
	}

public:
	virtual CarpRobotParameter* AddParameters(const CarpRobotDim& d, const std::string& name = "")
	{
		auto* p = new CarpRobotParameter(d, name);
		p->m_owner = this;
		p->GetValue().RandomizeUniform(m_gen, -0.1f, 0.1f);
		m_all_params.push_back(p);
		m_params.push_back(p);

		return p;
	}

public:
	// �������в����ݶ�
	void ResetGradient()
	{
		for (unsigned int i = 0; i < m_params.size(); ++i)
			m_params[i]->Clear();
	}

	// ͶӰȨ��
	void ProjectWeights(cr_real radius = 1.0f)
	{
		if (m_project_scratch.size() < m_all_params.size())
			m_project_scratch.resize(m_all_params.size());

		for (unsigned int i = 0; i < m_all_params.size(); ++i)
			m_all_params[i]->SquaredL2Norm(&m_project_scratch[i]);

		double gg = 0;
		for (unsigned int i = 0; i < m_all_params.size(); ++i)
			gg += m_project_scratch[i];
	}


	// ����Ȩ��˥��ϵ��
	void SetWeightDecayLambda(cr_real lambda)
	{
		m_weight_decay.SetLambda(lambda);
	}

	// L2��������
	cr_real GradientL2Norm()
	{
		if (m_gradient_norm_scratch.size() < m_all_params.size() + 1)
			m_gradient_norm_scratch.resize(m_all_params.size() + 1);

		for (unsigned int i = 0; i < m_all_params.size(); ++i)
			m_all_params[i]->GSquaredL2Norm(&(m_gradient_norm_scratch[i]));

		CarpRobotDim dscr(static_cast<int>(m_all_params.size()));
		CarpRobotTensor scratch_t;
		scratch_t.RefrenceMemory(dscr, &(m_gradient_norm_scratch[0]));

		CarpRobotDim dsum(1);
		CarpRobotTensor sum_t;
		sum_t.RefrenceMemory(dsum, &(m_gradient_norm_scratch[m_all_params.size()]));

		sum_t.t<0>() = scratch_t.t<1>().sum().sqrt();
		return m_gradient_norm_scratch[m_all_params.size()];
	}

	// ��ȡ����˥������
	CarpRobotL2WeightDecay& GetWeightDecay() override { return m_weight_decay; }

	// ���������������
	void ScaleGradient(cr_real a)
	{
		for (unsigned int i = 0; i < m_all_params.size(); ++i)
			m_all_params[i]->ScaleGradient(a);
	}

public:
	// ��ȡ�����ռ�������
	std::string GetFullName() const { return m_name; }

public:
	std::vector<CarpRobotParameterBase*> m_all_params;			// ���в����ļ��ϣ�����Parameter �� lookupParameter��
	std::vector<CarpRobotParameter*> m_params;					// Parameter�����ļ���
	
private:
	CarpRobotL2WeightDecay m_weight_decay;						// Ȩ��˥��

	std::string m_name;									// �ռ�������
	std::random_device m_rd;
	std::mt19937 m_gen;

private:
	std::vector<cr_real> m_project_scratch;
	std::vector<cr_real> m_gradient_norm_scratch;
};

// ����ڵ�
class CarpRobotNode
{
public:
	CarpRobotNode() {};
	CarpRobotNode(const std::vector<int>& args) : m_args(args) {}
	virtual ~CarpRobotNode() {}

public:
	// ִ����ǰ���㣬��Batch���д���Ȼ�����ForwardImpl
	virtual void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) = 0;
	// ִ�з�����㣬��Batch���д���Ȼ�����BackwardImpl
	virtual void Backward(const std::vector<const CarpRobotTensor*>& xs,	// �ýڵ������ֵ
		const CarpRobotTensor& fx,											// �ýڵ�����ֵ
		const CarpRobotTensor& dEdf,										// �ýڵ������ڵ�������
		unsigned int xs_i,													// �ýڵ������ڵ�����
		CarpRobotTensor& dEdxi) = 0;									// ����ڵ�������
	
public:
	// ��������ڵ���±�
	std::vector<int> m_args;
};

// ��������ڵ�
class CarpRobotInputNode : public CarpRobotNode
{
public:
	// ֵ����
	CarpRobotInputNode(const CarpRobotDim& d, const std::vector<cr_real>& dat) : m_dim(d), m_data(dat), m_pdata(&m_data) {}
	// ��ַ����
	CarpRobotInputNode(const CarpRobotDim& d, std::vector<cr_real>* pdat) : m_dim(d), m_data(), m_pdata(pdat) {}

protected:
	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 0, "Failed dimension");
		// ֱ�������ڴ�
		fx.RefrenceMemory(m_dim, m_pdata->data());
	}
	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		unsigned int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		// ����ڵ㣬���ܷ��򴫲�
		CARP_ROBOT_ASSERT(0, "called backward() on arity 0 node: i = " << xs_i);
	}

public:
	CarpRobotDim m_dim;
	std::vector<cr_real> m_data;
	std::vector<cr_real>* m_pdata;
};

// ��������ڵ�
class CarpRobotScalarInputNode : public CarpRobotNode
{
public:
	// ֵ����
	CarpRobotScalarInputNode(cr_real s) : m_data(s), m_pdata(&m_data), m_dim(1) {}
	// ��ַ����
	CarpRobotScalarInputNode(cr_real* ps) : m_data(0), m_pdata(ps), m_dim(1) {}

protected:
	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 0, "Failed dimension");
		// ֱ�������ڴ�
		fx.RefrenceMemory(m_dim, m_pdata);
	}
	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		unsigned int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		// ����ڵ㣬���ܷ��򴫲�
		CARP_ROBOT_ASSERT(0, "called backward() on arity 0 node: i = " << xs_i);
	}

public:
	CarpRobotDim m_dim;
	cr_real m_data = 0;
	cr_real* m_pdata = nullptr;
};

// �����ڵ�����
class CarpRobotParameterNodeBase : public CarpRobotNode
{
public:
	virtual ~CarpRobotParameterNodeBase() {}
	virtual void AccumulateGrad(const CarpRobotTensor& g) {}
};

// ��ͨ��������ڵ�
class CarpRobotParameterNode : public CarpRobotParameterNodeBase
{
public:
	CarpRobotParameterNode(CarpRobotParameter* p) : m_dim(p->m_dim), m_params(p) {}

public:
	// ִ�з��򴫵��ݶ�
	void AccumulateGrad(const CarpRobotTensor& g) override { m_params->AccumulateGrad(g); }

protected:
	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		fx.SetDim(m_dim);
		fx.tvec() = m_params->m_values.tvec() * m_params->CurrentWeightDecay();
	}
	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		unsigned int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		CARP_ROBOT_ASSERT(0, "called backward() on arity 0 node: i = " << xs_i);
	}

public:
	CarpRobotDim m_dim;
	CarpRobotParameter* m_params = nullptr;
};

// ������������ڵ�
class CarpRobotConstParameterNode : public CarpRobotNode
{
public:
	CarpRobotConstParameterNode(CarpRobotParameter* p) : m_dim(p->m_dim), m_params(p) { }

protected:
	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		fx.SetDim(m_dim);
		fx.tvec() = m_params->m_values.tvec() * m_params->CurrentWeightDecay();
	}
	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		unsigned int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		CARP_ROBOT_ASSERT(0, "called backward() on arity 0 node: i = " << xs_i);
	}

public:
	CarpRobotDim m_dim;
	CarpRobotParameter* m_params = nullptr;
};

// һ����������
// ������ y = -x_1
// �󵼺��� dx_1 = -1
class CarpRobotNegateNode : public CarpRobotNode
{
public:
	CarpRobotNegateNode(const std::vector<int>& a) : CarpRobotNode(a) {}

protected:
	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"CarpRobotNegateNode ������һ������");
		// v��ÿ��Ԫ��ȡ����
		fx.SetDim(xs[0]->d);
		fx.tvec() = -xs[0]->tvec();
	}
	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		unsigned int xs_i,
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
	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 2, u8"CarpRobotCwiseSumNode ��������������");
		CARP_ROBOT_ASSERT(xs[0]->d.SoftEqual(xs[1]->d), u8"CarpRobotCwiseSumNode ����������ά�ȱ���һ��, xs[0]:" << xs[0]->d.ToString() << " != xs[1]:" << xs[1]->d.ToString());

		fx.SetDim(xs[0]->d);
		fx.tvec() = xs[0]->tvec() + xs[1]->tvec();
	}
	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		unsigned int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		// ����Ҫ��ôдdEdxi.tvec() += dEdf.tvec() * 1; ����Ϊ�˼���һ���˷����㣬����д����������
		dEdxi.tvec() += dEdf.tvec();
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

	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"CarpRobotConstantPlusXNode ������һ������");
		
		fx.SetDim(xs[0]->d);
		// unaryExpr��ʾ��xs[0]->tvec()����Ԫ�ؽ��б�����ÿ��Ԫ�ض�����ConstAddOp����
		fx.tvec() = xs[0]->tvec().unaryExpr(ConstAddOp(m_value));
	}

	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		unsigned int xs_i,
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
	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"CarpRobotConstantMinusXNode ������һ������");

		fx.SetDim(xs[0]->d);
		// unaryExpr��ʾ��xs[0]->tvec()����Ԫ�ؽ��б�����ÿ��Ԫ�ض�����ConstAddOp����
		fx.tvec() = xs[0]->tvec().unaryExpr(ConstMinusOp(m_value));
	}
	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		unsigned int xs_i,
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
	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 2, "CarpRobotMatrixMultiplyNode ��������������");
		CARP_ROBOT_ASSERT(xs[0]->d.Cols() == xs[1]->d.Rows(), u8"CarpRobotMatrixMultiplyNode ǰ����б�����ں������");
		CARP_ROBOT_ASSERT(xs[0]->d.nd <= 2 && xs[1]->d.nd <= 2, "CarpRobotMatrixMultiplyNode ������������2ά��");

		CarpRobotDim dim(xs[0]->d.Rows(), xs[1]->d.Cols());
		fx.SetDim(dim);
		fx.m().noalias() = xs[0]->m() * xs[1]->m();
	}
	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		unsigned int xs_i,
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
	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 1, "CarpRobotConstScalarMultiplyNode ������һ������");
		fx.SetDim(xs[0]->d);
		fx.tvec() = xs[0]->tvec() * m_value;
	}
	void Backward(const std::vector<const CarpRobotTensor*>& xs,
		const CarpRobotTensor& fx,
		const CarpRobotTensor& dEdf,
		unsigned int xs_i,
		CarpRobotTensor& dEdxi) override
	{
		CARP_ROBOT_ASSERT(xs_i == 0, "CarpRobotConstScalarMultiplyNode ������һ������");
		dEdxi.tvec() += dEdf.tvec() * m_value;
	}

private:
	cr_real m_value;
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
	void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) override
	{
		CARP_ROBOT_ASSERT(xs.size() == 2, u8"CarpRobotCwiseQuotientNode ��������������");
		CARP_ROBOT_ASSERT(xs[0]->d.SoftEqual(xs[1]->d), u8"CarpRobotCwiseQuotientNode ����������ά�ȱ���һ��, xs[0]:" << xs[0]->d.ToString() << " != xs[1]:" << xs[1]->d.ToString());

		fx.SetDim(xs[0]->d);

		// ��Ϊ�������������ά��һ��
		// ����Ϊtģ�庯��֧����ߵ�4��ά��
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
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"LogisticSigmoidNode ������һ������");
		fx.SetDim(xs[0]->d);
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
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"CarpRobotSquareNode ������һ������");
		fx.SetDim(xs[0]->d);
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
		CARP_ROBOT_ASSERT(xs.size() == 2, u8"CarpRobotBinaryLogLossNode ��������������");
		CARP_ROBOT_ASSERT(xs[0]->d.Rows() == 2 || xs[0]->d.nd == 1, u8"����ά����Ϣ����");
		CARP_ROBOT_ASSERT(xs[1]->d.Rows() == 2 || xs[1]->d.nd == 1, u8"����ά����Ϣ����");

		fx.SetDim(CarpRobotDim(1));
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

class CarpRobotComputationGraphBase
{
public:
	virtual ~CarpRobotComputationGraphBase() {}
	virtual const CarpRobotTensor& GetValue(int i) = 0;
	virtual const CarpRobotTensor& GetGradient(int i) const = 0;
	virtual int AddNode(CarpRobotNode* node) = 0;
};

class CarpRobotExpression
{
public:
	CarpRobotExpression() {}
	CarpRobotExpression(CarpRobotComputationGraphBase* pg, int ii) : pg(pg), i(ii) {}

public:
	const CarpRobotTensor& GetValue() const { return pg->GetValue(i); }
	const CarpRobotTensor& GetGradient() const { return pg->GetGradient(i); }
	CarpRobotExpression Square() { std::vector<int> args; args.push_back(i); return CarpRobotExpression(pg, pg->AddNode(new CarpRobotSquareNode(args))); }
	CarpRobotExpression Sigmoid() { std::vector<int> args; args.push_back(i); return CarpRobotExpression(pg, pg->AddNode(new CarpRobotLogisticSigmoidNode(args))); }

public:
	CarpRobotComputationGraphBase* pg = nullptr;
	int i = 0;
};

// 1. ����ͼ���������нڵ�����˽ṹ
// 2. ǰ����㣬���򴫲�
class CarpRobotComputationGraph : public CarpRobotComputationGraphBase
{
public:
	// ���캯������ʼ��
	CarpRobotComputationGraph() { }
	~CarpRobotComputationGraph() { Clear(); }

public:
	// ���һ����������ڵ�
	CarpRobotExpression AddInput(cr_real s) { return CarpRobotExpression(this, AddNode(new CarpRobotScalarInputNode(s))); }
	CarpRobotExpression AddInput(cr_real* ps) { return CarpRobotExpression(this, AddNode(new CarpRobotScalarInputNode(ps))); }

	// ���ָ���ṹ������
	CarpRobotExpression AddInput(const CarpRobotDim& d, const std::vector<cr_real>& data) { return CarpRobotExpression(this, AddNode(new CarpRobotInputNode(d, data))); }
	CarpRobotExpression AddInput(const CarpRobotDim& d, std::vector<cr_real>* pdata) { return CarpRobotExpression(this, AddNode(new CarpRobotInputNode(d, pdata))); }

public:
	// ���һ��Parameter�����ڵ�
	CarpRobotExpression AddParameters(CarpRobotParameter* p) { return CarpRobotExpression(this, AddParameterNode(new CarpRobotParameterNode(p))); }
	// ���һ��Parameter���������ڵ�
	CarpRobotExpression AddConstParameters(CarpRobotParameter* p) { return CarpRobotExpression(this, AddNode(new CarpRobotConstParameterNode(p))); }

public:
	// ���һ������ڵ�
	int AddNode(CarpRobotNode* node) override
	{
		// ��ӵ��ڵ��б�
		m_nodes.push_back(node);
		// ��ʼ��ά�ȣ����������ڴ�
		m_nfxs.push_back(CarpRobotTensor());

		// �����Ҫ������㣬��ô������ǰ����
		if (m_immediate_compute) Forward();
		// ��������
		return (int)m_nodes.size() - 1;
	}

	// ���һ�������ڵ�
	int AddParameterNode(CarpRobotParameterNodeBase* node)
	{
		// ��ӵ�ͼ
		int index = AddNode(node);
		// ��ǲ����ڵ�
		m_parameter_nodes[index] = node;
		// ��������
		return index;
	}

public:
	const CarpRobotTensor& GetValue(int i) override
	{
		Forward();
		CARP_ROBOT_ASSERT(i < (int)m_nfxs.size(), u8"����Խ��");
		return m_nfxs[i];
	}
	const CarpRobotTensor& GetGradient(int i) const  override
	{
		CARP_ROBOT_ASSERT(i < (int)m_ndEdfs.size(), u8"����Խ��");
		return m_ndEdfs[i];
	}

public:
	// ����������Ϣ
	void Clear()
	{
		for (unsigned int i = 0; i < m_nodes.size(); ++i)
			delete m_nodes[i];
		m_nodes.resize(0);
		m_parameter_nodes.clear();
		m_nfxs.resize(0);
		m_ndEdfs.resize(0);
		m_evaluated_index = 0;
	}

	// ���˽ṹ���䣬��ͷ��ʼ��
	void Invalidate()
	{
		m_evaluated_index = 0;
	}

	// �����Ƿ�ֱ�Ӽ���ڵ�
	void SetImmediateCompute(bool immediate) { m_immediate_compute = immediate; }

public:
	// ��ǰ����
	const CarpRobotTensor& Forward()
	{
		CARP_ROBOT_ASSERT(m_nodes.size() > 0, u8"��ǰû�нڵ㣬�޷���ǰ����");

		// ���������棬����Ƶ������
		std::vector<const CarpRobotTensor*> xs;

		// �ӵ�ǰ�ڵ���ǰ���㣬ֱ�����
		while (m_evaluated_index < (int)m_nodes.size())
		{
			// ��ȡ��ǰ����ڵ�
			auto* node = m_nodes[m_evaluated_index];

			// �����������飬���һ�ȡ�������
			xs.resize(node->m_args.size());
			for (unsigned int i = 0; i < node->m_args.size(); ++i)
				xs[i] = &m_nfxs[node->m_args[i]];

			// ִ�нڵ��ǰ�����
			node->Forward(xs, m_nfxs[m_evaluated_index]);

			// ��ǰ��һ��
			++m_evaluated_index;
		}

		return m_nfxs[m_evaluated_index - 1];
	}

	int GetResultCount()
	{
		return (int)m_nfxs.size();
	}
	cr_real GetResultAsScalar(int i)
	{
		Forward();

		if (i >= (int)m_nfxs.size())
		{
			return 0;
		}

		return m_nfxs[i].AsScalar();
	}

	// ���򴫲�
	void Backward()
	{
		// ִ����ǰ���㣬��֤���нڵ�ȫ���������
		Forward();

		// ��ȡ��ǰ�ڵ����
		int num_nodes = (int)m_nodes.size();

		// ����������С
		m_ndEdfs.resize(num_nodes);

		// ��ʼ��ά����Ϣ���ڴ棬Ȼ������
		for (unsigned int i = 0; i < m_ndEdfs.size(); ++i)
			m_ndEdfs[i].SetDim(m_nfxs[i].d, true);
		m_ndEdfs[num_nodes - 1].Constant(1);

		// �����ڵ���Ҫ���򴫲�
		std::vector<bool> needs_derivative(num_nodes, false);
		for (auto& pair : m_parameter_nodes)
			needs_derivative[pair.first] = true;

		// ��ӵ�в����ڵ�Ľڵ㣬����Ϊ��Ҫ���򴫲�
		for (int i = 0; i < num_nodes; ++i)
		{
			bool need = needs_derivative[i];
			if (need) continue;

			// �������ڵ��ĳ���ڵ���Ҫ�󵼣���ô���ڵ�Ҳ��Ҫ��
			const CarpRobotNode* node = m_nodes[i];
			for (unsigned int j = 0; j < node->m_args.size(); ++j)
			{
				int arg = node->m_args[j];
				if (needs_derivative[arg])
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
			xs.resize(node->m_args.size());
			for (unsigned int j = 0; j < node->m_args.size(); ++j)
			{
				int arg = node->m_args[j];

				// �������ڵ�Ϊ�������Ľڵ�
				in_computation[arg] = true;

				// ��������ڵ�
				xs[j] = &m_nfxs[arg];
			}

			// ������������ڵ㣬��Ҫ�󵼵����붼Ҫ���з��򴫲�
			for (unsigned int j = 0; j < node->m_args.size(); ++j)
			{
				int arg = node->m_args[j];
				if (needs_derivative[arg])
					node->Backward(xs, m_nfxs[i], m_ndEdfs[i], j, m_ndEdfs[arg]);
			}
		}

		// �����Ľ�����õ��ڵ���ȥ
		for (auto& pair : m_parameter_nodes)
			pair.second->AccumulateGrad(m_ndEdfs[pair.first]);
	}

	// ��ȡ�ݶ�����
	int GetGradCount() { return (int)m_ndEdfs.size(); }
	// ��ȡĳ���ݶ�ֵ
	cr_real GetGradAsScalar(int i) { if (i >= (int)m_ndEdfs.size()) return 0; return m_ndEdfs[i].AsScalar(); }

private:
	std::vector<CarpRobotNode*> m_nodes;			// ���ڴ�ż���ͼ�����˽ṹ
	std::map<int, CarpRobotParameterNodeBase*> m_parameter_nodes;	// ��¼�����ڵ������

private:
	std::vector<CarpRobotTensor> m_nfxs;			// ����ǰ�����Ľ��
	std::vector<CarpRobotTensor> m_ndEdfs;		// ���淴�����Ľ��
	int m_evaluated_index = 0;				// ��¼��ǰ���㵽�Ǹ��ڵ�
	bool m_immediate_compute = false;			// �Ƿ���������
};

CarpRobotExpression operator-(const CarpRobotExpression& x) { return CarpRobotExpression(x.pg, x.pg->AddNode( new CarpRobotNegateNode({ x.i }))); }
CarpRobotExpression operator+(const CarpRobotExpression& x, const CarpRobotExpression& y) { return CarpRobotExpression(x.pg, x.pg->AddNode(new CarpRobotCwiseSumNode({ x.i, y.i }))); }
CarpRobotExpression operator+(cr_real x, const CarpRobotExpression& y) { return CarpRobotExpression(y.pg, y.pg->AddNode(new CarpRobotConstantPlusXNode({ y.i }, x))); }
CarpRobotExpression operator+(const CarpRobotExpression& x, cr_real y) { return y + x; }
CarpRobotExpression operator-(const CarpRobotExpression& x, const CarpRobotExpression& y) { return x + (-y); }
CarpRobotExpression operator-(cr_real x, const CarpRobotExpression& y) { return CarpRobotExpression(y.pg, y.pg->AddNode(new CarpRobotConstantMinusXNode({ y.i }, x))); }
CarpRobotExpression operator-(const CarpRobotExpression& x, cr_real y) { return -(y - x); }
CarpRobotExpression operator*(const CarpRobotExpression& x, const CarpRobotExpression& y) { return CarpRobotExpression(x.pg, x.pg->AddNode(new CarpRobotMatrixMultiplyNode({ x.i, y.i }))); }
CarpRobotExpression operator*(const CarpRobotExpression& x, cr_real y) { return CarpRobotExpression(x.pg, x.pg->AddNode(new CarpRobotConstScalarMultiplyNode({ x.i }, y))); }
CarpRobotExpression operator/(const CarpRobotExpression& x, const CarpRobotExpression& y) { return CarpRobotExpression(x.pg, x.pg->AddNode(new CarpRobotCwiseQuotientNode({ x.i, y.i }))); }

class CarpRobotTrainer
{
public:
	/**
	 *	Trainer���캯��
	 *	@param m : ��Ҫѵ����ParameterCollection����
	 *	@param learning_rate : ��ʼ����ѧϰ��
	**/
	CarpRobotTrainer(CarpRobotParameterCollection* m, cr_real learning_rate) :
		m_learning_rate(learning_rate),			// ѧϰ��
		m_clipping_enabled(true),				// �Ƿ����ݶȲü�
		m_clip_threshold(5),					// �ü���ֵ
		m_clips(0),								// �ü�����
		m_updates(0),							// ���´���
		m_model(m)
	{}
	virtual ~CarpRobotTrainer() {}

public:
	/**
	 *	�����ʵ��Ĺ������parameters
	**/
	virtual void Update()
	{
		// ��ȡ���еı���
		const cr_real gscale = ClipGradients();
		// ��ȡparameters�б�
		const std::vector<CarpRobotParameter*>& params = m_model->m_params;
		// ����parameters�б�
		for (size_t i = 0; i < params.size(); ++i)
		{
			// �Ƿ�Ӧ�ø����������
			if (params[i]->m_updated)
			{
				// ����gscale�����������
				UpdateParams(gscale, i);
				// ���ݶ�ֵ����
				params[i]->Clear();
			}
		}

		++m_updates;

		// ��ȡ������
		auto& wd = m_model->GetWeightDecay();
		// ����Ȩ��
		wd.UpdateWeightDecay();
		if (wd.ParametersNeedRescaled())
			RescaleAndResetWeightDecay();
	}

	/**
	 *	����optimizer
	 *	�������momentum �� assimilate
	**/
	virtual void Restart() = 0;

	/**
	 *	����optimizer
	 *	�������momentum �� assimilate ���������� ѧϰ��

	 *	@param lr : ѧϰ��
	**/
	void SetLearningRate(cr_real lr) { m_learning_rate = lr; }

	/**
	 *	�ݶȲ���
	 *	����������в����ݶȹ��󣬲������ʵ��Ĳ��б�������

	 *	@return : �ʵ��Ĳ��б�������
	**/
	cr_real ClipGradients()
	{
		// ����gscale����ʼ��Ϊ1
		cr_real g_scale = 1;

		// �������ü�
		if (m_clipping_enabled)
		{
			// ��ȡ����L2����������ݶ�
			cr_real gg = m_model->GradientL2Norm();

			// gg��������󣬲���Ϊ����
			// isnan : ���Ը������Ƿ��Ƿ�����
			// isinf : ���Ը������Ƿ����޴�
			if ((std::isnan(gg)) || (std::isinf(gg)))
			{
				// CARP_ERROR(u8"Magnitude of gradient is bad:" << gg);
			}
			else
			{
				// ���gg>m_clip_threshold
				if (gg > m_clip_threshold)
				{
					++m_clips;
					g_scale = m_clip_threshold / gg;
				}
			}
		}

		return g_scale;
	}

	/**
	 *	TODO : ������ʱ���ܱ�����ֱ���ܸ��õ����л�Ȩ��˥���ڱ���ģ���ʱ��
	 *	�����model���µ�������
	**/
	void RescaleAndResetWeightDecay()
	{
		// ��ȡ�����
		const cr_real weitht_decay = m_model->GetWeightDecay().CurrentWeightDecay();

		// �������ݵ�modelÿ��parameters �� lookup_parameters ����
		for (unsigned int i = 0; i < m_model->m_params.size(); ++i)
		{
			auto* p = m_model->m_params[i];
			if (p->IsUpdated())
				p->ScaleParameters(weitht_decay);
		}

		// ���������
		m_model->GetWeightDecay().ResetWeightDecay();
	}

public:
	cr_real m_learning_rate = 0.1f;	// ѧϰ��

	/* �ݶȲü�(�����ݶȱ�ըʱ��) */
	bool m_clipping_enabled = false;			// �Ƿ������ݶȲü�
	cr_real m_clip_threshold = 0;				// �ü��ٽ�ֵ
	cr_real m_clips = 0;
	cr_real m_updates = 0;

	CarpRobotParameterCollection* m_model = nullptr;		// ParameterCollection����

protected:
	CarpRobotTrainer() {}
	/**
	*	���ø��²�����ʵ�ʹ���
	*
	*	@param scale �� ���µı���(i.e. ѧϰ��)
	*	@param gscale : ���ڲ��е��ݶȱ���
	*	@param values : �����ر�ָ����ֵ
	*/
	virtual void UpdateRule(cr_real gscale, const std::vector<CarpRobotTensor*>& values) = 0;
	/**
	*	parameter���º���
	*
	*	@param scale : ���±��� (i.e. learning rate)
	*	@param gscale : ���ڲ��е��ݶȱ���
	*	@param idx : ��Ҫ���µ�parameter����
	*/
	virtual void UpdateParams(cr_real gscale, size_t idx) = 0;
};

class CarpRobotSimpleSGDTrainer : public CarpRobotTrainer
{

public:
	/**
	 *	@param m : ��Ҫѵ����ParameterCollection����
	 *	@param learning_rate : ��ʼ����ѧϰ��
	**/
	CarpRobotSimpleSGDTrainer(CarpRobotParameterCollection* m, cr_real learning_rate = 0.1) : CarpRobotTrainer(m, learning_rate) {}
	~CarpRobotSimpleSGDTrainer() {}

	void Restart() {}

	void Close() {}

protected:
	/**
	*	parameter���º���
	*
	*	@param scale : ���±��� (i.e. learning rate)
	*	@param gscale : ���ڲ��е��ݶȱ���
	*	@param idx : ��Ҫ���µ�parameter����
	*/
	void UpdateParams(cr_real g_scale, size_t idx) override
	{
		auto* p = m_model->m_params[idx];
		std::vector<CarpRobotTensor*> values;
		values.push_back(&(p->m_values));
		values.push_back(&(p->m_grad));
		UpdateRule(g_scale, values);
	}

	/**
	*	���ø��²�����ʵ�ʹ���
	*
	*	@param scale �� ���µı���(i.e. ѧϰ��)
	*	@param gscale : ���ڲ��е��ݶȱ���
	*	@param values : �����ر�ָ����ֵ
	*/
	void UpdateRule(cr_real g_scale, const std::vector<CarpRobotTensor*>& values) override
	{
		values[0]->tvec() -= values[1]->tvec() * (m_learning_rate * g_scale / m_model->GetWeightDecay().CurrentWeightDecay());
	}
};

#endif