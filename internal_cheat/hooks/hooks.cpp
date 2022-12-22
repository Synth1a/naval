// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "hooks.hpp"

#include <tchar.h>
#include <iostream>
#include <d3d9.h>
#include <dinput.h>

#include "..\cheats\misc\logs.h"
#include "..\cheats\misc\misc.h"
#include "..\cheats\visuals\other_esp.h"
#include "..\cheats\visuals\world_esp.h"

#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")

#include <shlobj.h>
#include <shlwapi.h>
#include <thread>

#include "..\cheats\ui\menu.h"
#include "..\cheats\ui\dpi_scale.h"

#include "..\byte\Bytesa.h"

auto _visible = true;
static auto d3d_init = false;

namespace INIT
{
	HMODULE Dll;
	HWND Window;
	WNDPROC OldWindow;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern IMGUI_IMPL_API LRESULT ImGui_ImplDX9_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace hooks
{
	int rage_weapon = 0;
	int legit_weapon = 0;
	bool menu_open = false;
	bool input_shouldListen = false;

	ButtonCode_t* input_receivedKeyval;

	LRESULT __stdcall Hooked_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		static auto is_down = true;
		static auto is_clicked = false;

		if (GetAsyncKeyState(VK_INSERT))
		{
			is_clicked = false;
			is_down = true;
		}
		else if (!GetAsyncKeyState(VK_INSERT) && is_down)
		{
			is_clicked = true;
			is_down = false;
		}
		else
		{
			is_clicked = false;
			is_down = false;
		}

		if (is_clicked)
		{
			menu_open = !menu_open;

			if (menu_open && g_ctx.available())
			{
				if (g_ctx.globals.current_weapon != -1)
				{
					if (g_cfg.ragebot.enable)
						rage_weapon = g_ctx.globals.current_weapon;
					else if (g_cfg.legitbot.enabled)
						legit_weapon = g_ctx.globals.current_weapon;
				}
			}
		}

		auto pressed_buttons = false;
		auto pressed_menu_key = uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP || uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONUP || uMsg == WM_MOUSEWHEEL;

		if (g_ctx.available() && g_ctx.local()->is_alive() && !pressed_menu_key && !g_ctx.globals.focused_on_input)
			pressed_buttons = true;

		if (!pressed_buttons && d3d_init && menu_open && ImGui_ImplDX9_WndProcHandler(hWnd, uMsg, wParam, lParam) && !input_shouldListen)
			return true;

		if (menu_open && (uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP || uMsg == WM_MOUSEMOVE) && !input_shouldListen)
			return false;

		return CallWindowProc(INIT::OldWindow, hWnd, uMsg, wParam, lParam);
	}

	long __stdcall Hooked_EndScene(IDirect3DDevice9* pDevice)
	{
		static auto original_fn = directx_hook->get_func_address <EndSceneFn>(42);
		return original_fn(pDevice);
	}

	void GUI_Init(IDirect3DDevice9* pDevice)
	{
		ImGui::CreateContext();

		ImGui_ImplWin32_Init(INIT::Window);
		ImGui_ImplDX9_Init(pDevice);

		auto& io = ImGui::GetIO();

		auto& style = ImGui::GetStyle();

		style.WindowMinSize = ImVec2(10, 10);

		ImFontConfig m_config;
		m_config.OversampleH = 1; //or 2 is the same
		m_config.OversampleV = 1;
		m_config.PixelSnapH = 1;

		io.Fonts->AddFontFromMemoryCompressedTTF(museo_compressed_data, museo_compressed_size, 12, NULL, io.Fonts->GetGlyphRangesCyrillic());
		g_ctx.fonts.keybind_font = io.Fonts->AddFontFromMemoryCompressedTTF(museo_compressed_data, museo_compressed_size, 13, NULL, io.Fonts->GetGlyphRangesCyrillic());
		g_ctx.fonts.generic_font = io.Fonts->AddFontFromMemoryCompressedTTF(museo_compressed_data, museo_compressed_size, 12, NULL, io.Fonts->GetGlyphRangesCyrillic());
		g_ctx.fonts.large_generic_font = io.Fonts->AddFontFromMemoryCompressedTTF(museo_compressed_data, museo_compressed_size, 16, NULL, io.Fonts->GetGlyphRangesCyrillic());
		g_ctx.fonts.weapon_icon  = io.Fonts->AddFontFromMemoryCompressedTTF(undefeated_compressed_data, undefeated_compressed_size, 16, &m_config, io.Fonts->GetGlyphRangesCyrillic());
		g_ctx.fonts.tab_icon     = io.Fonts->AddFontFromMemoryTTF((void*)icon_img, sizeof(icon_img), 25, &m_config, io.Fonts->GetGlyphRangesCyrillic());

		ImGui_ImplDX9_CreateDeviceObjects();
		d3d_init = true;
	}

