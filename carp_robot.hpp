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

// 定义最大的维度数量
const int CARP_ROBOT_MAX_TENSOR_DIM = 7;

// 序列化
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

// 反序列化
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

// 用来存储表达式的数据维度相关信息
class CarpRobotDim
{
public:
	// 构造函数，初始化
	CarpRobotDim() : nd(0) { memset(d, 0, sizeof(d)); }
	CarpRobotDim(unsigned int d0) : nd(1) { memset(d, 0, sizeof(d)); d[0] = d0; }
	CarpRobotDim(unsigned int d0, unsigned int d1) : nd(2) { memset(d, 0, sizeof(d)); d[0] = d0; d[1] = d1; }
	CarpRobotDim(unsigned int d0, unsigned int d1, unsigned int d2) : nd(3) { memset(d, 0, sizeof(d)); d[0] = d0; d[1] = d1; d[2] = d2; }
	CarpRobotDim(unsigned int d0, unsigned int d1, unsigned int d2, unsigned int d3) : nd(4) { memset(d, 0, sizeof(d)); d[0] = d0; d[1] = d1; d[2] = d2; d[3] = d3; }
	CarpRobotDim(unsigned int d0, unsigned int d1, unsigned int d2, unsigned int d3, unsigned int d4) : nd(5) { memset(d, 0, sizeof(d)); d[0] = d0; d[1] = d1; d[2] = d2; d[3] = d3; d[4] = d4; }

public:
	// 获取总大小
	int GetTotalSize() const { int p = 1; for (unsigned int i = 0; i < nd; ++i) p *= d[i]; return p; }

public:
	// 重载==符号
	bool operator == (const CarpRobotDim& right) const { if (nd != right.nd) return false; return std::memcmp(d, right.d, nd * sizeof(unsigned int)) == 0; }
	// 重载!=符号
	bool operator != (const CarpRobotDim& right) const { return !((*this) == right); }

	// 非强制相等
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
	// 复制一个Dim，并截断尾部为1的维度
	CarpRobotDim Truncate() const { CarpRobotDim r = *this; unsigned int m = nd; while (m > 1 && d[m - 1] == 1) --m; r.Resize(m); return r; }
	// 修改维度的数量，如果nd小于i，那么多出来的部分全部设置为1
	void Resize(unsigned int i) { while (nd < i) d[nd++] = 1; nd = i; }
	// 确认当前是二维时，获取第一个维度的大小
	unsigned int Rows() const { return d[0]; }
	// 确认当前是二维时，获取第二个维度的大小，如果没有就返回1
	unsigned int Cols() const { return nd > 1 ? d[1] : 1; }
	// 指定某个维度的大小
	void Set(unsigned int i, unsigned int s) { CARP_ROBOT_ASSERT(i < nd || s == 1, u8"Out of bounds exception in Dim::set(" << i << "," << s << ") for node of size " << nd); CARP_ROBOT_ASSERT(s != 0, u8"Attempt to set dimension size to zero in Dim::set(" << i << "," << s << ") for node of size " << nd); d[i] = s; }
	// 设置各个维度大小
	void Sets(const std::vector<long>& dims) { CARP_ROBOT_ASSERT(dims.size() <= CARP_ROBOT_MAX_TENSOR_DIM, u8"Out of bounds"); nd = (unsigned int)dims.size(); for (unsigned int i = 0; i < dims.size(); ++i) d[i] = dims[i]; }

	// 获取和设置维度大小
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
	// 用来存放维度的数组，比如是2×3×4，那么d数组的长度为3，第0个元素是2，第2个元素是3，第三个元素是4
	unsigned int d[CARP_ROBOT_MAX_TENSOR_DIM] = { 0 };
	// d数组的长度
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
	// 引用其他的Tensor内存
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

	// 复制
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

