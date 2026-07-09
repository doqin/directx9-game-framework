#pragma once
#include <random>

namespace Demo {
    class RNG {
    private:
        // Đảm bảo chỉ có DUY NHẤT 1 máy quay xổ số được tạo ra trên toàn game (Meyer's Singleton)
        static std::mt19937& GetEngine() {
            static std::mt19937 gen(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));
            return gen;
        }

    public:
        // Trả về số nguyên Random (Ví dụ: roll chiêu 1 đến 3)
        static int Range(int min, int max) {
            std::uniform_int_distribution<int> dist(min, max);
            return dist(GetEngine());
        }

        // Trả về số thực Random (Ví dụ: roll tọa độ đạn -450.f đến 150.f)
        static float Range(float min, float max) {
            std::uniform_real_distribution<float> dist(min, max);
            return dist(GetEngine());
        }
    };
}