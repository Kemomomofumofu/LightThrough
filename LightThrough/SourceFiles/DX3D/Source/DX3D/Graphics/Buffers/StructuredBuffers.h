#pragma once
#include <DX3D/Graphics/GraphicsResource.h>
#include <DX3D/Graphics/DeviceContext.h>

namespace dx3d {

	//! @brief 構造化バッファクラス
    class StructuredBuffer final : public GraphicsResource
    {
    public:
        StructuredBuffer(const StructuredBufferDesc& _desc, const GraphicsResourceDesc& _gDesc);
        ID3D11ShaderResourceView* GetSRV() const { return srv_.Get(); }
        ID3D11Buffer* GetBuffer() const { return buffer_.Get(); }

    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> buffer_;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_;
    };

	//! @brief 読み書き可能な構造化バッファクラス
    class RWStructuredBuffer final : public GraphicsResource
    {
    public:
        RWStructuredBuffer(const RWStructuredBufferDesc& _desc, const GraphicsResourceDesc& _gDesc);
        ID3D11UnorderedAccessView* GetUAV() const { return uav_.Get(); }
        ID3D11Buffer* GetBuffer() const { return buffer_.Get(); }
    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> buffer_;
        Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> uav_;
    };

	//! @brief ステージングバッファクラス
    class StagingBuffer : public GraphicsResource {
    public:
        StagingBuffer(const StagingBufferDesc& _desc, const GraphicsResourceDesc& _gDesc);
        ID3D11Buffer* GetBuffer() const { return buffer_.Get(); }

    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> buffer_;
    };
}
