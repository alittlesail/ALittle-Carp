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

private:
	CarpRobotParameter* m_k = nullptr;
	CarpRobotExpression m_K;

private:
	int m_stride_width = 0;
	int m_stride_height = 0;
	bool m_padding_type = true;
};

class CarpRobotLstm
{
public:
	CarpRobotLstm(CarpRobotParameterCollection* model
		, int num_lstm, int input_dim, int hidden_dim, bool ln_lstm)
		: m_model(model), m_num_lstm(num_lstm), m_ln_lstm(ln_lstm), m_input_dim(input_dim), m_hidden_dim(hidden_dim)
	{
		int layer_input_dim = input_dim;
		for (int i = 0; i < num_lstm; ++i)
		{
			auto* p_x2i = model->AddParameters(CarpRobotDim({ hidden_dim * 4, layer_input_dim }), "LSTM-p_x2i");
			auto* p_h2i = model->AddParameters(CarpRobotDim({ hidden_dim * 4, hidden_dim }), "LSTM-p_h2i");
			auto* p_bi = model->AddParameters(CarpRobotDim({ hidden_dim * 4 }), "LSTM-p_bi");

			std::vector<CarpRobotParameter*> ps;
			ps.push_back(p_x2i);
			ps.push_back(p_h2i);
			ps.push_back(p_bi);
			m_params.push_back(ps);

			if (m_ln_lstm)
			{
				auto* p_gh = model->AddParameters(CarpRobotDim({ hidden_dim * 4 }), "LSTM-p_gh");
				auto* p_bh = model->AddParameters(CarpRobotDim({ hidden_dim * 4 }), "LSTM-p_bh");
				auto* p_gx = model->AddParameters(CarpRobotDim({ hidden_dim * 4 }), "LSTM-p_gx");
				auto* p_bx = model->AddParameters(CarpRobotDim({ hidden_dim * 4 }), "LSTM-p_bx");
				auto* p_gc = model->AddParameters(CarpRobotDim({ hidden_dim }), "LSTM-p_gc");
				auto* p_bc = model->AddParameters(CarpRobotDim({ hidden_dim }), "LSTM-p_bc");
				std::vector<CarpRobotParameter*> ln_ps;
				ln_ps.push_back(p_gh);
				ln_ps.push_back(p_bh);
				ln_ps.push_back(p_gx);
				ln_ps.push_back(p_bx);
				ln_ps.push_back(p_gc);
				ln_ps.push_back(p_bc);
				m_ln_params.push_back(ln_ps);
			}

			layer_input_dim = hidden_dim;
		}
	}

public:
	enum IndexType
	{
		_X2I = 0,
		_H2I = 1,
		_BI = 2,

		LN_GH = 0,
		LN_BH = 1,
		LN_GX = 2,
		LN_BX = 3,
		LN_GC = 4,
		LN_BC = 5
	};

public:
	void BuildForLua(CarpRobotComputationGraph* graph, bool update)
	{
		Build(graph, nullptr, nullptr, update);
	}

	void Build(CarpRobotComputationGraph* graph, std::vector<CarpRobotExpression>* hinit, std::vector<CarpRobotExpression>* cinit, bool update)
	{
		// 重置当前下标
		m_cur = -1;

		// 把参数添加到图里面, 把表达式存起来
		m_exprs.clear();
		m_ln_exprs.clear();

		for (int i = 0; i < m_num_lstm; ++i)
		{
			auto& ps = m_params[i];
			std::vector<CarpRobotExpression> exprs;
			for (auto& p : ps)
			{
				if (update)
					exprs.push_back(graph->AddParameters(p));
				else
					exprs.push_back(graph->AddConstParameters(p));
			}
			m_exprs.push_back(exprs);

			if (m_ln_lstm)
			{
				auto& ln_ps = m_ln_params[i];
				std::vector<CarpRobotExpression> ln_exprs;
				for (auto& ps : ln_ps)
				{
					if (update)
						ln_exprs.push_back(graph->AddParameters(ps));
					else
						ln_exprs.push_back(graph->AddConstParameters(ps));
				}
				m_ln_exprs.push_back(ln_exprs);
			}
		}

		// 初始化序列
		m_h.clear();
		m_c.clear();

		m_has_initial_state = false;
		int hinit_len = 0; if (hinit) hinit_len = (int)hinit->size();
		int cinit_len = 0; if (cinit) cinit_len = (int)cinit->size();
		if (hinit_len > 0 && cinit_len > 0)
		{
			CARP_ROBOT_ASSERT(m_num_lstm == hinit_len && m_num_lstm == cinit_len, u8"hinit和cinit的长度必须是lstm相等");

			m_h0.clear();
			m_c0.clear();
			for (int i = 0; i < m_num_lstm; ++i)
			{
				m_h0[i] = (*hinit)[i];
				m_c0[i] = (*cinit)[i];
			}
			m_has_initial_state = true;
		}

		// 标记dropout
		m_masks.clear();
		for (int i = 0; i < m_num_lstm; ++i)
		{
			std::vector<CarpRobotExpression> masks_i;
			int idim = m_hidden_dim;
			if (i == 0) idim = m_input_dim;

			if (m_dropout_rate > 0 || m_dropout_rate_h > 0)
			{
				float retention_rate = 1 - m_dropout_rate;
				float retention_rate_h = 1 - m_dropout_rate_h;
				float scale = 1 / retention_rate;
				float scale_h = 1 / retention_rate_h;

				// input
				CarpRobotDim dim_input({ idim });
				masks_i.push_back(graph->RandomBernoulli(dim_input, retention_rate, scale));
				// h
				CarpRobotDim dim_h({ m_hidden_dim });
				masks_i.push_back(graph->RandomBernoulli(dim_h, retention_rate_h, scale_h));
				m_masks.push_back(masks_i);
			}
		}
	}

public:
	void SetDropoutRate(float rate) { m_dropout_rate = rate; }
	void SetDropoutRateH(float rate) { m_dropout_rate_h = rate; }

public:
	int AddInputForLua(CarpRobotComputationGraph* graph, int input_index) { return AddInput(CarpRobotExpression(graph, input_index)).GetIndex(); }

