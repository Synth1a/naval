//========= Copyright   1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#pragma once
#include "..\math\VMatrix.hpp"

class CBoneAccessor
{
public:
	inline matrix3x4_t* GetBoneArrayForWrite(void) const
	{
		return m_pBones;
	}

	inline void SetBoneArrayForWrite(matrix3x4_t* bonearray)
	{
		m_pBones = bonearray;
	}
	alignas(16) matrix3x4_t* m_pBones;

	int m_ReadableBones;		// Which bones can be read.
	int m_WritableBones;		// Which bones can be written.
};