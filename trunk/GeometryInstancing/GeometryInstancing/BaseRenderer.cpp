//--------------------------------------------------------------------------------------
// File: BaseRenderer.cpp
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

#include "BaseRenderer.h"

BaseRenderer::BaseRenderer(	ID3D11InputLayout* iVertexLayout,
							ID3DX11EffectTechnique* iTechnique,
							ID3D11Buffer* iVertexBuffer ) {
	mVertexLayout = iVertexLayout;
	mTechnique = iTechnique;
	mVertexBuffer = iVertexBuffer;
}

// Non implementata al momento (usiamo il blending additivo hardcoded nello shader)
void BaseRenderer::setBlending(SPK::BlendingMode blendMode ) {}

void BaseRenderer::render(const SPK::Group &group ){

	size_t particlesNumber = group.getNbParticles();
	if (!particlesNumber) return;

	VertexWithColor vertices[MAX_PARTICLES_NUMBER*4];

	SPK::Vector3D position;
	float alpha;
	D3DXVECTOR4 color;

	for (size_t c=0; c<particlesNumber; c++) {
		position = group.getParticle(c).position();
		alpha = (group.getParticle(c).getLifeLeft() / PARTICLE_LIFETIME_MAX);
		color = D3DXVECTOR4( 1.0f, 1.0f, 1.0f, alpha );

		vertices[c*4].Position = D3DXVECTOR3( position.x - 0.1f, position.y - 0.1f, position.z );
		vertices[c*4].Color = color;
		vertices[c*4].TexCoords = D3DXVECTOR2(0.0f,0.0f);

		vertices[c*4+1].Position = D3DXVECTOR3( position.x - 0.1f, position.y + 0.1f, position.z );
		vertices[c*4+1].Color = color;
		vertices[c*4+1].TexCoords = D3DXVECTOR2(0.0f,1.0f);

		vertices[c*4+2].Position = D3DXVECTOR3( position.x + 0.1f, position.y - 0.1f, position.z );
		vertices[c*4+2].Color = color;
		vertices[c*4+2].TexCoords = D3DXVECTOR2(1.0f,0.0f);

		vertices[c*4+3].Position = D3DXVECTOR3( position.x + 0.1f, position.y + 0.1f, position.z );
		vertices[c*4+3].Color = color;
		vertices[c*4+3].TexCoords = D3DXVECTOR2(1.0f,1.0f);
	}

	ID3D11DeviceContext* context = DXUTGetD3D11DeviceContext();
	D3D11_MAPPED_SUBRESOURCE ms;

	context->Map(mVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
	memcpy(ms.pData, &vertices[0], sizeof(VertexWithColor) * particlesNumber * 4);
	context->Unmap(mVertexBuffer, 0);

	for (size_t c=0; c<particlesNumber; c++) {

		// Settaggio Layout di input 
		context->IASetInputLayout( mVertexLayout );

		UINT stride = sizeof(VertexWithColor);
		UINT offset = 0;
		context->IASetVertexBuffers( 0, 1, &mVertexBuffer, &stride, &offset );
		// Settiamo il tipo di primitiva utilizzata
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
			context->Draw( 4, c*4 );
		}

	}

}