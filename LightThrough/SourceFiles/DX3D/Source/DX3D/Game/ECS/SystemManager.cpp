#include "SystemManager.h"

void ecs::SystemManager::EntitySignatureChanged(Entity _e, Signature _eSignature)
{
	for (auto const& pair : systems_) {
		auto const& type = pair.first;
		auto const& system = pair.second;
		auto const& sysSig = signature_[type];

		if ((_eSignature & sysSig) == sysSig) {
			system->entities_.insert(_e);
		}
		else {
			system->entities_.erase(_e);
		}
	}
}

void ecs::SystemManager::EntityDestroyed(Entity _e)
{
	for (auto const& pair : systems_) {
		pair.second->entities_.erase(_e);
	}
}
