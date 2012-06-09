//--------------------------------------------------------------------------------------
// File: InstanceRenderer.h
// 
// *************************************************************************************
// Gestisce il rendering del particle system con Direct3D 11.
// L'intero sistema è renderizzato con una sola draw call.
// Cicla sul pool di particelle e prepara un array con i dati delle istanze.
// Ciascuna istanza è caratterizzata da un colore - offset di posizione, rispetto a una
// struttura di base memorizzata nel vertex buffer.
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

class InstanceRenderer : public SPK::Renderer {
public:
	SPK_IMPLEMENT_REGISTERABLE(InstanceRenderer);
	InstanceRenderer( ID3D11InputLayout* iInstanceLayout,
					  ID3DX11EffectTechnique* iTechnique,
					  ID3D11Buffer* iVertexBuffer,
					  ID3D11Buffer* iInstanceBuffer);
	void setBlending(SPK::BlendingMode blendMode);
	void render(const SPK::Group &group);
private:
	ID3D11InputLayout* mInstanceLayout;
	ID3DX11EffectTechnique* mTechnique;
	ID3D11Buffer* mVertexBuffer;
	ID3D11Buffer* mInstanceBuffer;
};