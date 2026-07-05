#include "pch.h"
#include "TransitionCommand.h"

namespace Demo {
    TransitionCommand::TransitionCommand(DX9GF::GraphicsDevice* gd, DX9GF::Camera* uiCamera, float duration, bool isTransitioningIn, int numPillars)
        : graphicsDevice(gd), uiCamera(uiCamera), duration(duration), elapsedTime(0.0f), isTransitioningIn(isTransitioningIn), numPillars(numPillars)
    {
        auto [camW, camH] = uiCamera->GetScreenResolution();
        screenWidth = camW;
        screenHeight = camH;
    }

    void TransitionCommand::Execute(unsigned long long deltaTime) {
        if (IsFinished()) return;
        frame++;

        elapsedTime += static_cast<float>(deltaTime) / 1000.0f;
        if (elapsedTime >= duration) {
            elapsedTime = duration;
            MarkFinished();
        }

        float progress = elapsedTime / duration;
        float pillarWidth = static_cast<float>(screenWidth) / numPillars;

        float startX = -screenWidth / 2.0f;
        float startY = -screenHeight / 2.0f;

        for (int i = 0; i < numPillars; ++i) {
            float staggerDuration = 0.5f;
            float tStart = (static_cast<float>(i) / numPillars) * staggerDuration;
            float tEnd = tStart + (1.0f - staggerDuration);

            float localProgress = 0.0f;
            if (progress > tStart) {
                localProgress = (progress - tStart) / (tEnd - tStart);
                if (localProgress > 1.0f) localProgress = 1.0f;
            }

            localProgress = EaseInOutQuad(localProgress);

            float currentHeight = 0.0f;
            float y = startY;

            if (isTransitioningIn) {

                currentHeight = screenHeight * localProgress;
            }
            else {
                currentHeight = screenHeight * (1.0f - localProgress);
            }

            if (currentHeight > 0) {
                graphicsDevice->DrawRectangle(
                    *uiCamera,
                    startX + (i * pillarWidth), y,
                    pillarWidth + 1.0f, currentHeight,
                    0xFF000000, true
                );
            }
        }
    }
}