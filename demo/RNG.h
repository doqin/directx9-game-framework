#pragma once
#include <random>

namespace Demo {
    class RNG {
    public:
        static int Range(int min, int max) {
            std::uniform_int_distribution<int> dist(min, max);
            return dist(GetEngine());
        }

        static float Range(float min, float max) {
            std::uniform_real_distribution<float> dist(min, max);
            return dist(GetEngine());
        }

        //singleton
        static std::mt19937& GetEngine() {
            static std::mt19937 gen(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));
            return gen;
        }
    };
}