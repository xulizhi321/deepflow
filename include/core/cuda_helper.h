#pragma once

#include "core/export.h"
#include "core/common_cu.h"

#include <string>
#include <iostream>

#define DF_CUDNN_CHECK(XX) { cudnnStatus_t status;	LOG_IF(FATAL, (status = XX) != 0) << "[FAILED] " <<  #XX << " - " << cudnnGetErrorString(status); }
#define DF_CUDA_CHECK(XX) { cudaError_t status; LOG_IF(FATAL, (status = XX) != 0 ) << "[FAILED] " #XX << " - " << cudaGetErrorString(status); }
#define DF_KERNEL_CHECK() LOG_IF(FATAL, cudaPeekAtLastError() != 0);

class DeepFlowDllExport CudaHelper {
public:		
	static void setOptimalThreadsPerBlock();
	static size_t numOfBlocks(const size_t &size);
	static int maxThreadsPerBlock;
protected:
	static std::vector<cudaDeviceProp> _cudaDeviceProps;	
};