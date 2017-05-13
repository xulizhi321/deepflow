#include "nodes/psnr.h"
#include "core/common_cu.h"

__global__
void SquareErrorKernel(const int n, const float * __restrict__ a, const float * __restrict__ b, float * __restrict__ c)
{
	int i = blockIdx.x*blockDim.x + threadIdx.x;
	if (i < n) {
		float tmp = a[i] - b[i];
		c[i] = tmp * tmp;
	}
}

Psnr::Psnr(const deepflow::NodeParam &_block_param) : Node(_block_param) {
	LOG_IF(FATAL, _block_param.has_psnr_param() == false) << "param.has_psnr_param() == false";
	auto printParam = _param.print_param();	
	_print_time = (PrintTime)printParam.print_time();
}

void Psnr::initForward() {
	LOG_IF(FATAL, _inputs[0]->value()->size() != _inputs[0]->value()->size()) << "Size mismatch [FAILED]";	
	LOG(INFO) << "Initializing PSNR " << _name;
	DF_CUDNN_CHECK(cudnnCreate(&_cudnnHandle));
	DF_CUDA_CHECK(cudaMalloc(&d_square_error, _inputs[0]->value()->sizeInBytes()));
	DF_CUDA_CHECK(cudaMalloc(&d_sum_square_error, sizeof(float)));
	DF_CUDNN_CHECK(cudnnCreateReduceTensorDescriptor(&_reduce_tensor_desciptor));
	cudnnReduceTensorOp_t op = CUDNN_REDUCE_TENSOR_ADD;
	cudnnReduceTensorIndices_t indices = CUDNN_REDUCE_TENSOR_NO_INDICES;
	DF_CUDNN_CHECK(cudnnSetReduceTensorDescriptor(_reduce_tensor_desciptor, op, CUDNN_DATA_FLOAT, CUDNN_PROPAGATE_NAN, indices, CUDNN_32BIT_INDICES));	
	DF_CUDNN_CHECK(cudnnCreateTensorDescriptor(&_output_desc));
	DF_CUDNN_CHECK(cudnnSetTensor4dDescriptor(_output_desc, CUDNN_TENSOR_NCHW, CUDNN_DATA_FLOAT, 1, 1, 1, 1));
	DF_CUDNN_CHECK(cudnnGetReductionWorkspaceSize(_cudnnHandle, _reduce_tensor_desciptor, _inputs[0]->value()->descriptor(), _output_desc, &_workspaceSizeInBytes));
	DF_CUDA_CHECK(cudaMalloc(&_d_workspace, _workspaceSizeInBytes));
}

void Psnr::initBackward() {

}

void Psnr::forward() {
	if (_print_time == END_OF_EPOCH && _context->last_batch == false)
		return;
	auto size = _inputs[0]->value()->size();
	SquareErrorKernel <<< numOfBlocks(size), maxThreadsPerBlock >>> (size, (float*)_inputs[0]->value()->data(), (float*)_inputs[1]->value()->data(), d_square_error);
	DF_KERNEL_CHECK();	
	DF_CUDNN_CHECK(
		cudnnReduceTensor(
			_cudnnHandle,
			_reduce_tensor_desciptor,
			NULL,
			NULL,
			_d_workspace,
			_workspaceSizeInBytes,
			&alpha,
			_inputs[0]->value()->descriptor(),
			d_square_error,
			&beta,
			_output_desc,
			d_sum_square_error));
	float sse;
	DF_CUDA_CHECK(cudaMemcpy(&sse, d_sum_square_error, sizeof(float), cudaMemcpyDeviceToHost));
	float psnr = 20.0f * log10(2.0f) - 10.0f * log10(sse / size);
	LOG(INFO) << _name << " - PSNR: " << psnr;
}

void Psnr::backward() {
	auto size = _inputs[0]->value()->size();
	DF_KERNEL_CHECK();
}

std::string Psnr::to_cpp() const
{	
	std::string cpp = "df.psnr(" + _input_name_for_cpp(0) + ", " + _input_name_for_cpp(1) + ", ";
	if (_print_time == END_OF_EPOCH) {
		cpp += "Psnr::END_OF_EPOCH, ";
	}
	else {
		cpp += "Psnr::EVERY_PASS, ";
	}
	cpp += "\"" + _name + "\", ";
	cpp += "{" + _to_cpp_phases() + "});";
	return cpp;
}
