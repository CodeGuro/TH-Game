#include <string>

extern std::string const DefaultShader = 
	"																\n\
	//Default shaders												\n\
																	\n\
	uniform float4x4 WorldViewProjMat;								\n\
	uniform float4x4 WorldViewMat;									\n\
	sampler texsampler;												\n\
																	\n\
	struct Vertex3D													\n\
	{																\n\
		float4 vposition : POSITION0;								\n\
		float2 tposition : TEXCOORD0;								\n\
		float4 color : COLOR0;										\n\
		float fogfac : FOG;											\n\
	};																\n\
	void vs_main( inout Vertex3D data )								\n\
	{																\n\
		data.vposition = mul( data.vposition, WorldViewProjMat );	\n\
		float4 vert_viewspace = mul( data.vposition, WorldViewMat );\n\
		data.fogfac = saturate((30 - vert_viewspace.z ) / (30 - 10));\n\
	}																\n\
																	\n\
	float4 ps_main( in Vertex3D data ) : COLOR						\n\
	{																\n\
		float4 finalcolor =											\n\
			tex2D( texsampler, data.tposition.xy );					\n\
			finalcolor = data.fogfac * finalcolor +  (1.0 - data.fogfac) * float4(100.0/255,30.0/255,180.0/255,1) ;\n\
		return finalcolor;											\n\
	}																\n\
	";