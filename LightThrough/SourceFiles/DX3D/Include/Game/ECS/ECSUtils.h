#pragma once
/**
 * @file ECSUtils.h
 * @brief ECSで使う便利機能を置いとく場所
 * @author Arima Keita
 * @date 2025-08-06
 */

// ---------- インクルード ---------- // 
#include <cstdint>
#include <bitset>


namespace ecs {


	// ---------- Entity ---------- // 
	constexpr uint32_t INDEX_BITS = 24;	// idのBit数
	constexpr uint32_t VERSION_BITS = 8;	// VersionのBit数
	constexpr uint32_t INDEX_MASK = (1 << INDEX_BITS) - 1;
	constexpr uint32_t VERSION_MASK = (1 << VERSION_BITS) - 1;

	/**
	 * @brief Entity生成
	 * @param _index EntityのIndex
	 * @param _version EntityのVersion
	 * @return EntityのIndexとVersionをまとめた物
	 */
	inline uint32_t CreateEntity(uint32_t _index, uint32_t _version) {
		return((_version & VERSION_MASK) << INDEX_BITS) | (_index & INDEX_MASK);
	}
	
	inline uint32_t GetIndex(uint32_t _entity) {
		return _entity & INDEX_MASK;
	}

	inline uint32_t GetVersion(uint32_t _entity) {
		return (_entity >> INDEX_BITS) & VERSION_MASK;
	}


	// ---------- Component ---------- // 
	constexpr std::size_t MAX_COMPONENTS = 64;	// 最大Component数
	using ComponentType = std::uint8_t;	// ComponentのType( 0 ~ MAX_COMPONENT - 1 を表せれる整数) 


	// ---------- System ---------- // 
	using Signature = std::bitset<MAX_COMPONENTS>;	// 各Entityが持つComponentのSignature
}
