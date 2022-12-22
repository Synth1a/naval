#pragma once

#include "..\..\includes.hpp"
#include "..\lagcompensation\animation_system.h"

struct skeleton_info_t {
    ModelRenderInfo_t render_info;
    DrawModelState_t state;

    matrix3x4_t bones[128];
    matrix3x4_t cur_bones;
    float time;

    player_t* player;
};

class hitchams : public singleton <hitchams>
{
private:
	std::vector<skeleton_info_t> skeleton_info;
public:
	void add_matrix(lagcompensation::LagRecord_t* record);
	void draw_hit_matrix();
};