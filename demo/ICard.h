#pragma once
#include "DX9GFExtras.h"

namespace Demo {
	class DraggableManager;
	class Player;
	class ICard : virtual public DX9GF::IGameObject, public DX9GF::ISaveable {
	protected:
		Player* owner = nullptr;
		bool isLocked = false;
		int lockedTurns = 0;
	public:
		inline ICard(std::weak_ptr<DX9GF::TransformManager> transformManager) : IGameObject(transformManager) {}
		inline ICard(
			std::weak_ptr<DX9GF::TransformManager> transformManager,
			float x,
			float y,
			float rotation = 0,
			float scaleX = 1,
			float scaleY = 1
		) : IGameObject(transformManager, x, y, rotation, scaleX, scaleY) {
		}
		inline ICard(
			std::weak_ptr<DX9GF::TransformManager> transformManager,
			std::weak_ptr<DX9GF::IGameObject> parent,
			float x,
			float y,
			float rotation = 0,
			float scaleX = 1,
			float scaleY = 1
		) : IGameObject(transformManager, parent, x, y, rotation, scaleX, scaleY) {
		}

		void SetOwner(Player* p) { owner = p; }
		Player* GetOwner() const { return owner; }

		virtual size_t GetCost() const { return 0; }
		virtual std::wstring GetDescription() const { return L""; }

		static std::shared_ptr<ICard> CreateCard(
			const std::string& id,
			std::weak_ptr<DX9GF::TransformManager> transformManager,
			std::shared_ptr<DraggableManager> draggableManager = nullptr,
			DX9GF::GraphicsDevice* graphicsDevice = nullptr,
			DX9GF::Camera* camera = nullptr
		);

		virtual std::string GetSaveID() const override {
			std::string name = typeid(*this).name();
			size_t pos = name.find_last_of(':');
			if (pos != std::string::npos) return name.substr(pos + 1);
			pos = name.find_last_of(' ');
			if (pos != std::string::npos) return name.substr(pos + 1);
			return name;
		}
		virtual void GenerateSaveData(nlohmann::json& outData) override {}
		virtual void RestoreSaveData(const nlohmann::json& inData) override {}
		bool IsLocked() const { return isLocked; }
		int GetLockedTurns() const { return lockedTurns; }
		void SetLocked(int turns) {
			isLocked = true;
			lockedTurns = turns;
		}
		void TickLock() {
			if (isLocked) {
				lockedTurns--;
				if (lockedTurns <= 0) {
					isLocked = false;
					lockedTurns = 0;
				}
			}
		}
	};
}
