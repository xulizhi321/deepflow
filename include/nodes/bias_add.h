#pragma once

#include "core/node.h"

class DeepFlowDllExport BiasAdd : public Node {
public:
	BiasAdd(deepflow::NodeParam *param);
	int minNumInputs() { return 2; }
	int minNumOutputs() { return 1; }
	void initForward();
	void initBackward();
	void forward();
	void backward();
	std::string to_cpp() const;
	ForwardType forwardType() { return DEPENDS_ON_OUTPUTS; }
	BackwardType backwardType() { return DEPENDS_ON_INPUTS; }
private:
	int _inner_dim = 0;
	int _bias_dim = 0;
	int _sample_dim = 0;
};
