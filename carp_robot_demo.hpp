#ifndef CARP_ROBOT_DEMO_INCLUDED
#define CARP_ROBOT_DEMO_INCLUDED

#include "carp_robot.hpp"
#include "carp_robot_model.hpp"
#include "carp_log.hpp"

class CarpRobotDemo
{
public:
	// 基本运算
    void Demo1()
    {
		CarpRobotComputationGraph g;

		std::vector<float> data_a = { 1, 2, 3, 4, 5, 6 };
		CarpRobotDim dim_a({ 2, 3 });
		auto a = g.AddInput(dim_a, data_a);
		
		std::vector<float> data_b = { 1, 2, 3, 4, 5, 6 };
		CarpRobotDim dim_b({ 2, 3 });
		auto b = g.AddInput(dim_b, data_b);

		std::vector<float> data_c = { 1, 2, 3, 4, 5, 6 };
		CarpRobotDim dim_c({ 3, 2 });
		auto c = g.AddInput(dim_c, data_c);

		auto negate = -a;
		CARP_INFO("Negate:" << negate.GetValue().ToString());

		auto cwise_sum = a + b;
		CARP_INFO("CwiseSum:" << cwise_sum.GetValue().ToString());

		auto constant_plus_x = 10 + a;
		CARP_INFO("ConstantPlusX:" << constant_plus_x.GetValue().ToString());

		auto constant_minus_x = 10 - a;
		CARP_INFO("ConstantMinusX:" << constant_minus_x.GetValue().ToString());

		auto matrix_multiply = a * c;
		CARP_INFO("MatrixMultiply:" << matrix_multiply.GetValue().ToString());
		
		auto scalar_multiply = a * 10;
		CARP_INFO("ScalarMultiply:" << scalar_multiply.GetValue().ToString());
		
		auto cwise_quotient = a  / b;
		CARP_INFO("CwiseQuotient:" << cwise_quotient.GetValue().ToString());
    }

	// 线性分类
	void Demo2()
	{
		std::default_random_engine rng;
		std::normal_distribution<float> normal(-0.1f, 1.0f);

		// 固定一个x点
		float x = -1 + 2.0f / 100;
		std::vector<float> xs;
		std::vector<float> ys;
		for (int i = 0; i < 100; ++i) {
			// 然后随机y = 2x附近随机几个点
			float y = 2 * x + normal(rng) * 0.33f;

			// 作为数据点添加进去
			xs.push_back(x);
			ys.push_back(y);
		}

		CarpRobotParameterCollection model;
		auto* pW = model.AddParameters(CarpRobotDim({ 1 }));

		CarpRobotSGDTrainer trainer(model, 0.1f);
		CarpRobotComputationGraph cg;

		for (size_t i = 0; i < xs.size(); ++i) {
			auto W = cg.AddParameters(pW);
			auto pred = W * xs[i];
			auto loss = (pred - ys[i]).Square();
			cg.Forward();
			CARP_INFO("loss:" << loss.GetValue().AsScalar());
			cg.Backward();
			trainer.Update();
			cg.Clear();
		}
		// 最后训练的结果应该是和2相近的一个数
		CARP_INFO("w:" << pW->GetValue().AsScalar());
	}

	// XOR
	void Demo3()
	{
		CarpRobotParameterCollection model;
		CarpRobotLinear linear_1(model, 2, 8);
		CarpRobotLinear linear_2(model, 8, 1);

		CarpRobotAdamTrainer trainer(model, 0.1f);
		CarpRobotComputationGraph cg;

		std::vector<float> x_value = { 0, 0 };
		float y_value = 0;

		linear_1.Build(cg);
		linear_2.Build(cg);
		auto x = cg.AddInput(CarpRobotDim({ 2 }), &x_value);
		auto y = cg.AddInput(&y_value);
		auto out_1 = linear_1.Forward(x);
		out_1 = out_1.Sigmoid();
		auto out_2 = linear_2.Forward(out_1);
		auto loss = (out_2 - y).Square();

		for (int aa = 0; aa < 1000; ++aa) {
			x_value[0] = 0;
			x_value[1] = 0;
			y_value = 0;

			cg.Backward();
			trainer.Update();
			cg.Invalidate();

			x_value[0] = 0;
			x_value[1] = 1;
			y_value = 1;

			cg.Backward();
			trainer.Update();
			cg.Invalidate();

			x_value[0] = 1;
			x_value[1] = 0;
			y_value = 1;

			cg.Backward();
			trainer.Update();
			cg.Invalidate();

			x_value[0] = 1;
			x_value[1] = 1;
			y_value = 0;

			cg.Backward();
			trainer.Update();
			cg.Invalidate();
		}

		{
			x_value[0] = 0;
			x_value[1] = 0;
			auto result = out_2.GetValue().AsScalar();
			std::string check = std::abs(result - 0) < 0.000001 ? "yes" : "no";
			CARP_INFO(check << "\t" << x_value[0] << ", " << x_value[1] << "=" << result);
			cg.Invalidate();

			x_value[0] = 0;
			x_value[1] = 1;
			result = out_2.GetValue().AsScalar();
			check = std::abs(result - 1) < 0.000001 ? "yes" : "no";
			CARP_INFO(check << "\t" << x_value[0] << ", " << x_value[1] << "=" << result);
			cg.Invalidate();

			x_value[0] = 1;
			x_value[1] = 0;
			result = out_2.GetValue().AsScalar();
			check = std::abs(result - 1) < 0.000001 ? "yes" : "no";
			CARP_INFO(check << "\t" << x_value[0] << ", " << x_value[1] << "=" << result);
			cg.Invalidate();

			x_value[0] = 1;
			x_value[1] = 1;
			result = out_2.GetValue().AsScalar();
			check = std::abs(result - 0) < 0.000001 ? "yes" : "no";
			CARP_INFO(check << "\t" << x_value[0] << ", " << x_value[1] << "=" << result);
			cg.Invalidate();
		}
	}
};

#endif