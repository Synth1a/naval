/* This file is part of nSkinz by namazso, licensed under the MIT license:
*
* MIT License
*
* Copyright (c) namazso 2018
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
#pragma once
#include <map>
#include <vector>

#include "..\sdk\misc\Enums.hpp"
#include "..\utils\crypt_str.h"

static auto is_knife(const int i) -> bool
{
    return (i >= WEAPON_KNIFE_BAYONET && i < GLOVE_STUDDED_BLOODHOUND) || i == WEAPON_KNIFE_T || i == WEAPON_KNIFE;
}

//extern const std::map<size_t, weapon_info> k_weapon_info;
namespace game_data
{
    // Stupid MSVC requires separate constexpr constructors for any initialization
    struct weapon_info
    {
        constexpr weapon_info(const char* model, const char* icon = nullptr) :
            model(model),
            icon(icon)
        {}

        const char* model;
        const char* icon;
    };

    struct weapon_name
    {
        constexpr weapon_name(const int definition_index, const char* name) :
            definition_index(definition_index),
            name(name)
        {}

        int definition_index = 0;
        const char* name = nullptr;
    };

    struct quality_name
    {
        constexpr quality_name(const int index, const char* name) :
            index(index),
            name(name)
        {}

        int index = 0;
        const char* name = nullptr;
    };

    const weapon_info* get_weapon_info(int defindex);

    const weapon_name knife_names[]{
        {0, crypt_arr("Default Knife")},
        {WEAPON_KNIFE_BAYONET, crypt_arr("Bayonet")},
        {WEAPON_KNIFE_CSS, crypt_arr("Classic Knife")},
        {WEAPON_KNIFE_SKELETON, crypt_arr("Skeleton Knife")},
        {WEAPON_KNIFE_OUTDOOR, crypt_arr("Nomad Knife")},
        {WEAPON_KNIFE_CORD, crypt_arr("Paracord Knife")},
        {WEAPON_KNIFE_CANIS, crypt_arr("Survival Knife")},
        {WEAPON_KNIFE_FLIP, crypt_arr("Flip Knife")},
        {WEAPON_KNIFE_GUT, crypt_arr("Gut Knife")},
        {WEAPON_KNIFE_KARAMBIT, crypt_arr("Karambit")},
        {WEAPON_KNIFE_M9_BAYONET, crypt_arr("M9 Bayonet")},
        {WEAPON_KNIFE_TACTICAL, crypt_arr("Huntsman Knife")},
        {WEAPON_KNIFE_FALCHION, crypt_arr("Falchion Knife")},
        {WEAPON_KNIFE_SURVIVAL_BOWIE, crypt_arr("Bowie Knife")},
        {WEAPON_KNIFE_BUTTERFLY, crypt_arr("Butterfly Knife")},
        {WEAPON_KNIFE_PUSH, crypt_arr("Shadow Daggers")},
        {WEAPON_KNIFE_URSUS, crypt_arr("Ursus Knife")},
        {WEAPON_KNIFE_GYPSY_JACKKNIFE, crypt_arr("Navaja Knife")},
        {WEAPON_KNIFE_STILETTO, crypt_arr("Stiletto Knife")},
        {WEAPON_KNIFE_WIDOWMAKER, crypt_arr("Talon Knife")}
    };

    const weapon_name glove_names[]{
        {0, crypt_arr("Default Glove")},
        {GLOVE_STUDDED_BLOODHOUND, crypt_arr("Bloodhound")},
        {GLOVE_T_SIDE, crypt_arr("Default (Terrorists)")},
        {GLOVE_CT_SIDE, crypt_arr("Default (Counter-Terrorists)")},
        {GLOVE_SPORTY, crypt_arr("Sporty")},
        {GLOVE_SLICK, crypt_arr("Slick")},
        {GLOVE_LEATHER_WRAP, crypt_arr("Handwrap")},
        {GLOVE_MOTORCYCLE, crypt_arr("Motorcycle")},
        {GLOVE_SPECIALIST, crypt_arr("Specialist")},
        {GLOVE_HYDRA, crypt_arr("Hydra")}
    };

    const weapon_name weapon_names[]{
        {WEAPON_KNIFE, crypt_arr("Knife")},
        {GLOVE_T_SIDE, crypt_arr("Glove")},
        {WEAPON_AK47, crypt_arr("AK-47")},
        {WEAPON_AUG, crypt_arr("AUG")},
        {WEAPON_AWP, crypt_arr("AWP")},
        {WEAPON_CZ75A, crypt_arr("CZ75 Auto")},
        {WEAPON_DEAGLE, crypt_arr("Desert Eagle")},
        {WEAPON_ELITE, crypt_arr("Dual Berettas")},
        {WEAPON_FAMAS, crypt_arr("FAMAS")},
        {WEAPON_FIVESEVEN, crypt_arr("Five-SeveN")},
        {WEAPON_G3SG1, crypt_arr("G3SG1")},
        {WEAPON_GALILAR, crypt_arr("Galil AR")},
        {WEAPON_GLOCK, crypt_arr("Glock-18")},
        {WEAPON_M249, crypt_arr("M249")},
        {WEAPON_M4A1_SILENCER, crypt_arr("M4A1-S")},
        {WEAPON_M4A1, crypt_arr("M4A4")},
        {WEAPON_MAC10, crypt_arr("MAC-10")},
        {WEAPON_MAG7, crypt_arr("MAG-7")},
        {WEAPON_MP5SD, crypt_arr("MP5-SD")},
        {WEAPON_MP7, crypt_arr("MP7")},
        {WEAPON_MP9, crypt_arr("MP9")},
        {WEAPON_NEGEV, crypt_arr("Negev")},
        {WEAPON_NOVA, crypt_arr("Nova")},
        {WEAPON_HKP2000, crypt_arr("P2000")},
        {WEAPON_P250, crypt_arr("P250")},
        {WEAPON_P90, crypt_arr("P90")},
        {WEAPON_BIZON, crypt_arr("PP-Bizon")},
        {WEAPON_REVOLVER, crypt_arr("R8 Revolver")},
        {WEAPON_SAWEDOFF, crypt_arr("Sawed-Off")},
        {WEAPON_SCAR20, crypt_arr("SCAR-20")},
        {WEAPON_SSG08, crypt_arr("SSG 08")},
        {WEAPON_SG553, crypt_arr("SG 553")},
        {WEAPON_TEC9, crypt_arr("Tec-9")},
        {WEAPON_UMP45, crypt_arr("UMP-45")},
        {WEAPON_USP_SILENCER, crypt_arr("USP-S")},
        {WEAPON_XM1014, crypt_arr("XM1014")},
    };

    const quality_name quality_names[]{
        {0, crypt_arr("Default Quality")},
        {1, crypt_arr("Genuine")},
        {2, crypt_arr("Vintage")},
        {3, crypt_arr("Unusual")},
        {5, crypt_arr("Community")},
        {6, crypt_arr("Developer")},
        {7, crypt_arr("Self-Made")},
        {8, crypt_arr("Customized")},
        {9, crypt_arr("Strange")},
        {10, crypt_arr("Completed")},
        {12, crypt_arr("Tournament")}
    };
}
