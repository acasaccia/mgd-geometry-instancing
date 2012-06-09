//--------------------------------------------------------------------------------------
// basic_effect.fx
//
// @author: Andrea Casaccia <acasaccia@gmail.com>
// @date: 15/05/2012
//--------------------------------------------------------------------------------------

//Definizione variabili e strutture

matrix World;
matrix View;
matrix Projection;
Texture2D Particle;

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Col : COLOR;
	float2 Tex : TEXCOORD;
};

// Stato Sampling bilineare per texture
SamplerState samplingLinear
{
    // TODO: Definire un filtraggio trilineare
	Filter = MIN_MAG_MIP_LINEAR;
};

// Stato Z-Buffer Abilitato
DepthStencilState EnableDepth{
	 DepthEnable = true; 
	DepthWriteMask = ALL;  // Può essere ALL o ZERO
	DepthFunc = Less;  // Può essere Less Greater Less_Equal ...
	// Setup stencil states 
	StencilEnable = false; 
};

BlendState SrcAlphaBlendingAdd
{
	// TODO:Definire un blending di tipo additivo
	BlendEnable[0] = TRUE;
	SrcBlend = SRC_ALPHA;
	DestBlend = INV_SRC_ALPHA;
	BlendOp = ADD;
	SrcBlendAlpha = ONE;
	DestBlendAlpha = ONE;
	BlendOpAlpha = ADD;
	RenderTargetWriteMask[0] = 0x0F;
};

//--------------------------------------------------------------------------------------
// Simple Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS( float4 Pos : POSITION, float4 Col : COLOR, float2 Tex : TEXCOORD )
{
    VS_OUTPUT output = (VS_OUTPUT)0;    
    output.Pos = mul( Pos, World );
    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection );
	output.Col = Col;
	output.Tex = Tex;
    return output;
}


//--------------------------------------------------------------------------------------
// Instanced Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS_INST( float4 Pos : POSITION, float2 Tex : TEXCOORD0, float4 Col : TEXCOORD1, float4 Offset : TEXCOORD2)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
	output.Pos = mul( Pos, World );

	output.Pos.x += Offset.x;
	output.Pos.y += Offset.y;
	output.Pos.z += Offset.z;

    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection );        
	output.Col = Col;
	output.Tex = Tex;
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( VS_OUTPUT input ) : SV_Target
{
	float4 textureColor;
	float4 finalColor;

    // Sample the pixel color from the texture using the sampler at this texture coordinate location.
    textureColor = Particle.Sample(samplingLinear, input.Tex);

	// Combine the texture color and the particle color to get the final color result.
    finalColor = textureColor * input.Col;

    return finalColor;
}


//--------------------------------------------------------------------------------------
technique11 SimpleRender
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS() ) );  
        SetDepthStencilState(EnableDepth,0);
		SetBlendState(SrcAlphaBlendingAdd, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}

//--------------------------------------------------------------------------------------
technique11 InstancedRender
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS_INST() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS() ) );  
        SetDepthStencilState(EnableDepth,0);
		SetBlendState(SrcAlphaBlendingAdd, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}
