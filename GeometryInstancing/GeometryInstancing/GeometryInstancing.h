//--------------------------------------------------------------------------------------
// File: GeometryInstancing.h
// 
// *************************************************************************************
//
// *************************************************************************************
//
// @author: Andrea Casaccia <acasaccia@gmail.com>
// @date: 15/05/2012
//--------------------------------------------------------------------------------------

#pragma once

#define DEG2RAD( a ) ( a * D3DX_PI / 180.f )
#define MAX_PARTICLES_NUMBER 1000
#define PARTICLE_LIFETIME_MIN 1.0f
#define PARTICLE_LIFETIME_MAX 2.0f
#define PARTICLE_FLOW 10.0f
#define PARTICLE_FORCE_MIN 5.0f
#define PARTICLE_FORCE_MAX 10.0f
#define PARTICLE_TANK -1
#define PARTICLE_GRAVITY -5.0f
#define PARTICLE_FRICTION 2.0f


//--------------------------------------------------------------------------------------
// Dichiarazione strutture
//--------------------------------------------------------------------------------------
struct Vertex
{
	D3DXVECTOR3 Position;
	D3DXVECTOR2 TexCoords;
};

struct VertexWithColor
{
	D3DXVECTOR3 Position;
	D3DXVECTOR4 Color;
	D3DXVECTOR2 TexCoords;
};

struct Instance
{
	D3DXVECTOR4 Color;
	D3DXVECTOR3 Offset;
};