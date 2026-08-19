#include "pch.h"
#include "ICard.h"
#include "AdvancedCards.h"
#include "StrikeCard.h"
#include "EnergyCard.h"
#include "UtilityCards.h"
#include "FinisherCards.h"
#include "MainBlockCard.h"

std::shared_ptr<Demo::ICard> Demo::ICard::CreateCard(const std::string& id, std::weak_ptr<DX9GF::TransformManager> transformManager, std::shared_ptr<DraggableManager> draggableManager, DX9GF::GraphicsDevice* graphicsDevice, DX9GF::Camera* camera) {
	std::shared_ptr<ICard> card;
	if (id == "HeavyStrikeCard") card = std::make_shared<HeavyStrikeCard>(transformManager);
	else if (id == "TwinStrikeCard") card = std::make_shared<TwinStrikeCard>(transformManager);
	else if (id == "CleaveCard") card = std::make_shared<CleaveCard>(transformManager);
	else if (id == "ChainLightningCard") card = std::make_shared<ChainLightningCard>(transformManager);
	else if (id == "PoisonCard") card = std::make_shared<PoisonCard>(transformManager);
	else if (id == "VulnerableCard") card = std::make_shared<VulnerableCard>(transformManager);
	else if (id == "WeaknessCard") card = std::make_shared<WeaknessCard>(transformManager);
	else if (id == "StunCard") card = std::make_shared<StunCard>(transformManager);
	else if (id == "StrikeCard") card = std::make_shared<StrikeCard>(transformManager);
	else if (id == "EnergyCard") card = std::make_shared<EnergyCard>(transformManager);
	else if (id == "JabCard") card = std::make_shared<JabCard>(transformManager);
	else if (id == "MarkCard") card = std::make_shared<MarkCard>(transformManager);
	else if (id == "BraceCard") card = std::make_shared<BraceCard>(transformManager);
	else if (id == "PrefetchCard") card = std::make_shared<PrefetchCard>(transformManager);
	else if (id == "OverclockCard") card = std::make_shared<OverclockCard>(transformManager);
	else if (id == "JumpstartCard") card = std::make_shared<JumpstartCard>(transformManager);
	else if (id == "ForesightCard") card = std::make_shared<ForesightCard>(transformManager);
	else if (id == "TerminateCard") card = std::make_shared<TerminateCard>(transformManager);
	else if (id == "InfernoCard") card = std::make_shared<InfernoCard>(transformManager);
	else if (id == "SystemPurgeCard") card = std::make_shared<SystemPurgeCard>(transformManager);
	else if (id == "OverdriveCard") card = std::make_shared<OverdriveCard>(transformManager);
	else if (id == "MainBlockCard") card = std::make_shared<MainBlockCard>(transformManager);
	else if (id == "IgniteCard") card = std::make_shared<IgniteCard>(transformManager);
	else if (id == "FireDetonationCard") card = std::make_shared<FireDetonationCard>(transformManager);
	else if (id == "RagingStrikeCard") card = std::make_shared<RagingStrikeCard>(transformManager);
	else if (id == "OverloadCard") card = std::make_shared<OverloadCard>(transformManager);
	else if (id == "ChainReactionCard") card = std::make_shared<ChainReactionCard>(transformManager);
	else if (id == "LethalHarvestCard") card = std::make_shared<LethalHarvestCard>(transformManager);
	else if (id == "ShieldBashCard") card = std::make_shared<ShieldBashCard>(transformManager);
	else if (id == "CruelStrikeCard") card = std::make_shared<CruelStrikeCard>(transformManager);
	else if (id == "ArmorPiercerCard") card = std::make_shared<ArmorPiercerCard>(transformManager);
	else if (id == "ExecuteCard") card = std::make_shared<ExecuteCard>(transformManager);
	if (card && draggableManager && graphicsDevice && camera) {
		if (auto dragCard = std::dynamic_pointer_cast<IDraggable>(card)) {
			dragCard->Init(draggableManager, graphicsDevice, camera);
		}
	}
	return card;
}
