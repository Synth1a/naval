#include "..\..\includes.hpp"
#define M_RADPI 57.295779513082f

// Define rough directions
enum directions {
	FORWARDS = 0,
	BACKWARDS = 180,
	LEFT = 90,
	RIGHT = -90
};

class airstrafe : public singleton <airstrafe>
{
private:
	bool is_bhopping;
	float calculated_direction;
	bool in_transition;
	float true_direction;
	float wish_direction;
	float step;
	float rough_direction;

public:
	void create_move(CUserCmd* m_pcmd, float& wish_yaw);
};