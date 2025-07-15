/**
 * @file Base.cpp
 * @brief 基底クラスさん
 * @author Arima Keita
 * @date 2025-06-25
 */

/*---------- インクルード ----------*/
#include <DX3D/Core/Base.h>

/*---------- 名前空間 ----------*/
using namespace dx3d;


/**
 * @brief コンストラクタ
 * @param _desc 
 */
dx3d::Base::Base(const BaseDesc& _desc): logger_(_desc.logger)
{
}

dx3d::Base::~Base()
{
}


/**
 * @brief ロガーの取得
 * @return ロガー
 */
Logger& dx3d::Base::GetLogger() noexcept
{
	return logger_;
}
