#include "pch.h"
#include "TestBattleScene.h"
#include "DemonEyeEnemy.h"
#include "CupidEnemy.h"
#include "VampireBatEnemy.h"
#include "MimicEnemy.h"
#include "WarlockEnemy.h"
void Demo::TestBattleScene::Init()
{
	IBattleScene::Init();
	/*auto testEnemy = std::make_shared<TestEnemy>(transformManager, 50.f);
	testEnemy->Init(game->GetGraphicsDevice(), &camera);
	testEnemy->SetOnRequestEnemyCard([this](std::shared_ptr<IEnemy> enemy) {
		CreateEnemyCard(enemy);
	});
	enemies.push_back(testEnemy);
	testEnemy = std::make_shared<TestEnemy>(transformManager, 50.f);
	testEnemy->Init(game->GetGraphicsDevice(), &camera);
	testEnemy->SetOnRequestEnemyCard([this](std::shared_ptr<IEnemy> enemy) {
		CreateEnemyCard(enemy);
	});
	enemies.push_back(testEnemy);*/

	//auto demonEye = std::make_shared<DemonEyeEnemy>(transformManager, 60.f);
	//demonEye->Init(game->GetGraphicsDevice(), &camera);
	//demonEye->SetOnRequestEnemyCard([this](std::shared_ptr<IEnemy> enemy) {
	//	CreateEnemyCard(enemy);
	//	});
	//enemies.push_back(demonEye);

	//auto cupid = std::make_shared<CupidEnemy>(transformManager, 50.f);
	//cupid->Init(game->GetGraphicsDevice(), &camera);
	//cupid->SetOnRequestEnemyCard([this](std::shared_ptr<IEnemy> enemy) {
	//	CreateEnemyCard(enemy);
	//	});
	//enemies.push_back(cupid);

	/*for (size_t i = 0; i < 9; ++i) {
		auto strikeCard = std::make_shared<StrikeCard>(transformManager, -260.f, -80.f);
		strikeCard->Init(draggableManager, game->GetGraphicsDevice(), &camera);
		drawPile.push_back(strikeCard);
	}*/

	//auto bat = std::make_shared<VampireBatEnemy>(transformManager, 45.f);
	//bat->Init(game->GetGraphicsDevice(), &camera);
	//bat->SetOnRequestEnemyCard([this](std::shared_ptr<IEnemy> enemy) { CreateEnemyCard(enemy); });
	//enemies.push_back(bat);

	//auto mimic = std::make_shared<MimicEnemy>(transformManager, 70.f);
	//mimic->Init(game->GetGraphicsDevice(), &camera);
	//mimic->SetOnRequestEnemyCard([this](std::shared_ptr<IEnemy> enemy) { CreateEnemyCard(enemy); });
	//enemies.push_back(mimic);

	auto warlock = std::make_shared<WarlockEnemy>(transformManager, 55.f);
	warlock->Init(game->GetGraphicsDevice(), &camera);
	warlock->SetOnRequestEnemyCard([this](std::shared_ptr<IEnemy> enemy) { CreateEnemyCard(enemy); });
	enemies.push_back(warlock);

	// Heavy Strike
	auto heavy = std::make_shared<HeavyStrikeCard>(transformManager, -260.f, -80.f);
	drawPile.push_back(heavy);

	// Twin Strike
	auto twin = std::make_shared<TwinStrikeCard>(transformManager, -260.f, -80.f);
	drawPile.push_back(twin);

	// Cleave
	auto cleave = std::make_shared<CleaveCard>(transformManager, -260.f, -80.f);
	drawPile.push_back(cleave);

	// Chain Lightning
	auto chain = std::make_shared<ChainLightningCard>(transformManager, -260.f, -80.f);
	drawPile.push_back(chain);

	// Poison
	auto poison = std::make_shared<PoisonCard>(transformManager, -260.f, -80.f);
	drawPile.push_back(poison);

	// Vulnerable
	auto vuln = std::make_shared<VulnerableCard>(transformManager, -260.f, -80.f);
	drawPile.push_back(vuln);

	// Weakness
	auto weak = std::make_shared<WeaknessCard>(transformManager, -260.f, -80.f);
	drawPile.push_back(weak);

	// Stun
	auto stun = std::make_shared<StunCard>(transformManager, -260.f, -80.f);
	drawPile.push_back(stun);

	// Strike
	auto strikeCard = std::make_shared<StrikeCard>(transformManager, -260.f, -80.f);
	drawPile.push_back(strikeCard);

	heavy->Init(draggableManager, game->GetGraphicsDevice(), &camera);
	twin->Init(draggableManager, game->GetGraphicsDevice(), &camera);
	cleave->Init(draggableManager, game->GetGraphicsDevice(), &camera);
	chain->Init(draggableManager, game->GetGraphicsDevice(), &camera);
	poison->Init(draggableManager, game->GetGraphicsDevice(), &camera);
	vuln->Init(draggableManager, game->GetGraphicsDevice(), &camera);
	weak->Init(draggableManager, game->GetGraphicsDevice(), &camera);
	stun->Init(draggableManager, game->GetGraphicsDevice(), &camera);
	strikeCard->Init(draggableManager, game->GetGraphicsDevice(), &camera);

	this->GiveTestItems();

	transformManager->RebuildHierarchy();
	StartBattle();
}

void Demo::TestBattleScene::GiveTestItems()
{
	ItemInventory& testItems = this->player->GetInventoryItems();
	testItems.InitFixedInventory(10);

	testItems.AddItem(0, 5);
	testItems.AddItem(1, 3);
	testItems.AddItem(2, 2);
	testItems.AddItem(3, 1);
	testItems.AddItem(4, 1);
	testItems.AddItem(5, 1);
	testItems.AddItem(6, 1);
	testItems.AddItem(7, 1);
	testItems.AddItem(8, 1);
	testItems.AddItem(9, 1);
}
