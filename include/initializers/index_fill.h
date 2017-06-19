#pragma once

#include "core/export.h"
#include "core/initializer.h"

class DeepFlowDllExport IndexFill : public Initializer {
public:
	IndexFill(deepflow::InitParam *param);
	void apply(Variable *variable);
	void init() {}
	std::string to_cpp() const;
};