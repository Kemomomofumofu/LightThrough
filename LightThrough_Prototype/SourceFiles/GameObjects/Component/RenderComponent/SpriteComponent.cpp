//==================================================
// [SpriteComponent.h] 描画コンポーネント
// 著者：有馬啓太
//--------------------------------------------------
// 説明：とても描画コンポーネントの宣言
//==================================================

/*----- インクルード -----*/
#include <iostream>
#include <format>
#include <cassert>
#include <string>
#include <d3d11.h>

#include "../../../GameManager.h"
#include "../../../TextureManager.h"
#include "../../../SubSystem/dx11helper.h"
#include "../../GameObject.h"

#include "SpriteComponent.h"

#include <wrl/client.h>
using namespace DirectX::SimpleMath;

//--------------------------------------------------
// コンストラクタ
//--------------------------------------------------
SpriteComponent::SpriteComponent(GameObject* _owner, const std::string _imgname, int _drawOrder)
	: RenderComponent(_owner, _drawOrder)
	, texture_(nullptr)
	, current_frame_x_(0)
	, current_frame_y_(0)
{
	owner_transform_ = owner_->GetComponent<TransformComponent>();

	if (_imgname != "default") {
		// テクスチャ取得
		texture_ = TextureManager::GetInstance().GetTexture(_imgname);

		// バッファ初期化
		this->InitBuffers("unlitTextureVS.hlsl", "unlitTexturePS.hlsl");
	}
	else {
		this->InitBuffers("SimpleVS.hlsl", "SimplePS.hlsl");
	}



	this->Init();
}
//--------------------------------------------------
// デストラクタ
//--------------------------------------------------
SpriteComponent::~SpriteComponent()
{

	Uninit();
}

//--------------------------------------------------
// 初期化処理
//--------------------------------------------------
void SpriteComponent::Init()
{

}

//--------------------------------------------------
// 終了処理
//--------------------------------------------------

void SpriteComponent::Uninit()
{
}


//--------------------------------------------------
// 更新処理
//--------------------------------------------------
void SpriteComponent::Update()
{
}

//--------------------------------------------------
// 描画処理
//--------------------------------------------------
void SpriteComponent::Draw()
{

if (!owner_transform_)
	{
		owner_transform_ = owner_->GetComponent<TransformComponent>();
	}


	auto si = owner_transform_->GetSize();
	auto t = owner_transform_->GetPosition();
	auto r = owner_transform_->GetRotation();
	auto sc = owner_transform_->GetScale();

	auto finalSize = Vector3(si * sc);	// 最終的なサイズ


	Matrix rot = Matrix::CreateFromYawPitchRoll(r.x, r.y, r.z);
	Matrix pos = Matrix::CreateTranslation(t);
	Matrix scale = Matrix::CreateScale(finalSize);



	Matrix worldmtx;
	worldmtx = scale * rot * pos;
	Renderer::SetWorldMatrix(&worldmtx); // GPUにセット

	// 描画の処理
	ID3D11DeviceContext* devicecontext;
	devicecontext = Renderer::GetDeviceContext();
	assert(devicecontext);

	// トポロジーをセット（プリミティブタイプ）
	devicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);



	shader_.SetGPU();
	vertex_buffer_.SetGPU();
	index_buffer_.SetGPU();

	if (texture_)
	{
		// サンプラーステートをセット
		Renderer::SetWrapSamplerState();

		//// テクスチャがあるならアニメーションの処理
		//static float frameCount = 0;
		//const int framesPerUpdate = 3;

		//frameCount++;
		//if (frameCount >= framesPerUpdate) {
		//	frameCount = 0;

		//	current_frame_x_ = (current_frame_x_ + 1) % texture_->GetU();
		//	if (current_frame_x_ == 0)
		//	{
		//		current_frame_y_ = (current_frame_y_ + 1) % texture_->GetV();
		//	}
		//}

		//// UV座標を設定
		//SetAnimationFrame(current_frame_x_, current_frame_y_);



		texture_->SetGPU();
				
	}
	else {
		// テクスチャがないなら頂点カラーで描画
		// テクスチャのキャッシュが残っていたら嫌なので消しとく
		ID3D11ShaderResourceView* nullSRV = nullptr;
		devicecontext->PSSetShaderResources(0, 1, &nullSRV);
	}


	devicecontext->DrawIndexed(
		4,							// 描画するインデックス数（四角形なんで４）
		0,							// 最初のインデックスバッファの位置
		0);
}


void SpriteComponent::SetTexture(const std::string _imgname)
{
	texture_ = TextureManager::GetInstance().GetTexture(_imgname);

}

void SpriteComponent::SetAnimationFrame(int _frameX, int _frameY)
{
	auto frameSize = texture_->GetFrameSize();
	float u = _frameX * frameSize.x;
	float v = _frameY * frameSize.y;

	// UV座標を設定
	vertices_[0].uv = Vector2(u, v);
	vertices_[1].uv = Vector2(u + frameSize.x, v);
	vertices_[2].uv = Vector2(u , v + frameSize.y);
	vertices_[3].uv = Vector2(u+ frameSize.x, v + frameSize.y);

	// 更新
	vertex_buffer_.Modify(vertices_);
}

void SpriteComponent::SetTextureRepeat(bool _enable, float _unitSize)
{
	if (!texture_) return;

	if (!owner_transform_)
	{
		owner_transform_ = owner_->GetComponent<TransformComponent>();
	}

	auto size = owner_transform_->GetSize();
	if (_enable) {
		// テクスチャのタイリング
		float tileX = size.x / _unitSize;
		float tileY = size.y / _unitSize;

		vertices_[0].uv = { 0.0f, 0.0f };
		vertices_[1].uv = { tileX, 0.0f };
		vertices_[2].uv = { 0.0f, tileY };
		vertices_[3].uv = { tileX, tileY };
	}
	else {
		// 通常のテクスチャ座標にする
		vertices_[0].uv = { 0.0f, 0.0f };
		vertices_[1].uv = { 1.0f, 0.0f };
		vertices_[2].uv = { 0.0f, 1.0f };
		vertices_[3].uv = { 1.0f, 1.0f };
	}

	vertex_buffer_.Modify(vertices_);
}

//--------------------------------------------------
// 色変更
//--------------------------------------------------
void SpriteComponent::SetColor(const DirectX::SimpleMath::Vector4& _color)
{
	for (auto& vertex : vertices_)
	{
		vertex.color = _color;		// 色を変更
	}

	vertex_buffer_.Modify(vertices_);	// バッファを書き換え
}