	CarpRobotExpression AddInput(CarpRobotExpression input, int in_index) { return AddInput(input, in_index, 0); }
	CarpRobotExpression AddInput(CarpRobotExpression input, int* out_index) { return AddInput(input, m_cur, out_index); }
	CarpRobotExpression AddInput(CarpRobotExpression input) { return AddInput(input, m_cur, 0); }

	CarpRobotExpression AddInput(CarpRobotExpression input, int in_index, int* out_index)
	{
		m_h.push_back(std::vector<CarpRobotExpression>());
		std::vector<CarpRobotExpression>& ht = m_h.back();
		ht.resize(m_num_lstm);
		m_c.push_back(std::vector<CarpRobotExpression>());
		std::vector<CarpRobotExpression>& ct = m_c.back();
		ct.resize(m_num_lstm);

		int cur = in_index;

		CarpRobotExpression x = input;
		for (int i = 0; i < m_num_lstm; ++i)
		{
			std::vector<CarpRobotExpression>& exprs = m_exprs[i];

			CarpRobotExpression i_h_tm1, i_c_tm1;
			bool has_prev_state = (cur >= 0 || m_has_initial_state);
			if (cur < 0)
			{
				if (m_has_initial_state)
				{
					// intial value for h and c at timestep 0 in layer i
					// defaults to zero matrix input if not set in add_parameter_edges
					i_h_tm1 = m_h0[i];
					i_c_tm1 = m_c0[i];
				}
			}
			else
			{
				i_h_tm1 = m_h[cur][i];
				i_c_tm1 = m_c[cur][i];
			}

			// apply dropout according to https://arxiv.org/abs/1512.05287 (tied weights)
			if (m_dropout_rate > 0)
				x = x.CwiseMultiply(m_masks[i][0]);
			if (has_prev_state && m_dropout_rate_h > 0)
				i_h_tm1 = i_h_tm1.CwiseMultiply(m_masks[i][1]);

			// input
			CarpRobotExpression tmp;
			if (m_ln_lstm)
			{
				std::vector<CarpRobotExpression>& ln_exprs = m_ln_exprs[i];
				if (has_prev_state)
					tmp = exprs[_BI] + (exprs[_X2I] * x).LayerNorm(ln_exprs[LN_GX], ln_exprs[LN_BX]) + (exprs[_H2I] * i_h_tm1).LayerNorm(ln_exprs[LN_GH], ln_exprs[LN_BH]);
				else
					tmp = exprs[_BI] + (exprs[_X2I] * x).LayerNorm(ln_exprs[LN_GX], ln_exprs[LN_BX]);
			}
			else
			{
				if (has_prev_state)
				{
					std::vector<CarpRobotExpression> ii;
					ii.push_back(exprs[_X2I]); ii.push_back(x);
					ii.push_back(exprs[_H2I]); ii.push_back(i_h_tm1);
					tmp = exprs[_BI].AffineTransform(ii);
				}
				else
				{
					std::vector<CarpRobotExpression> ii;
					ii.push_back(exprs[_X2I]); ii.push_back(x);
					tmp = exprs[_BI].AffineTransform(ii);
				}
			}

			CarpRobotExpression i_ait = tmp.PickRange(0, m_hidden_dim, 0);
			CarpRobotExpression i_aft = tmp.PickRange(m_hidden_dim, m_hidden_dim * 2, 0);
			CarpRobotExpression i_aot = tmp.PickRange(m_hidden_dim * 2, m_hidden_dim * 3, 0);
			CarpRobotExpression i_agt = tmp.PickRange(m_hidden_dim * 3, m_hidden_dim * 4, 0);
			CarpRobotExpression i_it = i_ait.Sigmoid();
			// TODO(odashi): Should the forget bias be a hyperparameter?
			CarpRobotExpression i_ft = (i_aft + 1).Sigmoid();
			CarpRobotExpression i_ot = i_aot.Sigmoid();
			CarpRobotExpression i_gt = i_agt.Tanh();

			if (has_prev_state)
				ct[i] = i_ft.CwiseMultiply(i_c_tm1) + i_it.CwiseMultiply(i_gt);
			else
				ct[i] = i_it.CwiseMultiply(i_gt);

			if (m_ln_lstm)
			{
				std::vector<CarpRobotExpression>& ln_exprs = m_ln_exprs[i];
				ht[i] = i_ot.CwiseMultiply(ct[i].LayerNorm(ln_exprs[LN_GC], ln_exprs[LN_BC]).Tanh());
				x = ht[i];
			}
			else
			{
				ht[i] = i_ot.CwiseMultiply(ct[i].Tanh());
				x = ht[i];
			}
		}

		m_cur = m_cur + 1;
		if (out_index) *out_index = m_cur;

		return ht.back();
	}

private:
	// 参数模型列表
	std::vector<std::vector<CarpRobotParameter*>> m_params;
	// 归一化参数模型列表
	std::vector<std::vector<CarpRobotParameter*>> m_ln_params;

