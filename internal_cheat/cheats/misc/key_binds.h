#pragma once

#include <unordered_map>
#include "..\..\sdk\interfaces\IInputSystem.hpp"
#include "..\..\utils\singleton.h"

enum key_bind_mode
{
	HOLD,
	TOGGLE
};

struct key_bind
{
	ButtonCode_t key = KEY_NONE;
	key_bind_mode mode = HOLD;
	bool holding = false;
	bool active = false;

	key_bind(key_bind_mode mode = HOLD)
	{
		this->mode = mode;
	}

	key_bind(ButtonCode_t key, key_bind_mode mode)
	{
		this->key = key;
		this->mode = mode;
	}
};

class key_binds : public singleton <key_binds>
{
	void update_key_bind(key_bind* key_bind, int key_bind_id);
public:
	void initialize_key_binds();
	void update_key_binds(); 
};