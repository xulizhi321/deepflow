#pragma once


#include "core/solver.h"

class DeepFlowDllExport AdaDeltaSolver : public Solver {
public:
	AdaDeltaSolver(const deepflow::SolverParam &_block_param);
	void apply(std::shared_ptr<Variable> var);
	void init(std::shared_ptr<Variable> var);
	std::string to_cpp() const;
private:
	float * _h1 = NULL;
	float * _h2 = NULL;	
	deepflow::AdaDeltaSolverParam _my_param;
};
