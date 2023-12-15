cbuffer cbObject : register(b0)
{
	matrix m_world;
	matrix m_view;
	matrix m_proj;
};

Texture2D txAlbedo : register(t0);
TextureCube txCube : register(t1);
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
	//vout.pos = float4(vin.pos.xy * 0.2f, 0.2f, 0.3f);
	vout.inPos = vin.pos;
	vout.norm = normalize(vin.norm);
	vout.uv = vin.uv;
	vout.col = vin.col;
	return vout;
}


float4 PSMain(VertexOut pin) : SV_TARGET
{
	float4 tex = txAlbedo.SampleLevel(ssLinear, pin.uv, 8.0);
	if (length(m_world._22) > 10.0f)
		tex = txCube.SampleLevel(ssLinear, pin.inPos.xyz, 0.0);
	//if (pin.uv.x > 0.5)
	{
		//tex = txAlbedo.Sample(ssLinear, pin.uv);
	}
	return tex;
}