	// 设置维度信息，
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
	bool v_shared;	// 是否是共用其他Tensor的内存
	cr_real* v;		// 内存
	CarpRobotDim d;

public:
	void Constant(cr_real c) { tvec() = tvec().constant(c); }
	void Zero() { Constant(0); }
	void Clip(cr_real left, cr_real right) { tvec() = tvec().cwiseMax(left).cwiseMin(right); }
	void Scale(cr_real left, cr_real right) { tvec() = tvec() * left + right; }
	void Identity()
	{
		CARP_ROBOT_ASSERT(d.nd == 2 && d.d[0] == d.d[1], u8"必须是方形矩阵");

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
		CARP_ROBOT_ASSERT(d.nd == 2 && d.d[0] == d.d[1], u8"必须是方形矩阵");
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
		CARP_ROBOT_ASSERT(d.GetTotalSize() == from.d.GetTotalSize(), u8"两个Tensor大小必须一致");
		tvec() += from.tvec();
	}

public:
	// 获取创建矩阵对象
	Eigen::Map<Eigen::MatrixXf> m() { return Eigen::Map<Eigen::MatrixXf>(v, d.Rows(), d.Cols()); }
	const Eigen::Map<Eigen::MatrixXf> m() const { return Eigen::Map<Eigen::MatrixXf>(v, d.Rows(), d.Cols()); }

	// 使用当前数据创建一个向量，然后返回
	Eigen::Map<Eigen::VectorXf> vec() { return Eigen::Map<Eigen::VectorXf>(v, d.GetTotalSize()); }
	const Eigen::Map<Eigen::VectorXf> vec() const { return Eigen::Map<Eigen::VectorXf>(v, d.GetTotalSize()); }

	// 使用当前数据，创建一个只有一维的Tensor
	Eigen::TensorMap<Eigen::Tensor<cr_real, 1>> tvec() { return Eigen::TensorMap<Eigen::Tensor<cr_real, 1>>(v, d.GetTotalSize()); }
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 1>> tvec() const { return Eigen::TensorMap<Eigen::Tensor<cr_real, 1>>(v, d.GetTotalSize()); }

