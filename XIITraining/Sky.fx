cbuffer cbFrame : register(b0)
{
	matrix m_view;
	matrix m_proj;
};

cbuffer cbObject : register(b1)
{
	matrix m_world;
}

TextureCube txCube : register(t0);
SamplerState ssLinear : register(s0);

struct VertexIn
{
	float3 pos : POSITION;
	float3 norm : NORMAL;
	float2 uv : TEXCOORD;
	float4 col : COLOR;
};

struct VertexOut
{
	float4 pos : SV_POSITION;
	float3 inPos : POSITION0;
	float3 norm : NORMAL;
	float2 uv : TEXCOORD;
	float4 col : COLOR;
};

VertexOut VSMain(VertexIn vin)
{
	VertexOut vout;
	vout.pos = mul(float4(vin.pos, 1.0f), m_world);
	vout.pos = mul(vout.pos, m_view);
	vout.pos = mul(vout.pos, m_proj);
	vout.inPos = vin.pos;
	vout.norm = normalize(vin.norm);
	vout.uv = vin.uv;
	vout.col = vin.col;
	return vout;
}


float4 PSMain(VertexOut pin) : SV_TARGET
{
	return txCube.SampleLevel(ssLinear, pin.inPos.xyz, 4.0);
}
