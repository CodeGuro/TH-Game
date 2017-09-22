#include <string>

extern std::string const DefaultShader = R"string_border(

	//Default shaders
	
	sampler texsampler;
	uniform float4x4 WorldViewProjMat;
	uniform float4x4 WorldViewMat;
	uniform float4 FogRGBA;
	uniform float2 FogNF;
	
	struct Vertex3D
	{
		float4 vposition : POSITION0;
		float2 tposition : TEXCOORD0;
		float4 color : COLOR0;
		float4 fogcol : FOG1;
		float fogfac : FOG0;
	};
	
	void vs_main3D( inout Vertex3D data )
	{
		float4 vert_viewspace = mul( data.vposition, WorldViewMat );
		data.fogfac = saturate((FogNF.y - vert_viewspace.z ) / (FogNF.y - FogNF.x));
		data.vposition = mul( data.vposition, WorldViewProjMat );
		/*shortcut*///data.fogfac = saturate((FogNF.y - data.vposition.z ) / (FogNF.y - FogNF.x));
		data.fogcol = FogRGBA;
	}
	
	float4 ps_main3D( in Vertex3D data ) : COLOR
	{
		float4 finalcolor =
			tex2D( texsampler, data.tposition.xy );
			finalcolor.rgb = data.color.rgb * (data.fogfac * finalcolor.rgb +  (1.0 - data.fogfac) * data.fogcol.rgb);
		return finalcolor;
	}
	
	void vs_main2D( inout Vertex3D data )
	{
		data.vposition = mul( data.vposition, WorldViewProjMat );
	}
	
	float4 ps_main2D( in Vertex3D data ) : COLOR
	{
		return tex2D( texsampler, data.tposition.xy ) * data.color;
	}

)string_border";