	// 使用当前数据，创建一个Order维度的Tensor对象
	template <int Order>
	Eigen::TensorMap<Eigen::Tensor<cr_real, Order>> t() { }
	template <int Order>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, Order>> t() const { }
	template <>
	Eigen::TensorMap<Eigen::Tensor<cr_real, 0>> t<0>()
	{
		CARP_ROBOT_ASSERT(d.GetTotalSize() == 1, u8"非法维度信息 t<0>(): dim=" << d.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 0>>(v);
	}
	template <>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 0>> t<0>() const
	{
		CARP_ROBOT_ASSERT(d.GetTotalSize() == 1, u8"非法维度信息 t<0>(): dim=" << d.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 0>>(v);
	}
	template <>
	Eigen::TensorMap<Eigen::Tensor<cr_real, 1>> t<1>()
	{
		CARP_ROBOT_ASSERT(d.nd <= 1, u8"非法维度信息 t<1>(): dim=" << d.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 1>>(v, (int)d.d[0]);
	}
	template <>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 1>> t<1>() const
	{
		CARP_ROBOT_ASSERT(d.nd <= 1, u8"非法维度信息 t<1>(): dim=" << d.ToString());
		return Eigen::TensorMap<Eigen::Tensor<cr_real, 1>>(v, (int)d.d[0]);
	}
	template <>
	Eigen::TensorMap<Eigen::Tensor<cr_real, 2>> t<2>()
	{
		CARP_ROBOT_ASSERT(d.nd <= 2, u8"非法维度信息 t<2>(): dim=" << d.ToString());
		if (d.nd == 2) return Eigen::TensorMap<Eigen::Tensor<cr_real, 2>>(v, (int)d.d[0], (int)d.d[1]);
		else                return Eigen::TensorMap<Eigen::Tensor<cr_real, 2>>(v, (int)d.d[0], (int)1);
	}
	template <>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 2>> t<2>() const
	{
		CARP_ROBOT_ASSERT(d.nd <= 2, u8"非法维度信息 t<2>(): dim=" << d.ToString());
		if (d.nd == 2) return Eigen::TensorMap<Eigen::Tensor<cr_real, 2>>(v, (int)d.d[0], (int)d.d[1]);
		else                return Eigen::TensorMap<Eigen::Tensor<cr_real, 2>>(v, (int)d.d[0], (int)1);
	}
	template <>
	Eigen::TensorMap<Eigen::Tensor<cr_real, 3>> t<3>()
	{
		CARP_ROBOT_ASSERT(d.nd <= 3, u8"非法维度信息 t<3>(): dim=" << d.ToString());
		if (d.nd == 3) return Eigen::TensorMap<Eigen::Tensor<cr_real, 3>>(v, (int)d.d[0], (int)d.d[1], (int)d.d[2]);
		else if (d.nd == 2) return Eigen::TensorMap<Eigen::Tensor<cr_real, 3>>(v, (int)d.d[0], (int)d.d[1], (int)1);
		else                return Eigen::TensorMap<Eigen::Tensor<cr_real, 3>>(v, (int)d.d[0], (int)1, (int)1);
	}
	template <>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 3>> t<3>() const
	{
		CARP_ROBOT_ASSERT(d.nd <= 3, u8"非法维度信息 t<3>(): dim=" << d.ToString());
		if (d.nd == 3) return Eigen::TensorMap<Eigen::Tensor<cr_real, 3>>(v, (int)d.d[0], (int)d.d[1], (int)d.d[2]);
		else if (d.nd == 2) return Eigen::TensorMap<Eigen::Tensor<cr_real, 3>>(v, (int)d.d[0], (int)d.d[1], (int)1);
		else                return Eigen::TensorMap<Eigen::Tensor<cr_real, 3>>(v, (int)d.d[0], (int)1, (int)1);
	}
	template <>
	Eigen::TensorMap<Eigen::Tensor<cr_real, 4>> t<4>()
	{
		CARP_ROBOT_ASSERT(d.nd <= 4, u8"非法维度信息 t<4>(): dim=" << d.ToString());
		if (d.nd == 4)      return Eigen::TensorMap<Eigen::Tensor<cr_real, 4>>(v, (int)d.d[0], (int)d.d[1], (int)d.d[2], (int)d.d[3]);
		else if (d.nd == 3) return Eigen::TensorMap<Eigen::Tensor<cr_real, 4>>(v, (int)d.d[0], (int)d.d[1], (int)d.d[2], (int)1);
		else if (d.nd == 2) return Eigen::TensorMap<Eigen::Tensor<cr_real, 4>>(v, (int)d.d[0], (int)d.d[1], (int)1, (int)1);
		else                return Eigen::TensorMap<Eigen::Tensor<cr_real, 4>>(v, (int)d.d[0], (int)1, (int)1, (int)1);
	}
	template <>
	const Eigen::TensorMap<Eigen::Tensor<cr_real, 4>> t<4>() const
	{
		CARP_ROBOT_ASSERT(d.nd <= 4, u8"非法维度信息 t<4>(): dim=" << d.ToString());
		if (d.nd == 4)      return Eigen::TensorMap<Eigen::Tensor<cr_real, 4>>(v, (int)d.d[0], (int)d.d[1], (int)d.d[2], (int)d.d[3]);
		else if (d.nd == 3) return Eigen::TensorMap<Eigen::Tensor<cr_real, 4>>(v, (int)d.d[0], (int)d.d[1], (int)d.d[2], (int)1);
		else if (d.nd == 2) return Eigen::TensorMap<Eigen::Tensor<cr_real, 4>>(v, (int)d.d[0], (int)d.d[1], (int)1, (int)1);
		else                return Eigen::TensorMap<Eigen::Tensor<cr_real, 4>>(v, (int)d.d[0], (int)1, (int)1, (int)1);
	}

public:
	// 当作标量返回
	cr_real AsScalar() const { CARP_ROBOT_ASSERT(d.GetTotalSize() == 1, u8"当前Tensor不是标量"); return v[0]; }
	// 当作向量返回
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
	// 获取参数名
	virtual const std::string& GetFullName() const = 0;

	// 获取正则项权重
	virtual cr_real CurrentWeightDecay() const = 0;

	// 缩放参数
	virtual void ScaleParameters(cr_real a) = 0;

	// 缩放误差项
	virtual void ScaleGradient(cr_real a) = 0;

	// 参数清零
	virtual void Zero() = 0;

	// 得到参数的平方l2范数
	virtual void SquaredL2Norm(cr_real* sqnorm) const = 0;

	// 得到误差项的平方l2范数
	virtual void GSquaredL2Norm(cr_real* sqnorm) const = 0;

	// 参数是否已经被更新
	virtual bool IsUpdated() const = 0;

	// 判断误差项是不是0
	virtual bool HasGrad() const = 0;
};

// L2正则项
class CarpRobotL2WeightDecay
{
public:
	CarpRobotL2WeightDecay(cr_real lambda = 1e-6) : m_weight_decay(1) { SetLambda(lambda); }

public:
	void SetLambda(cr_real lam)
	{
		CARP_ROBOT_ASSERT(lam >= 0, u8"lam 不能小于0");
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
		// 设置参数的维度信息
		m_values.SetDim(d, true);
		m_grad.SetDim(d, true);
		m_grad.Zero();
	}
	virtual ~CarpRobotParameter() {}

public:
	// 复制一个Parameter
	void Copy(const CarpRobotParameter& param)
	{
		// 判断数据结构是否一致，不一致则无法复制
		CARP_ROBOT_ASSERT(m_dim == param.m_dim, u8"维度不一致: " << m_dim.ToString() << " != " << param.m_dim.ToString());

		// 复制数据
		m_values.CopyElements(param.m_values);
	}

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
		if (m_grad.v) m_grad.Zero();
	}

	// 裁切参数的值在[left,right]范围内
	void Clip(cr_real left, cr_real right)
	{
		m_values.Clip(left, right);
	}

	// 根据正则项权重裁剪
	void ClipInplace(cr_real left, cr_real right)
	{
		cr_real my_scale = 1.0f / CurrentWeightDecay();
		Clip(left * my_scale, right * my_scale);
	}

	// 设置参数值
	void SetValue(const std::vector<cr_real>& val)
	{
		m_values.SetElements(val);
	}

	// 获取张量
	CarpRobotTensor& GetValue() { return m_values; }

public:
	// 获取参数名
	virtual const std::string& GetFullName() const override { return m_name; }

	// 获取正则项权重
	virtual cr_real CurrentWeightDecay() const override { return m_owner->GetWeightDecay().CurrentWeightDecay(); }

	// 按比例缩放参数
	virtual void ScaleParameters(cr_real a) override { m_values.tvec() = m_values.tvec() * a; }

	// 按比例缩放误差项
	virtual void ScaleGradient(cr_real a) override { m_grad.tvec() = m_grad.tvec() * a; }

	// 将误差项和参数都置为0
	virtual void Zero() override { m_values.Zero(); Clear(); }

	// 获取参数的平方L2范数
	// L2范数公式为 向量所有元素的平方和的1/2次方
	virtual void SquaredL2Norm(cr_real* sqnorm) const override
	{
		CarpRobotTensor sqnorm_t; sqnorm_t.RefrenceMemory(CarpRobotDim(1), sqnorm);
		sqnorm_t.t<0>() = m_values.tvec().square().sum();
	}

	// 获取梯度的平方L2范数
	virtual void GSquaredL2Norm(cr_real* sqnorm) const override
	{
		CARP_ROBOT_ASSERT(m_grad.v != 0, u8"m_grad 内存是空的!");
		CarpRobotTensor sqnorm_t; sqnorm_t.RefrenceMemory(CarpRobotDim(1), sqnorm);
		sqnorm_t.Zero();
		sqnorm_t.t<0>() = m_grad.tvec().square().sum();
	}

	// 当前参数是否更新过了
	virtual bool IsUpdated() const override { return m_updated; }
	void SetUpdated(bool b) { m_updated = b; }

	// 是否有误差项
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
		CARP_ROBOT_ASSERT(read_name == name_copy, u8"反序列化名称错误, 当前是:" << read_name << "应当是:" << name_copy);
		m_values.Deserialize(file);
	}

public:
	std::string m_name;				// 当前参数的名字，用来调试用的
	CarpRobotDim m_dim;						// 当前参数的维度信息
	CarpRobotTensor m_values;				// 当前参数的值
	CarpRobotTensor m_grad;					// 当前参数的误差项
	bool m_updated = true;					// 当前参数是否更新过了
	bool m_has_grad = false;			// 是否有误差项
	CarpRobotParameterCollectionBase* m_owner = nullptr;	// 所属的参数收集器
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
	// 重置所有参数梯度
	void ResetGradient()
	{
		for (unsigned int i = 0; i < m_params.size(); ++i)
			m_params[i]->Clear();
	}

	// 投影权重
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


	// 设置权重衰减系数
	void SetWeightDecayLambda(cr_real lambda)
	{
		m_weight_decay.SetLambda(lambda);
	}

	// L2正则项求导
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

	// 获取参数衰减对象
	CarpRobotL2WeightDecay& GetWeightDecay() override { return m_weight_decay; }

	// 按比例缩放误差项
	void ScaleGradient(cr_real a)
	{
		for (unsigned int i = 0; i < m_all_params.size(); ++i)
			m_all_params[i]->ScaleGradient(a);
	}

public:
	// 获取参数收集器名字
	std::string GetFullName() const { return m_name; }

public:
	std::vector<CarpRobotParameterBase*> m_all_params;			// 所有参数的集合（包括Parameter 和 lookupParameter）
	std::vector<CarpRobotParameter*> m_params;					// Parameter参数的集合
	
private:
	CarpRobotL2WeightDecay m_weight_decay;						// 权重衰减

	std::string m_name;									// 收集器名字
	std::random_device m_rd;
	std::mt19937 m_gen;

private:
	std::vector<cr_real> m_project_scratch;
	std::vector<cr_real> m_gradient_norm_scratch;
};

// 运算节点
class CarpRobotNode
{
public:
	CarpRobotNode() {};
	CarpRobotNode(const std::vector<int>& args) : m_args(args) {}
	virtual ~CarpRobotNode() {}

public:
	// 执行向前计算，对Batch进行处理，然后调用ForwardImpl
	virtual void Forward(const std::vector<const CarpRobotTensor*>& xs, CarpRobotTensor& fx) = 0;
	// 执行反向计算，对Batch进行处理，然后调用BackwardImpl
	virtual void Backward(const std::vector<const CarpRobotTensor*>& xs,	// 该节点的输入值
		const CarpRobotTensor& fx,											// 该节点的输出值
		const CarpRobotTensor& dEdf,										// 该节点的输出节点的误差项
		unsigned int xs_i,													// 该节点的输入节点索引
		CarpRobotTensor& dEdxi) = 0;									// 输入节点的误差项
	
public:
	// 保存输入节点的下标
	std::vector<int> m_args;
};

// 矩阵输入节点
class CarpRobotInputNode : public CarpRobotNode
{
public:
	// 值拷贝
	CarpRobotInputNode(const CarpRobotDim& d, const std::vector<cr_real>& dat) : m_dim(d), m_data(dat), m_pdata(&m_data) {}
	// 地址拷贝
	CarpRobotInputNode(const CarpRobotDim& d, std::vector<cr_real>* pdat) : m_dim(d), m_data(), m_pdata(pdat) {}

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
	CarpRobotScalarInputNode(cr_real s) : m_data(s), m_pdata(&m_data), m_dim(1) {}
	// 地址拷贝
	CarpRobotScalarInputNode(cr_real* ps) : m_data(0), m_pdata(ps), m_dim(1) {}

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
class CarpRobotParameterNodeBase : public CarpRobotNode
{
public:
	virtual ~CarpRobotParameterNodeBase() {}
	virtual void AccumulateGrad(const CarpRobotTensor& g) {}
};

// 普通参数输入节点
class CarpRobotParameterNode : public CarpRobotParameterNodeBase
{
public:
	CarpRobotParameterNode(CarpRobotParameter* p) : m_dim(p->m_dim), m_params(p) {}

public:
	// 执行反向传递梯度
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

// 常量参数输入节点
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
		fx.SetDim(xs[0]->d);
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
		CARP_ROBOT_ASSERT(xs[0]->d.SoftEqual(xs[1]->d), u8"CarpRobotCwiseSumNode 两个张量的维度必须一致, xs[0]:" << xs[0]->d.ToString() << " != xs[1]:" << xs[1]->d.ToString());

