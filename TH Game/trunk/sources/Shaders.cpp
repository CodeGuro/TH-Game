#include <string>

extern std::string const DefaultShader = 
	"																\n\
	//Default shaders												\n\
																	\n\
	uniform float4x4 WorldViewProjMat;								\n\
	sampler texsampler;												\n\
																	\n\
	struct Vertex3D													\n\
	{																\n\
		float4 vposition : POSITION0;								\n\
		float2 tposition : TEXCOORD0;								\n\
		float4 color : COLOR0;										\n\
	};																\n\
																	\n\
	void vs_main( inout Vertex3D data )								\n\
	{																\n\
		data.vposition = mul( data.vposition, WorldViewProjMat );	\n\
	}																\n\
																	\n\
	float4 ps_main( in Vertex3D data ) : COLOR						\n\
	{																\n\
		return tex2D( texsampler, data.tposition.xy ) * data.color;	\n\
	}																\n\
	";