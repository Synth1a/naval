// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "hitchams.h"
#include "..\..\hooks\hooks.hpp"

void hitchams::add_matrix(lagcompensation::LagRecord_t* record)
{
	static int m_nSkin = util::find_in_datamap(record->m_pEntity->GetPredDescMap(), crypt_str("m_nSkin"));
	static int m_nBody = util::find_in_datamap(record->m_pEntity->GetPredDescMap(), crypt_str("m_nBody"));

	IClientRenderable* pRenderable = record->m_pEntity->GetClientRenderable();
	if (!pRenderable)
		return;

	const model_t* pModel = record->m_pEntity->GetModel();
	if (!pModel)
		return;

	auto model = record->m_pEntity->GetModel();
	if (!model)
		return;

	auto hdr = m_modelinfo()->GetStudioModel(model);
	if (!hdr)
		return;

	auto& info = skeleton_info.emplace_back();

	info.player = record->m_pEntity;
	auto bones = record->m_pMatrix.main;
	std::memcpy(info.bones, bones, record->m_pEntity->m_CachedBoneData().Count() * sizeof(matrix3x4_t));

	info.time = m_globals()->m_realtime + g_cfg.player.shot_record_duration;

	info.render_info.origin = record->m_pEntity->m_vecOrigin();
	info.render_info.angles = record->m_pEntity->GetAbsAngles();

	info.state.m_pStudioHdr = hdr;
	info.state.m_pStudioHWData = m_modelcache()->GetHardwareData(model->studio);
	info.state.m_pRenderable = pRenderable;
	info.state.m_drawFlags = 0;

	info.render_info.pRenderable = pRenderable;
	info.render_info.pModel = model;
	info.render_info.pLightingOffset = nullptr;
	info.render_info.pLightingOrigin = nullptr;
	info.render_info.hitboxset = record->m_pEntity->m_nHitboxSet();
	info.render_info.skin = (int)(uintptr_t(info.player) + m_nSkin);
	info.render_info.body = (int)(uintptr_t(info.player) + m_nBody);
	info.render_info.entity_index = record->m_pEntity->EntIndex();
	info.render_info.instance = call_virtual<ModelInstanceHandle_t(__thiscall*)(void*) >(pRenderable, 30)(pRenderable);
	info.render_info.flags = 0x1;

	info.state.m_decals = 0;
	info.state.m_lod = 0;

	info.render_info.pModelToWorld = &info.cur_bones;
	info.state.m_pModelToWorld = &info.cur_bones;

	info.cur_bones.AngleMatrix(info.render_info.angles, info.render_info.origin);
}

using DrawModelExecute_t = void(__thiscall*)(IVModelRender*, IMatRenderContext*, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4_t*);

void hitchams::draw_hit_matrix()
{
	static auto original_fn = hooks::modelrender_hook->get_func_address <DrawModelExecute_t>(21);

	if (!g_ctx.available())
	{
		if (!skeleton_info.empty())
			skeleton_info.clear();

		return;
	}

	if (!g_cfg.player.shot_record_chams)
		return;

	auto ctx = m_materialsystem()->GetRenderContext();

	if (!ctx)
		return;

	for (int i = 0; i < skeleton_info.size(); ++i)
	{
		if (m_globals()->m_realtime - skeleton_info[i].time < 0.0f && m_entitylist()->GetClientEntity(skeleton_info[i].render_info.entity_index))
			continue;

		skeleton_info.erase(skeleton_info.begin() + i);
	}

	static auto material = g_ctx.chams.glow;
	for (auto it = skeleton_info.begin(); it != skeleton_info.end(); ++it) {
		if (!it->player || !it->state.m_pModelToWorld || !it->state.m_pRenderable || !it->state.m_pStudioHdr || !it->state.m_pStudioHWData ||
			!it->render_info.pRenderable || !it->render_info.pModelToWorld || !it->render_info.pModel || !m_entitylist()->GetClientEntity(it->render_info.entity_index)) {
			continue;
		}

		auto alpha = ((float)g_cfg.player.shot_record_color.a() / 255.0f);
		auto delta = m_globals()->m_realtime - it->time;
		if (delta > 0.0f) {
			alpha -= delta;
			if (delta > ((float)g_cfg.player.shot_record_color.a() / 255.0f)) {
				it = skeleton_info.erase(it);
				continue;
			}
		}

		auto alpha_c = ((float)g_cfg.player.shot_record_color.a() / 255.0f);

		float normal_color[3] =
		{
			g_cfg.player.shot_record_color[0] / 255.0f,
			g_cfg.player.shot_record_color[1] / 255.0f,
			g_cfg.player.shot_record_color[2] / 255.0f
		};

		m_renderview()->SetBlend(alpha_c * alpha);
		util::color_modulate(normal_color, material);

		material->IncrementReferenceCount();
		material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);

		m_modelrender()->ForcedMaterialOverride(material);
		original_fn(m_modelrender(), ctx, it->state, it->render_info, it->bones);
		m_modelrender()->ForcedMaterialOverride(nullptr);
	}
}