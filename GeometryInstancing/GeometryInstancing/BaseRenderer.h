//--------------------------------------------------------------------------------------
// File: BaseRenderer.h
// 
// *************************************************************************************
// Gestisce il rendering del particle system con Direct3D 11.
// Ciascuna particella è renderizzata con un TRIANGLE_STRIP.
// Cicla sul pool di particelle e esegue una draw per ciasuna di esse.
// Gestisce il blending additivo delle particelle (l'engine ci fornisce l'ordinamento).
// *************************************************************************************
//
// @author: Andrea Casaccia <acasaccia@gmail.com>
// @date: 15/05/2012
//--------------------------------------------------------------------------------------

#pragma once

#include "SPK.h"

#include "DXUT.h"
#include "DXUTmisc.h"
#include "DXUTgui.h"
#include "SDKmisc.h"
#include "d3dx11effect.h"

#include "GeometryInstancing.h"

class BaseRenderer : public SPK::Renderer {
public:
	SPK_IMPLEMENT_REGISTERABLE(BaseRenderer);
	BaseRenderer( ID3D11InputLayout* iVertexLayout,
				  ID3DX11EffectTechnique* iTechnique,
				  ID3D11Buffer* iVertexBuffer );
	void setBlending(SPK::BlendingMode blendMode);
	void render(const SPK::Group &group);
private:
	ID3D11InputLayout* mVertexLayout;
	ID3DX11EffectTechnique* mTechnique;
	ID3D11Buffer* mVertexBuffer;
};