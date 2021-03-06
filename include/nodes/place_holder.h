#pragma once


#include "core/export.h"
#include "core/node.h"

#include "proto/deepflow.pb.h"

class DeepFlowDllExport PlaceHolder : public Node {
public:
	PlaceHolder(deepflow::NodeParam *param);
	void init();	
	void forward();
	void backward();	
	int minNumInputs() { return 0; }
	int minNumOutputs() { return 1; }
	std::string op_name() const override { return "place_holder"; }
	std::string to_cpp() const;
	void set_dimentions(std::array<int, 4> dims);
};
