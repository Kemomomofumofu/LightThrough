#pragma once

#include	<d3d11.h>
#include	<string>
#include	<wrl/client.h> // ComPtrの定義を含むヘッダファイル
#include	<filesystem>
#include	<SimpleMath.h>

using Microsoft::WRL::ComPtr;
using namespace DirectX::SimpleMath;
//-----------------------------------------------------------------------------
//Textureクラス
//-----------------------------------------------------------------------------
class Texture
{
	std::string m_texname{}; // ファイル名
	ComPtr<ID3D11ShaderResourceView> m_srv{}; // シェーダーリソースビュー

	int m_width; // 幅
	int m_height; // 高さ
	int m_bpp; // BPP

	int m_u; // 横の分割数
	int m_v; // 縦の分割数
	float m_frameWidth;
	float m_frameHeight;

	bool m_loopFlg; // ループフラグ
public:
	bool Load(const std::string& filename);
	bool LoadFromFemory(const unsigned char* data, int len);

	void SetFrameInfo(int _u, int _v);
	auto GetU() const { return m_u; }
	auto GetV() const { return m_v; }
	Vector2 GetFrameSize() const { return Vector2(m_frameWidth, m_frameHeight); }

	void SetLoopFlg(bool _loopFlg) { m_loopFlg = _loopFlg; }
	auto GetLoopFlg() const { return m_loopFlg; }

	void SetGPU();
};