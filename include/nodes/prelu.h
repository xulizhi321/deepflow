#pragma once

#include "core/node.h"

class DeepFlowDllExport PRelu : public Node {
public:
	PRelu(deepflow::NodeParam *param);
	int minNumInputs() { return 2; }
	int minNumOutputs() { return 1; }
	std::string op_name() const override { return "prelu"; }
	void init();
	void forward();
	void backward();
	std::string to_cpp() const;
};