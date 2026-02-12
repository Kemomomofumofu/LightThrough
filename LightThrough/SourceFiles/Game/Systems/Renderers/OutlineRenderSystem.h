/**
 * @file OutlineRenderSystem.h
 * @brief アウトラインを描画するシステム(ポストプロセス)
 */

 // ---------- インクルード ---------- //
#include <Game/ECS/ISystem.h>
#include <DX3D/Graphics/GraphicsEngine.h>

namespace ecs
{
	class OutlineRenderSystem : public ISystem
	{
	public:
		explicit OutlineRenderSystem(const SystemDesc& _desc);

	};
}