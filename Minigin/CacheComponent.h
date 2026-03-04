#pragma once
#include "Component.h"
#include <vector>

namespace dae
{
    class CacheComponent final : public Component
    {
    public:
        explicit CacheComponent(GameObject* pOwner);
        void RenderUI() const override;

    private:
        mutable int m_samplesEx1{ 10 };
        mutable int m_samplesEx2{ 10 };

        mutable std::vector<float> m_intResults{};
        mutable std::vector<float> m_goResults{};
        mutable std::vector<float> m_goAltResults{};

        static std::vector<float> RunIntBenchmark(int samples);
        static std::vector<float> RunGameObjectBenchmark(int samples);
        static std::vector<float> RunGameObjectAltBenchmark(int samples);
    };
}