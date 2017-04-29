#pragma once

#include "nodes/variable.h"

#include "proto/deepflow.pb.h"

class Variable;

class DeepFlowDllExport Solver : public CudaHelper {
public:
	Solver(const SolverParam &param);
	virtual void apply(std::shared_ptr<Variable> var) = 0;
	virtual void init(std::shared_ptr<Variable> var) = 0;
	const SolverParam& param() const;
	const std::string name() const;
protected:	
	SolverParam _param;
	bool _initialized = false;
};