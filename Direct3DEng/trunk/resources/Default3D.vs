//Default vertex shader to transform vertices by multiplying
//vertices by the world:view:projection matrix

uniform float4x4 WorldViewProjMat;
struct Vertex3D
{
	float4 vposition : POSITION0;
	float2 tposition : TEXCOORD0;
	float4 color : COLOR0;
};


void vs_main( inout Vertex3D data )
{
	data.vposition = mul( data.vposition, WorldViewProjMat );
}