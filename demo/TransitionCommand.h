#pragma once
#include <memory>
#include <vector>
#include "DX9GFExtras.h"
#include "DX9GFGraphicsDevice.h"
#include "DX9GFApplication.h"

namespace Demo {
    class TransitionCommand : public DX9GF::ICommand {
    private:
        DX9GF::GraphicsDevice* graphicsDevice;
        DX9GF::Camera uiCamera;
        float duration;
        float elapsedTime;
        bool isTransitioningIn;
        int numPillars;
        int screenWidth;
        int screenHeight;
        int frame = 0;
    public:
        TransitionCommand(DX9GF::GraphicsDevice* gd, float duration, bool isTransitioningIn, int numPillars = 10);
        void Execute(unsigned long long deltaTime) override;

        static float EaseInOutQuad(float t) {
            return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
        }
    };
}