		fx.SetDim(xs[0]->d);
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
		
		fx.SetDim(xs[0]->d);
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

		fx.SetDim(xs[0]->d);
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
		CARP_ROBOT_ASSERT(xs[0]->d.Cols() == xs[1]->d.Rows(), u8"CarpRobotMatrixMultiplyNode 前项的列必须等于后项的行");
		CARP_ROBOT_ASSERT(xs[0]->d.nd <= 2 && xs[1]->d.nd <= 2, "CarpRobotMatrixMultiplyNode 矩阵相乘最多是2维的");

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
		fx.SetDim(xs[0]->d);
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
		CARP_ROBOT_ASSERT(xs[0]->d.SoftEqual(xs[1]->d), u8"CarpRobotCwiseQuotientNode 两个张量的维度必须一致, xs[0]:" << xs[0]->d.ToString() << " != xs[1]:" << xs[1]->d.ToString());

		fx.SetDim(xs[0]->d);

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
		CARP_ROBOT_ASSERT(xs.size() == 1, u8"CarpRobotSquareNode 必须是一个输入");
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
		CARP_ROBOT_ASSERT(xs.size() == 2, u8"CarpRobotBinaryLogLossNode 必须是两个输入");
		CARP_ROBOT_ASSERT(xs[0]->d.Rows() == 2 || xs[0]->d.nd == 1, u8"输入维度信息错误");
		CARP_ROBOT_ASSERT(xs[1]->d.Rows() == 2 || xs[1]->d.nd == 1, u8"输入维度信息错误");

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

// 1. 计算图，保存所有节点的拓扑结构
// 2. 前向计算，反向传播
class CarpRobotComputationGraph : public CarpRobotComputationGraphBase
{
public:
	// 构造函数，初始化
	CarpRobotComputationGraph() { }
	~CarpRobotComputationGraph() { Clear(); }

public:
	// 添加一个标量输入节点
	CarpRobotExpression AddInput(cr_real s) { return CarpRobotExpression(this, AddNode(new CarpRobotScalarInputNode(s))); }
	CarpRobotExpression AddInput(cr_real* ps) { return CarpRobotExpression(this, AddNode(new CarpRobotScalarInputNode(ps))); }

