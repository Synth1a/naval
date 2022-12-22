#include "..\..\includes.hpp"

class misc : public singleton <misc> 
{
public:
	struct {
		bool in_forward;
		bool in_back;
		bool in_right;
		bool in_left;
		bool in_moveright;
		bool in_moveleft;
		bool in_jump;
	} movement;

	penetration::PenetrationOutput_t pen_data;

	std::string get_config_direction();
	void load_config();
	void save_config();
	void remove_config();
	void add_config();

	void NoDuck(CUserCmd* cmd);
	void init_penetration();
	void ChatSpamer();
	void unlockhiddenconvars();
	void mouse_delta_fix(CUserCmd* m_pcmd);
	void AutoCrouch(CUserCmd* cmd);
	void SlideWalk(CUserCmd* cmd);
	void remove_player_patches();
	void automatic_peek(CUserCmd* cmd, float& wish_yaw);
	void ViewModel();
	void FullBright();
	void PovArrows(player_t* e, Color color);
	void NightmodeFix();
	void buybot();
	void zeus_range();
	void desync_arrows();
	void fast_ladder(CUserCmd* m_pcmd);
	void ragdolls();
	void rank_reveal();
	void fast_stop(CUserCmd* m_pcmd, float wish_yaw);
};