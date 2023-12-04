cbuffer CBObject : register(b0)
{
	matrix m_world;
	matrix m_view;
	matrix m_proj;
};

cbuffer CBVal : register(b1)
{
	// ´Ó val0 - val63
	float m_val0;
    float m_val1;
	float m_val2;
	float m_val3;
	float m_val4;
	float m_val5;
	float m_val6;
	float m_val7;
	float m_val8;
	float m_val9;
	float m_val10;
	float m_val11;
	float m_val12;
	float m_val13;
	float m_val14;
	float m_val15;
	float m_val16;
	float m_val17;
	float m_val18;
	float m_val19;
	float m_val20;
	float m_val21;
	float m_val22;
	float m_val23;
	float m_val24;
	float m_val25;
	float m_val26;
	float m_val27;
	float m_val28;
	float m_val29;
	float m_val30;
	float m_val31;
	float m_val32;
	float m_val33;
	float m_val34;
	float m_val35;
	float m_val36;
	float m_val37;
	float m_val38;
	float m_val39;
	float m_val40;
	float m_val41;
	float m_val42;
	float m_val43;
	float m_val44;
	float m_val45;
	float m_val46;
	float m_val47;
	float m_val48;
	float m_val49;
	float m_val50;
	float m_val51;
	float m_val52;
	float m_val53;
	float m_val54;
	float m_val55;
	float m_val56;
	float m_val57;
	float m_val58;
	float m_val59;
	float m_val60;
	float m_val61;
	float m_val62;
	float m_val63;
}

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
