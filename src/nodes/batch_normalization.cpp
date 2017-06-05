#include "nodes/batch_normalization.h"

BatchNormalization::BatchNormalization(const deepflow::NodeParam & param) : Node(param)
{
	LOG_IF(FATAL, param.has_batch_normalization_param() == false) << "param.has_batch_normalization_param() == false";
}

void BatchNormalization::initForward()
{
	auto param = _param.batch_normalization_param();
	auto input = _inputs[0];
	auto output = _outputs[0];
	auto inputDims = input->value()->dims();
	output->initValue(inputDims);
	_xDesc = input->value()->descriptor();
	_yDesc = output->value()->descriptor();
	_x = (float*)input->value()->mutableData();
	_y = (float*)output->value()->mutableData();
	DF_NODE_CUDNN_CHECK(cudnnCreateTensorDescriptor(&_bnScaleBiasMeanVarDesc));
	if (param.mode() == deepflow::BatchNormalizationparam_Mode_CUDNN_BATCHNORM_PER_ACTIVATION) {
		_batchNormMode = CUDNN_BATCHNORM_PER_ACTIVATION;
		DF_NODE_CUDNN_CHECK(cudnnSetTensor4dDescriptor(_bnScaleBiasMeanVarDesc, CUDNN_TENSOR_NCHW, CUDNN_DATA_FLOAT, 1, inputDims[1], inputDims[2], inputDims[3]));
		_size = inputDims[1] * inputDims[2] * inputDims[3];
	}
	else if (param.mode() == deepflow::BatchNormalizationparam_Mode_CUDNN_BATCHNORM_SPATIAL) {
		_batchNormMode = CUDNN_BATCHNORM_SPATIAL;
		DF_NODE_CUDNN_CHECK(cudnnSetTensor4dDescriptor(_bnScaleBiasMeanVarDesc, CUDNN_TENSOR_NCHW, CUDNN_DATA_FLOAT, 1, inputDims[1], 1, 1));
		_size = inputDims[1];
	}
	else {
		LOG(FATAL) << "Unsupported batch normalization mode.";
	}
	LOG(INFO) << "Initializing Batch Normalization " << _name << " - " << _outputs[0]->value()->shape();
	_eps = CUDNN_BN_MIN_EPSILON;
	DF_NODE_CUDNN_CHECK(cudnnCreate(&_cudnnHandle));
	DF_NODE_CUDA_CHECK(cudaMalloc(&_bnBias, _size * sizeof(float)));
	fill(_size, 0, _bnBias);
	DF_NODE_CUDA_CHECK(cudaMalloc(&_bnScale, _size * sizeof(float)));
	fill(_size, 1, _bnScale);
	DF_NODE_CUDA_CHECK(cudaMalloc(&_resultSaveMean, _size * sizeof(float)));
	DF_NODE_CUDA_CHECK(cudaMalloc(&_resultSaveInvVariance, _size * sizeof(float)));
}

void BatchNormalization::initBackward()
{	
	_outputs[0]->initDiff();
	_dy = (float*)_outputs[0]->diff()->mutableData();
	_dx = (float*)_inputs[0]->diff()->mutableData();
}

void BatchNormalization::forward()
{
	//if (_context->phase_behaviour == deepflow::PhaseParam_PhaseBehaviour_TRAIN || _context->phase_behaviour == deepflow::PhaseParam_PhaseBehaviour_TRAIN_AND_INFERENCE) {
		DF_NODE_CUDNN_CHECK(
			cudnnBatchNormalizationForwardTraining(
				_cudnnHandle,
				_batchNormMode,
				&one,
				&zero,
				_xDesc,
				_x,
				_yDesc,
				_y,
				_bnScaleBiasMeanVarDesc,
				_bnScale,
				_bnBias,
				0,
				nullptr,
				nullptr,
				_eps,
				_resultSaveMean,
				_resultSaveInvVariance
			));
	//}
}

void BatchNormalization::backward()
{
	if (_inputs[0]->connectedNode()->propagateBack()) {
		DF_NODE_CUDNN_CHECK(
			cudnnBatchNormalizationBackward(
				_cudnnHandle,
				_batchNormMode,
				&one,
				&zero,
				&one,
				&one,
				_xDesc, _x, _yDesc, _dy, _xDesc, _dx, _bnScaleBiasMeanVarDesc, _bnScale, _bnScale, _bnBias, _eps, _resultSaveMean, _resultSaveInvVariance));
	}
}

std::string BatchNormalization::to_cpp() const
{
	return std::string();
}