	// 添加指定结构的输入
	CarpRobotExpression AddInput(const CarpRobotDim& d, const std::vector<cr_real>& data) { return CarpRobotExpression(this, AddNode(new CarpRobotInputNode(d, data))); }
	CarpRobotExpression AddInput(const CarpRobotDim& d, std::vector<cr_real>* pdata) { return CarpRobotExpression(this, AddNode(new CarpRobotInputNode(d, pdata))); }

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
		m_nfxs.push_back(CarpRobotTensor());

		// 如果需要世界计算，那么进行向前计算
		if (m_immediate_compute) Forward();
		// 返回索引
		return (int)m_nodes.size() - 1;
	}

	// 添加一个参数节点
	int AddParameterNode(CarpRobotParameterNodeBase* node)
	{
		// 添加到图
		int index = AddNode(node);
		// 标记参数节点
		m_parameter_nodes[index] = node;
		// 返回索引
		return index;
	}

public:
	const CarpRobotTensor& GetValue(int i) override
	{
		Forward();
		CARP_ROBOT_ASSERT(i < (int)m_nfxs.size(), u8"访问越界");
		return m_nfxs[i];
	}
	const CarpRobotTensor& GetGradient(int i) const  override
	{
		CARP_ROBOT_ASSERT(i < (int)m_ndEdfs.size(), u8"访问越界");
		return m_ndEdfs[i];
	}

public:
	// 清理所有信息
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

