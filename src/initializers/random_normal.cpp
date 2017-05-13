#include "core/common_cu.h"

#include "initializers/random_normal.h"
#include "nodes/variable.h"

#include <random>

RandomNormal::RandomNormal(const deepflow::InitParam &_block_param) : Initializer(_block_param) {
	LOG_IF(FATAL, _block_param.has_random_normal_param() == false) << "param.has_random_normal_param() == false";
}

void RandomNormal::apply(Variable *variable) {
	auto size = variable->output(0)->value()->size();
	float mean = _param.random_normal_param().mean();
	float stddev = _param.random_normal_param().stddev();
	std::mt19937 generator;
	generator.seed(std::random_device()());
	std::normal_distribution<float> distribution(mean, stddev);
	float *h_rand = new float[size];
	for (int i = 0; i < size; ++i)
		h_rand[i] = distribution(generator);
	DF_CUDA_CHECK(cudaMemcpy((float*)variable->output(0)->value()->mutableData(), h_rand, variable->output(0)->value()->sizeInBytes(), cudaMemcpyHostToDevice));
}

std::string RandomNormal::to_cpp() const
{
	std::string cpp = "df.random_normal(";
	cpp += "{" + std::to_string(_dims[0]) + ", " + std::to_string(_dims[1]) + ", " + std::to_string(_dims[2]) + ", " + std::to_string(_dims[3]) + "}, ";
	float mean = _param.random_normal_param().mean();
	float stddev = _param.random_normal_param().stddev();
	cpp += std::to_string(mean) + ", " + std::to_string(stddev) + ")";
	return cpp;
}
