#pragma once
#include "pch.h"
#include "INPC.h"
#include <vector>
namespace Demo {
    enum class ChestRewardType { ITEM, CARD };

    struct ChestReward {
        ChestRewardType type = ChestRewardType::ITEM;
        int itemID = -1;
        std::string cardSaveID;
        int quantity = 1;

        static ChestReward Item(int id, int qty = 1) {
            return { ChestRewardType::ITEM, id, "", qty };
        }
        static ChestReward Card(const std::string& saveID) {
            return { ChestRewardType::CARD, -1, saveID, 1 };
        }
    };
    class TreasureChestNPC : public INPC {
    private:
        bool isOpened = false;
        bool randomPick = false;
        std::vector<ChestReward> rewards;
		std::shared_ptr<DX9GF::Texture> openedTexture;
		std::shared_ptr<DX9GF::AnimatedSprite> openedSprite;
		void MapCharacterVoices(std::unordered_map<std::wstring, std::string>& voiceMap) override {
			voiceMap[L"Treasure Chest"] = "bleep20";
		}
    public:
        TreasureChestNPC(std::weak_ptr<DX9GF::TransformManager> tm, float x, float y,
            std::vector<ChestReward> rewards, bool randomPick = false);
        void Init(DX9GF::GraphicsDevice* gd, DX9GF::Camera* camera, std::shared_ptr<Player> p, std::shared_ptr<DX9GF::ColliderManager> cm, std::shared_ptr<DX9GF::Font> font, std::shared_ptr<DX9GF::CommandBuffer> drawBuffer) override;
        void Draw(const DX9GF::Camera& camera, unsigned long long deltaTime) override;
        std::vector<ChestReward> Open(Player* p);
        bool CanInteract()  const override { return isPlayerNear && !isOpened; }
        bool GetIsOpened()  const { return isOpened; }
        void SetOpened(bool state) { isOpened = state; }
    };
}