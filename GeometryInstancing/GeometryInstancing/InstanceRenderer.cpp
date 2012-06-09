//--------------------------------------------------------------------------------------
// File: InstanceRenderer.cpp
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

#include "InstanceRenderer.h"

InstanceRenderer::InstanceRenderer(	ID3D11InputLayout* iInstanceLayout,
									ID3DX11EffectTechnique* iTechnique,
									ID3D11Buffer* iVertexBuffer,
									ID3D11Buffer* iInstanceBuffer) {
	mInstanceLayout = iInstanceLayout;
	mTechnique = iTechnique;
	mVertexBuffer = iVertexBuffer;
	mInstanceBuffer = iInstanceBuffer;
}

// Non implementata al momento (usiamo il blending additivo hardcoded nello shader)
void InstanceRenderer::setBlending(SPK::BlendingMode blendMode ) {}

void InstanceRenderer::render(const SPK::Group &group ){
	
	size_t particlesNumber = group.getNbParticles();
	if (!particlesNumber) return;

	Vertex vertices[4];
	Instance instances[MAX_PARTICLES_NUMBER];

	// Struttura base per il rendering della particella, tutte le istanze sono
	// posizionate a partire da questa
	vertices[0].Position = D3DXVECTOR3( - 0.1f, - 0.1f, 0.0f );
	vertices[0].TexCoords = D3DXVECTOR2(0.0f,0.0f);

	vertices[1].Position = D3DXVECTOR3( - 0.1f, + 0.1f, 0.0f );
	vertices[1].TexCoords = D3DXVECTOR2(0.0f,1.0f);

	vertices[2].Position = D3DXVECTOR3( + 0.1f, - 0.1f, 0.0f );
	vertices[2].TexCoords = D3DXVECTOR2(1.0f,0.0f);

	vertices[3].Position = D3DXVECTOR3( + 0.1f, + 0.1f, 0.0f );
	vertices[3].TexCoords = D3DXVECTOR2(1.0f,1.0f);

	SPK::Vector3D position;
	float alpha;
	D3DXVECTOR4 color;

	for (size_t c=0; c<particlesNumber; c++) {
		position = group.getParticle(c).position();
		alpha = (group.getParticle(c).getLifeLeft() / PARTICLE_LIFETIME_MAX);
		color = D3DXVECTOR4( 1.0f, 1.0f, 1.0f, alpha );

		instances[c].Offset = D3DXVECTOR3( position.x, position.y, position.z );
		instances[c].Color = color;
	}

	ID3D11DeviceContext* context = DXUTGetD3D11DeviceContext();
	D3D11_MAPPED_SUBRESOURCE ms;

	context->Map(mVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
	memcpy(ms.pData, &vertices[0], sizeof(Vertex) * 4);
	context->Unmap(mVertexBuffer, 0);

	context->Map(mInstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
	memcpy(ms.pData, &instances[0], sizeof(Instance) * particlesNumber );
	context->Unmap(mInstanceBuffer, 0);

	UINT offsets[2] = {0,0};

	ID3D11Buffer* buffers[2];
	buffers[0] = mVertexBuffer;
	buffers[1] = mInstanceBuffer;

	UINT strides[2] = { sizeof(Vertex), sizeof(Instance) };

	// Disegna usando instancing
	context->IASetVertexBuffers( 0, 2, buffers, strides, offsets );
	context->IASetInputLayout(mInstanceLayout);
	context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

	// Rendering della primitiva
	D3DX11_TECHNIQUE_DESC techDesc;
	mTechnique->GetDesc( &techDesc );

	// Otteniamo il descrittore della tecnica ed eseguiamo i vari passaggi 
	for( UINT p = 0; p < techDesc.Passes; ++p )
	{
		mTechnique->GetPassByIndex( p )->Apply(0, context);

		// Draw effettua il rendering.
		// Prende in input la dimensione e la posizione iniziale di vertici da caricare rispetto al buffer attivo 
		context->DrawInstanced( 4, particlesNumber, 0, 0 );
	}

}