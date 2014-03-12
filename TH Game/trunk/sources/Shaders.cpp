#include <string>

extern std::string const DefaultShader = 
	"																\n\
	//Default shaders												\n\
																	\n\
	sampler texsampler;												\n\
	uniform float4x4 WorldViewProjMat;								\n\
	uniform float4x4 WorldViewMat;									\n\
	uniform float4 FogRGBA;											\n\
	uniform float2 FogNF;											\n\
																	\n\
	struct Vertex3D													\n\
	{																\n\
		float4 vposition : POSITION0;								\n\
		float2 tposition : TEXCOORD0;								\n\
		float4 color : COLOR0;										\n\
		float4 fogcol : FOG1;										\n\
		float fogfac : FOG0;										\n\
	};																\n\
	void vs_main3D( inout Vertex3D data )							\n\
	{																\n\
		float4 vert_viewspace = mul( data.vposition, WorldViewMat );\n\
		data.fogfac = saturate((FogNF.y - vert_viewspace.z ) / (FogNF.y - FogNF.x));\n\
		data.vposition = mul( data.vposition, WorldViewProjMat );	\n\
		/*shortcut*///data.fogfac = saturate((FogNF.y - data.vposition.z ) / (FogNF.y - FogNF.x));		\n\
		data.fogcol = FogRGBA;										\n\
	}																\n\
																	\n\
	float4 ps_main3D( in Vertex3D data ) : COLOR					\n\
	{																\n\
		float4 finalcolor =											\n\
			tex2D( texsampler, data.tposition.xy );	\n\
			finalcolor = data.color * (data.fogfac * finalcolor +  (1.0 - data.fogfac) * data.fogcol);\n\
		return finalcolor;											\n\
	}																\n\
																	\n\
	void vs_main2D( inout Vertex3D data )							\n\
	{																\n\
		data.vposition = mul( data.vposition, WorldViewProjMat );	\n\
	}																\n\
																	\n\
	float4 ps_main2D( in Vertex3D data ) : COLOR					\n\
	{																\n\
		return tex2D( texsampler, data.tposition.xy ) * data.color;				\n\
	}																\n\
	";