	// 拓扑结构不变，从头开始算
	void Invalidate()
	{
		m_evaluated_index = 0;
	}

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
			xs.resize(node->m_args.size());
			for (unsigned int i = 0; i < node->m_args.size(); ++i)
				xs[i] = &m_nfxs[node->m_args[i]];

			// 执行节点的前向计算
			node->Forward(xs, m_nfxs[m_evaluated_index]);

			// 向前走一步
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

	// 反向传播
	void Backward()
	{
		// 执行向前计算，保证所有节点全部计算完毕
		Forward();

		// 获取当前节点个数
		int num_nodes = (int)m_nodes.size();

		// 设置误差项大小
		m_ndEdfs.resize(num_nodes);

		// 初始化维度信息和内存，然后清零
		for (unsigned int i = 0; i < m_ndEdfs.size(); ++i)
			m_ndEdfs[i].SetDim(m_nfxs[i].d, true);
		m_ndEdfs[num_nodes - 1].Constant(1);

		// 参数节点需要反向传播
		std::vector<bool> needs_derivative(num_nodes, false);
		for (auto& pair : m_parameter_nodes)
			needs_derivative[pair.first] = true;

		// 把拥有参数节点的节点，设置为需要反向传播
		for (int i = 0; i < num_nodes; ++i)
		{
			bool need = needs_derivative[i];
			if (need) continue;

			// 如果输入节点的某个节点需要求导，那么本节点也需要求导
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
			xs.resize(node->m_args.size());
			for (unsigned int j = 0; j < node->m_args.size(); ++j)
			{
				int arg = node->m_args[j];

				// 标记这个节点为参与计算的节点
				in_computation[arg] = true;

				// 设置输入节点
				xs[j] = &m_nfxs[arg];
			}

			// 遍历所有输入节点，需要求导的输入都要进行反向传播
			for (unsigned int j = 0; j < node->m_args.size(); ++j)
			{
				int arg = node->m_args[j];
				if (needs_derivative[arg])
					node->Backward(xs, m_nfxs[i], m_ndEdfs[i], j, m_ndEdfs[arg]);
			}
		}

		// 把最后的结果设置到节点上去
		for (auto& pair : m_parameter_nodes)
			pair.second->AccumulateGrad(m_ndEdfs[pair.first]);
	}

