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
		CarpRobotDim dim_a(2, 3);
		auto a = g.AddInput(dim_a, data_a);
		
		std::vector<float> data_b = { 1, 2, 3, 4, 5, 6 };
		CarpRobotDim dim_b(2, 3);
		auto b = g.AddInput(dim_b, data_b);

		std::vector<float> data_c = { 1, 2, 3, 4, 5, 6 };
		CarpRobotDim dim_c(3, 2);
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
		std::mt19937 gen(rng());
		std::normal_distribution<float> normal(-0.1f, 1.0f);

		std::vector<float> xs;
		std::vector<float> ys;
		for (int i = 0; i < 100; ++i) {
			float x = i + 2.0f / 100;
			float y = 2 * x + normal(rng) * 0.33f;
			xs.push_back(x);
			ys.push_back(y);
		}

		CarpRobotParameterCollection model;
		auto* pW = model.AddParameters({ 1 });
		pW->GetValue().RandomizeUniform(gen, -0.1f, 0.1f);
		auto* pB = model.AddParameters({ 1 });
		pB->GetValue().RandomizeUniform(gen, -0.1f, 0.1f);

		CarpRobotSimpleSGDTrainer trainer(&model, 0.1f);

		CarpRobotComputationGraph cg;
		auto W = cg.AddParameters(pW);
		auto B = cg.AddParameters(pB);
		CARP_INFO("learn before:" << W.GetValue().AsScalar() << ", " << B.GetValue().AsScalar());

		for (unsigned i = 0; i < xs.size(); ++i) {
			auto pred = W * xs[i] + B;
			auto loss = (pred - ys[i]).Square();
			cg.Forward();
			CARP_INFO("loss:" << loss.GetValue().AsScalar());
			cg.Backward();
			trainer.Update();
			CARP_INFO("learnling:" << W.GetValue().AsScalar() << ", " << B.GetValue().AsScalar());
		}

		CARP_INFO("learn after:" << W.GetValue().AsScalar() << ", " << B.GetValue().AsScalar());
	}
};

#endif