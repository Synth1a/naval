#pragma once
#include "..\..\includes.hpp"
#include <comdef.h>

class c_dpi_scale : public singleton<c_dpi_scale> {
public:
	void apply(float amount);
};