	// 获取梯度数量
	int GetGradCount() { return (int)m_ndEdfs.size(); }
	// 获取某个梯度值
	cr_real GetGradAsScalar(int i) { if (i >= (int)m_ndEdfs.size()) return 0; return m_ndEdfs[i].AsScalar(); }

private:
	std::vector<CarpRobotNode*> m_nodes;			// 用于存放计算图的拓扑结构
	std::map<int, CarpRobotParameterNodeBase*> m_parameter_nodes;	// 记录参数节点的索引

private:
	std::vector<CarpRobotTensor> m_nfxs;			// 保存前向计算的结果
	std::vector<CarpRobotTensor> m_ndEdfs;		// 保存反向计算的结果
	int m_evaluated_index = 0;				// 记录当前计算到那个节点
	bool m_immediate_compute = false;			// 是否立即计算
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
	 *	Trainer构造函数
	 *	@param m : 需要训练的ParameterCollection引用
	 *	@param learning_rate : 初始化的学习率
	**/
	CarpRobotTrainer(CarpRobotParameterCollection* m, cr_real learning_rate) :
		m_learning_rate(learning_rate),			// 学习率
		m_clipping_enabled(true),				// 是否开启梯度裁剪
		m_clip_threshold(5),					// 裁剪阔值
		m_clips(0),								// 裁剪次数
		m_updates(0),							// 更新次数
		m_model(m)
	{}
	virtual ~CarpRobotTrainer() {}

public:
	/**
	 *	根据适当的规则更新parameters
	**/
	virtual void Update()
	{
		// 获取裁切的比率
		const cr_real gscale = ClipGradients();
		// 获取parameters列表
		const std::vector<CarpRobotParameter*>& params = m_model->m_params;
		// 遍历parameters列表
		for (size_t i = 0; i < params.size(); ++i)
		{
			// 是否应该更新这个参数
			if (params[i]->m_updated)
			{
				// 按照gscale更新这个参数
				UpdateParams(gscale, i);
				// 将梯度值清零
				params[i]->Clear();
			}
		}

		++m_updates;

		// 获取误差对象
		auto& wd = m_model->GetWeightDecay();
		// 更新权重
		wd.UpdateWeightDecay();
		if (wd.ParametersNeedRescaled())
			RescaleAndResetWeightDecay();
	}

	/**
	 *	重启optimizer
	 *	清空所有momentum 和 assimilate
	**/
	virtual void Restart() = 0;

