cbuffer CBObject : register(b0)
{
	matrix m_world;
	matrix m_view;
	matrix m_proj;
};

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
	vout.norm = normalize(vin.norm);
	vout.uv = vin.uv;
	vout.col = vin.col;
	return vout;
}

float4 PSMain(VertexOut pin) : SV_TARGET
{
	return pin.col;
}
