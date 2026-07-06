#pragma once
#include "IEnemy.h"
#include "MimicEnemy.h"
#include <string>
#include <memory>

namespace Demo {
    class EnemyFactory {
    public:
        static std::shared_ptr<IEnemy> Create(const std::string& type, std::weak_ptr<DX9GF::TransformManager> tm, DX9GF::GraphicsDevice* gd, DX9GF::Camera* cam) {
            std::shared_ptr<IEnemy> enemy;

            if (type == "MimicEnemy") {
                auto mimic = std::make_shared<MimicEnemy>(tm, 50.f);
                mimic->Init(gd, cam);
                enemy = mimic;
            }
            // Thêm các loại quái khác ở đây

            return enemy;
        }
    };
}