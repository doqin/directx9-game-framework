#include "pch.h"
#include "TransitionCommand.h"

namespace Demo {
    TransitionCommand::TransitionCommand(DX9GF::GraphicsDevice* gd, float duration, bool isTransitioningIn, int numPillars)
        : graphicsDevice(gd), duration(duration), elapsedTime(0.0f), isTransitioningIn(isTransitioningIn), numPillars(numPillars), uiCamera(DX9GF::Application::GetInstance()->GetScreenWidth(), DX9GF::Application::GetInstance()->GetScreenHeight())
    {
        auto app = DX9GF::Application::GetInstance();
        screenWidth = app->GetScreenWidth();
        screenHeight = app->GetScreenHeight();
        uiCamera.SetPosition(screenWidth / 2.0f, screenHeight / 2.0f);
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

        for (int i = 0; i < numPillars; ++i) {
            // Calculate a stagger based on the pillar index for a left-to-right effect
            float staggerDuration = 0.5f; // Portion of the total duration for stagger
            float tStart = (static_cast<float>(i) / numPillars) * staggerDuration;
            float tEnd = tStart + (1.0f - staggerDuration);

            float localProgress = 0.0f;
            if (progress > tStart) {
                localProgress = (progress - tStart) / (tEnd - tStart);
                if (localProgress > 1.0f) localProgress = 1.0f;
            }

            localProgress = EaseInOutQuad(localProgress);

            float y = 0.0f;
            float currentHeight = 0.0f;

            if (isTransitioningIn) {
                // Falling down
                currentHeight = screenHeight * localProgress;
            }
            else {
                // Lifting up
                currentHeight = screenHeight * (1.0f - localProgress);
            }

            if (currentHeight > 0) {
                graphicsDevice->DrawRectangle(
                    uiCamera,
                    i * pillarWidth, y, 
                    pillarWidth + 1.0f, currentHeight, // +1.0f to overlap pillars and prevent 1px gap from rounding
                    0xFF000000, true
                );
            }
        }
    }
}
