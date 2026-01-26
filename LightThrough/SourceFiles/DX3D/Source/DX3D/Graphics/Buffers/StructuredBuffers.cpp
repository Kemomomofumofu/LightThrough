
/**
 * @file StructuredBuffers.cpp
 * @brief 
 */

// ---------- インクルード ---------- // 
#include <DX3D/Graphics/Buffers/StructuredBuffers.h>
#include <DX3D/Graphics/GraphicsDevice.h>

namespace dx3d {

    //! @brief StructuredBufferコンストラクタ
	StructuredBuffer::StructuredBuffer(const StructuredBufferDesc& _desc, const GraphicsResourceDesc& _gDesc)
        : GraphicsResource(_gDesc)
		, immediate_(_gDesc.immediateContext)
    {
		// バッファ作成
        D3D11_BUFFER_DESC bd{};
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.ByteWidth = _desc.elementSize * _desc.elementCount;
        bd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        bd.StructureByteStride = _desc.elementSize;

        D3D11_SUBRESOURCE_DATA srd{};
        D3D11_SUBRESOURCE_DATA* pInit = nullptr;
        if (_desc.initData) {
            srd.pSysMem = _desc.initData;
            pInit = &srd;
        }

        DX3DGraphicsLogThrowOnFail(
            device_.CreateBuffer(&bd, pInit, &buffer_),
            "[StructuredBuffer] CreateBuffer失敗"
        );

		// SRV作成
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
        srvDesc.BufferEx.FirstElement = 0;
        srvDesc.BufferEx.NumElements = _desc.elementCount;

        DX3DGraphicsLogThrowOnFail(
            device_.CreateShaderResourceView(buffer_.Get(), &srvDesc, &srv_),
            "[StructuredBuffer] SRV作成失敗"
        );
    }

	//! @brief バッファ更新
    void StructuredBuffer::Update(const void* _data, size_t _size)
    {
        if (!_data || _size == 0) { return; }

        D3D11_MAPPED_SUBRESOURCE mapped{};
        DX3DGraphicsLogThrowOnFail(
            immediate_->Map(buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped),
            "[StructuredBuffer] Mapに失敗"
        );

        memcpy(mapped.pData, _data, _size);
        immediate_->Unmap(buffer_.Get(), 0);
    }


    
    //! @brief RWStructuredBufferコンストラクタ
    RWStructuredBuffer::RWStructuredBuffer(const RWStructuredBufferDesc& _desc, const GraphicsResourceDesc& _gDesc)
        : GraphicsResource(_gDesc)
    {
		// バッファ作成
        D3D11_BUFFER_DESC bd{};
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = _desc.elementSize * _desc.elementCount;
        bd.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
        bd.CPUAccessFlags = 0;
        bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        bd.StructureByteStride = _desc.elementSize;

        DX3DGraphicsLogThrowOnFail(
            device_.CreateBuffer(&bd, nullptr, &buffer_),
            "[RWStructuredBuffer] CreateBuffer失敗"
        );

		// UAV作成
        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
        uavDesc.Format = DXGI_FORMAT_UNKNOWN;
        uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.NumElements = _desc.elementCount;

        DX3DGraphicsLogThrowOnFail(
            device_.CreateUnorderedAccessView(buffer_.Get(), &uavDesc, &uav_),
            "[RWStructuredBuffer] UAV作成失敗"
        );
    }

    //! @brief StagingBufferコンストラクタ
    StagingBuffer::StagingBuffer(const StagingBufferDesc& _desc, const GraphicsResourceDesc& _gDesc)
        : GraphicsResource(_gDesc)
        , immediate_(_gDesc.immediateContext)
    {
        D3D11_BUFFER_DESC bd{};
        bd.Usage = D3D11_USAGE_STAGING;
        bd.ByteWidth = _desc.elementSize * _desc.elementCount;
        bd.BindFlags = 0;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        bd.StructureByteStride = _desc.elementSize;

        DX3DGraphicsLogThrowOnFail(
            device_.CreateBuffer(&bd, nullptr, &buffer_),
            "[StagingBuffer] CreateBuffer失敗"
        );
    }

	// @brief バッファマップ
    void* StagingBuffer::Map()
    {
        D3D11_MAPPED_SUBRESOURCE mapped{};
        DX3DGraphicsLogThrowOnFail(
            immediate_->Map(buffer_.Get(), 0, D3D11_MAP_READ, 0, &mapped),
            "[StagingBuffer] Map失敗"
        );

        return mapped.pData;
    }

	// @brief バッファアンマップ
    void StagingBuffer::Unmap()
    {
        immediate_->Unmap(buffer_.Get(), 0);
    }

} // namespace dx3d
