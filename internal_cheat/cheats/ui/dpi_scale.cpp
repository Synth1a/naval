// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "dpi_scale.h"

void c_dpi_scale::apply(float amount)
{
	auto& io = ImGui::GetIO();

	io.DisplayFramebufferScale = ImVec2(amount, amount);
	io.DisplaySize.x /= amount;
	io.DisplaySize.y /= amount;

	if (io.MousePos.x != -FLT_MAX && io.MousePos.y != -FLT_MAX)
	{
		io.MousePos.x /= amount;
		io.MousePos.y /= amount;
	}
}