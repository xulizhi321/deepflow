#pragma once

#include "core/export.h"
#include "core/initializer.h"

class DeepFlowDllExport Step : public Initializer {
public:
	Step(const deepflow::InitParam &_block_param);
	void apply(Variable *variable);
	void init();
	std::string to_cpp() const;
};