	// 参数表达式列表
	std::vector<std::vector<CarpRobotExpression>> m_exprs;
	// 归一化表达式列表
	std::vector<std::vector<CarpRobotExpression>> m_ln_exprs;

	std::vector<std::vector<CarpRobotExpression>> m_h;
	std::vector<std::vector<CarpRobotExpression>> m_c;

	std::vector<CarpRobotExpression> m_h0;
	std::vector<CarpRobotExpression> m_c0;

	int m_cur = -1;
	bool m_has_initial_state = false;
	
	// 丢弃率
	std::vector<std::vector<CarpRobotExpression>> m_masks;
	float m_dropout_rate = 0.0f;
	float m_dropout_rate_h = 0.0f;

	CarpRobotParameterCollection* m_model = nullptr;
	int m_num_lstm = 0;
	bool m_ln_lstm = 0;
	int m_input_dim = 0;
	int m_hidden_dim = 0;
};

class CarpRobotBiLstm
{
public:
	CarpRobotBiLstm(CarpRobotParameterCollection* model, int num_lstm, int input_dim, int hidden_dim, bool ln_lstm)
		: m_l2r_lstm(model, num_lstm, input_dim, hidden_dim / 2, ln_lstm)
		, m_r2l_lstm(model, num_lstm, input_dim, hidden_dim / 2, ln_lstm)
	{
		CARP_ROBOT_ASSERT(hidden_dim % 2 == 0, u8"hidden_dim 必须是偶数");
	}
	~CarpRobotBiLstm() {}

public:
	void SetDropoutRate(float rate) { m_l2r_lstm.SetDropoutRate(rate); m_r2l_lstm.SetDropoutRate(rate); }
	void SetDropoutRateH(float rate) { m_l2r_lstm.SetDropoutRateH(rate); m_r2l_lstm.SetDropoutRateH(rate); }

public:
	void Build(CarpRobotComputationGraph* graph, bool update)
	{
		m_l2r_lstm.Build(graph, nullptr, nullptr, update);
		m_r2l_lstm.Build(graph, nullptr, nullptr, update);
	}

	void FullLstm(const std::vector<CarpRobotExpression>& input_list, std::vector<CarpRobotExpression>& out_list)
	{
		int input_len = (int)input_list.size();

		std::vector<CarpRobotExpression> l2r_output; l2r_output.resize(input_len);
		std::vector<CarpRobotExpression> r2l_output; r2l_output.resize(input_len);

		int pre_index = -1;
		for (int i = 0; i < input_len; ++i)
		{
			l2r_output[i] = m_l2r_lstm.AddInput(input_list[i]);
			r2l_output[input_len - i - 1] = m_r2l_lstm.AddInput(input_list[input_len - i - 1]);
		}

		for (int i = 0; i < input_len; ++i)
		{
			std::vector<CarpRobotExpression> ii;
			ii.push_back(r2l_output[i]);
			out_list.push_back(l2r_output[i].Concatenate(ii));
		}
	}

	CarpRobotExpression EmbeddingLstm(const std::vector<CarpRobotExpression>& input_list)
	{
		int input_len = (int)input_list.size();
		CarpRobotExpression l2r, r2l;
		for (int i = 0; i < input_len; ++i)
		{
			l2r = m_l2r_lstm.AddInput(input_list[i]);
			r2l = m_r2l_lstm.AddInput(input_list[input_len - i - 1]);
		}
		std::vector<CarpRobotExpression> ii;
		ii.push_back(r2l);
		return l2r.Concatenate(ii);
	}

private:
	CarpRobotLstm m_l2r_lstm;
	CarpRobotLstm m_r2l_lstm;
};
#endif