	long __stdcall hooked_present(IDirect3DDevice9* device, RECT* src_rect, RECT* dest_rect, HWND dest_wnd_override, RGNDATA* dirty_region)
	{
		static auto original = directx_hook->get_func_address <PresentFn>(17);
		g_ctx.local((player_t*)m_entitylist()->GetClientEntity(m_engine()->GetLocalPlayer()), true);

		if (!d3d_init)
			GUI_Init(device);

		key_binds::get().update_key_binds();

		IDirect3DVertexDeclaration9* vertex_dec;
		device->GetVertexDeclaration(&vertex_dec);

		IDirect3DVertexShader9* vertex_shader;
		device->GetVertexShader(&vertex_shader);

		g_ctx.globals.draw_device = device;

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();

		switch (g_cfg.menu.dpi_selection)
		{
		case 0:
			g_cfg.menu.dpi_scale = 1.0f;
			break;
		case 1:
			g_cfg.menu.dpi_scale = 1.1f;
			break;
		case 2:
			g_cfg.menu.dpi_scale = 1.2f;
			break;
		case 3:
			g_cfg.menu.dpi_scale = 1.3f;
			break;
		case 4:
			g_cfg.menu.dpi_scale = 1.4f;
			break;
		case 5:
			g_cfg.menu.dpi_scale = 1.5f;
			break;
		case 6:
			g_cfg.menu.dpi_scale = 1.6f;
			break;
		case 7:
			g_cfg.menu.dpi_scale = 1.7f;
			break;
		case 8:
			g_cfg.menu.dpi_scale = 1.8f;
			break;
		case 9:
			g_cfg.menu.dpi_scale = 1.9f;
			break;
		case 10:
			g_cfg.menu.dpi_scale = 2.0f;
			break;
		}

		c_dpi_scale::get().apply(g_cfg.menu.dpi_scale);

		ImGui::NewFrame();

		c_menu::get().draw(menu_open);

		otheresp::get().keybind_list();
		otheresp::get().spectator_list();
		otheresp::get().spread_crosshair();
		otheresp::get().watermark();

		ImGui::EndFrame();
		ImGui::Render();

		ImDrawData* draw_data = ImGui::GetDrawData();
		draw_data->ScaleClipRects(draw_data->FramebufferScale);
		ImGui_ImplDX9_RenderDrawData(draw_data);

		device->SetVertexShader(vertex_shader);
		device->SetVertexDeclaration(vertex_dec);

		return original(device, src_rect, dest_rect, dest_wnd_override, dirty_region);
	}

	long __stdcall Hooked_EndScene_Reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
	{
		static auto ofunc = directx_hook->get_func_address<EndSceneResetFn>(16);

		if (!d3d_init)
			return ofunc(pDevice, pPresentationParameters);

		ImGui_ImplDX9_InvalidateDeviceObjects();

		auto hr = ofunc(pDevice, pPresentationParameters);

		if (SUCCEEDED(hr))
			ImGui_ImplDX9_CreateDeviceObjects();

		return hr;
	}

	DWORD original_getforeignfallbackfontname;
	DWORD original_setupbones;
	DWORD original_doextrabonesprocessing;
	DWORD original_standardblendingrules;
	DWORD original_updateclientsideanimation;
	DWORD original_estimateabsvelocity;
	DWORD original_physicssimulate;
	DWORD original_calcviewmodelbob;
	DWORD original_buildtransformations;
	DWORD original_geteyeangles;
	DWORD original_calcview;
	DWORD original_processinterpolatedlist;
	DWORD original_clmove;
	DWORD original_checkforsequencechange;
	DWORD original_framestagenotify;
	DWORD installed_font;

	vmthook* directx_hook;
	vmthook* client_hook;
	vmthook* clientstate_hook;
	vmthook* engine_hook;
	vmthook* clientmode_hook;
	vmthook* inputinternal_hook;
	vmthook* renderview_hook;
	vmthook* panel_hook;
	vmthook* modelcache_hook;
	vmthook* materialsys_hook;
	vmthook* modelrender_hook;
	vmthook* prediction_hook;
	vmthook* surface_hook;
	vmthook* r_drawmodelstatsoverlay_hook;
	vmthook* sv_cheats_hook;
	vmthook* bspquery_hook;
	vmthook* trace_hook;
	vmthook* filesystem_hook;
	
	C_HookedEvents hooked_events;
}

void __fastcall hooks::hooked_setkeycodestate(void* thisptr, void* edx, ButtonCode_t code, bool bDown)
{
	static auto original_fn = inputinternal_hook->get_func_address <SetKeyCodeState_t>(91);

	if (input_shouldListen && bDown)
	{
		input_shouldListen = false;

		if (input_receivedKeyval)
			*input_receivedKeyval = code;
	}

	return original_fn(thisptr, code, bDown);
}

void __fastcall hooks::hooked_setmousecodestate(void* thisptr, void* edx, ButtonCode_t code, MouseCodeState_t state)
{
	static auto original_fn = inputinternal_hook->get_func_address <SetMouseCodeState_t>(92);

	if (input_shouldListen && state == BUTTON_PRESSED)
	{
		input_shouldListen = false;

		if (input_receivedKeyval)
			*input_receivedKeyval = code;
	}

	return original_fn(thisptr, code, state);
}