	/**
	 *	重启optimizer
	 *	清空所有momentum 和 assimilate 和重新设置 学习率

	 *	@param lr : 学习率
	**/
	void SetLearningRate(cr_real lr) { m_learning_rate = lr; }

	/**
	 *	梯度裁切
	 *	如果允许被裁切并且梯度过大，并返回适当的裁切比例因子

	 *	@return : 适当的裁切比例因子
	**/
	cr_real ClipGradients()
	{
		// 定义gscale并初始化为1
		cr_real g_scale = 1;

		// 如果允许裁剪
		if (m_clipping_enabled)
		{
			// 获取经过L2范数化后的梯度
			cr_real gg = m_model->GradientL2Norm();

			// gg不能无穷大，并且为数字
			// isnan : 测试浮点数是否是非数字
			// isinf : 测试浮点数是否无限大
			if ((std::isnan(gg)) || (std::isinf(gg)))
			{
				// CARP_ERROR(u8"Magnitude of gradient is bad:" << gg);
			}
			else
			{
				// 如果gg>m_clip_threshold
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
	 *	TODO : 这是暂时不受保护的直到能更好的序列化权重衰减在保存模板的时候
	 *	由这个model重新调整参数
	**/
	void RescaleAndResetWeightDecay()
	{
		// 获取误差项
		const cr_real weitht_decay = m_model->GetWeightDecay().CurrentWeightDecay();

		// 将误差项传递到model每个parameters 和 lookup_parameters 对象
		for (unsigned int i = 0; i < m_model->m_params.size(); ++i)
		{
			auto* p = m_model->m_params[i];
			if (p->IsUpdated())
				p->ScaleParameters(weitht_decay);
		}

		// 重置误差项
		m_model->GetWeightDecay().ResetWeightDecay();
	}

public:
	cr_real m_learning_rate = 0.1f;	// 学习率

	/* 梯度裁剪(产生梯度爆炸时用) */
	bool m_clipping_enabled = false;			// 是否允许梯度裁剪
	cr_real m_clip_threshold = 0;				// 裁剪临界值
	cr_real m_clips = 0;
	cr_real m_updates = 0;

	CarpRobotParameterCollection* m_model = nullptr;		// ParameterCollection对象

protected:
	CarpRobotTrainer() {}
	/**
	*	设置更新参数的实际规则
	*
	*	@param scale ： 更新的比率(i.e. 学习率)
	*	@param gscale : 基于裁切的梯度比率
	*	@param values : 更新特别指定的值
	*/
	virtual void UpdateRule(cr_real gscale, const std::vector<CarpRobotTensor*>& values) = 0;
	/**
	*	parameter更新函数
	*
	*	@param scale : 更新比率 (i.e. learning rate)
	*	@param gscale : 基于裁切的梯度比率
	*	@param idx : 需要更新的parameter索引
	*/
	virtual void UpdateParams(cr_real gscale, size_t idx) = 0;
};

class CarpRobotSimpleSGDTrainer : public CarpRobotTrainer
{

public:
	/**
	 *	@param m : 需要训练的ParameterCollection引用
	 *	@param learning_rate : 初始化的学习率
	**/
	CarpRobotSimpleSGDTrainer(CarpRobotParameterCollection* m, cr_real learning_rate = 0.1) : CarpRobotTrainer(m, learning_rate) {}
	~CarpRobotSimpleSGDTrainer() {}

	void Restart() {}

	void Close() {}

protected:
	/**
	*	parameter更新函数
	*
	*	@param scale : 更新比率 (i.e. learning rate)
	*	@param gscale : 基于裁切的梯度比率
	*	@param idx : 需要更新的parameter索引
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
	*	设置更新参数的实际规则
	*
	*	@param scale ： 更新的比率(i.e. 学习率)
	*	@param gscale : 基于裁切的梯度比率
	*	@param values : 更新特别指定的值
	*/
	void UpdateRule(cr_real g_scale, const std::vector<CarpRobotTensor*>& values) override
	{
		values[0]->tvec() -= values[1]->tvec() * (m_learning_rate * g_scale / m_model->GetWeightDecay().CurrentWeightDecay());
	}
};

#endif