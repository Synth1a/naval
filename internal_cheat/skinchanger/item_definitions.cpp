// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

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
* THE SOFTWARE IS PROVIDED "AS IS"), WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
#include "item_definitions.hpp"
// These are std::vectors because else I'd have to write their size in the header or write my own container

// We need these for overriding viewmodels and icons
const game_data::weapon_info* game_data::get_weapon_info(int defindex)
{
    const static std::map<int, weapon_info> info =
    {
        {WEAPON_KNIFE, {crypt_arr("models/weapons/v_knife_default_ct.mdl"), crypt_arr("knife")}},
        {WEAPON_KNIFE_T, {crypt_arr("models/weapons/v_knife_default_t.mdl"), crypt_arr("knife_t")}},
        {WEAPON_KNIFE_BAYONET, {crypt_arr("models/weapons/v_knife_bayonet.mdl"), crypt_arr("bayonet")}},
        {WEAPON_KNIFE_CSS, {crypt_arr("models/weapons/v_knife_css.mdl"), crypt_arr("knife_css")}},
        {WEAPON_KNIFE_SKELETON, {crypt_arr("models/weapons/v_knife_skeleton.mdl"), crypt_arr("knife_skeleton")}},
        {WEAPON_KNIFE_OUTDOOR, {crypt_arr("models/weapons/v_knife_outdoor.mdl"), crypt_arr("knife_outdoor")}},
        {WEAPON_KNIFE_CORD, {crypt_arr("models/weapons/v_knife_cord.mdl"), crypt_arr("knife_cord")}},
        {WEAPON_KNIFE_CANIS, {crypt_arr("models/weapons/v_knife_canis.mdl"), crypt_arr("knife_canis")}},
        {WEAPON_KNIFE_FLIP, {crypt_arr("models/weapons/v_knife_flip.mdl"), crypt_arr("knife_flip")}},
        {WEAPON_KNIFE_GUT, {crypt_arr("models/weapons/v_knife_gut.mdl"), crypt_arr("knife_gut")}},
        {WEAPON_KNIFE_KARAMBIT, {crypt_arr("models/weapons/v_knife_karam.mdl"), crypt_arr("knife_karambit")}},
        {WEAPON_KNIFE_M9_BAYONET, {crypt_arr("models/weapons/v_knife_m9_bay.mdl"), crypt_arr("knife_m9_bayonet")}},
        {WEAPON_KNIFE_TACTICAL, {crypt_arr("models/weapons/v_knife_tactical.mdl"), crypt_arr("knife_tactical")}},
        {WEAPON_KNIFE_FALCHION, {crypt_arr("models/weapons/v_knife_falchion_advanced.mdl"), crypt_arr("knife_falchion")}},
        {WEAPON_KNIFE_SURVIVAL_BOWIE, {crypt_arr("models/weapons/v_knife_survival_bowie.mdl"), crypt_arr("knife_survival_bowie")}},
        {WEAPON_KNIFE_BUTTERFLY, {crypt_arr("models/weapons/v_knife_butterfly.mdl"), crypt_arr("knife_butterfly")}},
        {WEAPON_KNIFE_PUSH, {crypt_arr("models/weapons/v_knife_push.mdl"), crypt_arr("knife_push")}},
        {WEAPON_KNIFE_URSUS, {crypt_arr("models/weapons/v_knife_ursus.mdl"), crypt_arr("knife_ursus")}},
        {WEAPON_KNIFE_GYPSY_JACKKNIFE, {crypt_arr("models/weapons/v_knife_gypsy_jackknife.mdl"), crypt_arr("knife_gypsy_jackknife")}},
        {WEAPON_KNIFE_STILETTO, {crypt_arr("models/weapons/v_knife_stiletto.mdl"), crypt_arr("knife_stiletto")}},
        {WEAPON_KNIFE_WIDOWMAKER,{ crypt_arr("models/weapons/v_knife_widowmaker.mdl"), crypt_arr("knife_widowmaker")}},

        {GLOVE_STUDDED_BLOODHOUND, {crypt_arr("models/weapons/w_models/arms/w_glove_bloodhound.mdl")}},
        {GLOVE_T_SIDE, {crypt_arr("models/weapons/w_models/arms/w_glove_fingerless.mdl")}},
        {GLOVE_CT_SIDE, {crypt_arr("models/weapons/w_models/arms/w_glove_hardknuckle.mdl")}},
        {GLOVE_SPORTY, {crypt_arr("models/weapons/w_models/arms/w_glove_sporty.mdl")}},
        {GLOVE_SLICK, {crypt_arr("models/weapons/w_models/arms/w_glove_slick.mdl")}},
        {GLOVE_LEATHER_WRAP, {crypt_arr("models/weapons/w_models/arms/w_glove_handwrap_leathery.mdl")}},
        {GLOVE_MOTORCYCLE, {crypt_arr("models/weapons/w_models/arms/w_glove_motorcycle.mdl")}},
        {GLOVE_SPECIALIST, {crypt_arr("models/weapons/w_models/arms/w_glove_specialist.mdl")}},
        {GLOVE_HYDRA, {crypt_arr("models/weapons/w_models/arms/w_glove_bloodhound_hydra.mdl")}}
    };

    const auto entry = info.find(defindex);
    return entry == end(info) ? nullptr : &entry->second;
}
