// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "spammers.h"
#include "..\misc\prediction_system.h"
#include "..\networking\networking.h"

void spammers::clan_tag()
{
	auto apply = [](const char* tag) -> void
	{
		using Fn = int(__fastcall*)(const char*, const char*);
		static auto fn = reinterpret_cast<Fn>(g_ctx.addresses.set_clantag);

		fn(tag, tag);
	};

	static auto removed = false;

	if (!g_cfg.misc.clantag_spammer && !removed)
	{
		removed = true;
		apply(crypt_str(""));
		return;
	}

	if (g_cfg.misc.clantag_spammer)
	{
		static auto time = 1;

		auto ticks = TIME_TO_TICKS(networking::get().flow_outgoing) + (float)m_globals()->m_tickcount; //-V807
		auto intervals = 0.4f / m_globals()->m_intervalpertick;

		auto main_time = (int)(ticks / intervals) % 11;

		if (main_time != time && m_clientstate()->iChokedCommands == 0)
		{
			auto tag = crypt_str("");

			switch (main_time)
			{
			case 0:
				tag = crypt_str(" ");
				break;
			case 1:
				tag = crypt_str("n ");
				break;
			case 2:
				tag = crypt_str("na ");
				break;
			case 3:
				tag = crypt_str("nav ");
				break;
			case 4:
				tag = crypt_str("nava ");
				break;
			case 5:
				tag = crypt_str("naval ");
				break;
			case 6:
				tag = crypt_str("nava ");
				break;
			case 7:
				tag = crypt_str("nav ");
				break;
			case 8:
				tag = crypt_str("na ");
				break;
			case 9:
				tag = crypt_str("n ");
				break;
			case 10:
				tag = crypt_str(" ");
				break;
			}

			apply(tag);
			time = main_time;
		}

		removed = false;
	}
}