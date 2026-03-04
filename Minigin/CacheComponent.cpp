#include "CacheComponent.h"
#include <imgui.h>
#include <implot.h>
#include <chrono>
#include <algorithm>
#include <numeric>

struct Transform
{
    float matrix[16]{
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
    };
};

class GameObject3D
{
public:
    Transform transform;
    int ID{};
};

class GameObject3DAlt
{
public:
    int ID{}; 
};

template<typename Fn>
static std::vector<float> Benchmark(Fn accessor, int elementCount, int samples)
{
    const int runs = std::max(samples, 3);
    std::vector<float> results;

    for (int step = 1; step <= 1024; step *= 2)
    {
        std::vector<long long> timings;
        timings.reserve(runs);

        for (int r = 0; r < runs; ++r)
        {
            const auto start = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < elementCount; i += step)
                accessor(i);
            const auto end = std::chrono::high_resolution_clock::now();
            timings.push_back(
                std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
        }

        std::sort(timings.begin(), timings.end());
        long long sum = std::accumulate(timings.begin() + 1, timings.end() - 1, 0LL);
        results.push_back(static_cast<float>(sum / static_cast<long long>(runs - 2)));
    }

    return results;
}

std::vector<float> dae::CacheComponent::RunIntBenchmark(int samples)
{
    constexpr int COUNT = 1 << 26; // 2^26 ints
    int* arr = new int[COUNT]();
    auto results = Benchmark([&](int i) { arr[i] *= 2; }, COUNT, samples);
    delete[] arr;
    return results;
}

std::vector<float> dae::CacheComponent::RunGameObjectBenchmark(int samples)
{
    constexpr int COUNT = 10'000'000;
    auto* arr = new GameObject3D[COUNT]();
    auto results = Benchmark([&](int i) { arr[i].ID *= 2; }, COUNT, samples);
    delete[] arr;
    return results;
}

std::vector<float> dae::CacheComponent::RunGameObjectAltBenchmark(int samples)
{
    constexpr int COUNT = 10'000'000;
    auto* arr = new GameObject3DAlt[COUNT]();
    auto results = Benchmark([&](int i) { arr[i].ID *= 2; }, COUNT, samples);
    delete[] arr;
    return results;
}

dae::CacheComponent::CacheComponent(GameObject* pOwner)
    : Component(pOwner)
{
}

void dae::CacheComponent::RenderUI() const
{
    static const double xs[] = { 0,1,2,3,4,5,6,7,8,9,10 };
    static const char* labels[] = { "1","2","4","8","16","32","64","128","256","512","1024" };
    constexpr int       stepCount = 11;

    ImGui::SetNextWindowSize(ImVec2(400, 280), ImGuiCond_Once);
    ImGui::Begin("Exercise 1");

    ImGui::InputInt("# samples", &m_samplesEx1);
    if (m_samplesEx1 < 1) m_samplesEx1 = 1;

    if (ImGui::Button("Thrash the cache"))
        m_intResults = RunIntBenchmark(m_samplesEx1);

    if (!m_intResults.empty())
    {
        if (ImPlot::BeginPlot("##ex1plot", ImVec2(-1, 180), ImPlotFlags_Crosshairs | ImPlotFlags_NoMouseText))
        {
            ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoTickLabels, ImPlotAxisFlags_NoTickLabels);
            ImPlot::SetupLegend(ImPlotLocation_East, ImPlotLegendFlags_Outside);
            ImPlot::SetupAxisTicks(ImAxis_X1, xs, stepCount, labels);
            ImPlot::PlotLine("int array", m_intResults.data(), stepCount);

            if (ImPlot::IsPlotHovered())
            {
                ImPlotPoint mouse = ImPlot::GetPlotMousePos();
                int idx = static_cast<int>(std::round(mouse.x));
                idx = std::clamp(idx, 0, (int)m_intResults.size() - 1);

                float px = static_cast<float>(idx);

                ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 6, ImVec4(1, 0, 0, 1), 1);
                ImPlot::PlotScatter("##dot1", &px, &m_intResults[idx], 1);

                ImGui::BeginTooltip();
                ImGui::Text("x=%.2f, y=%.2f", std::pow(2.0, idx), m_intResults[idx]);
                ImGui::EndTooltip();
            }

            ImPlot::EndPlot();
        }
    }

    ImGui::End();

    ImGui::SetNextWindowSize(ImVec2(400, 310), ImGuiCond_Once);
    ImGui::Begin("Exercise 2");

    ImGui::InputInt("# samples", &m_samplesEx2);
    if (m_samplesEx2 < 1) m_samplesEx2 = 1;

    if (ImGui::Button("Thrash the cache with GameObject3D"))
        m_goResults = RunGameObjectBenchmark(m_samplesEx2);

    if (ImGui::Button("Thrash the cache with GameObject3DAlt"))
        m_goAltResults = RunGameObjectAltBenchmark(m_samplesEx2);

    if (!m_goResults.empty() || !m_goAltResults.empty())
    {
        if (ImPlot::BeginPlot("##ex2plot", ImVec2(-1, 180), ImPlotFlags_Crosshairs | ImPlotFlags_NoMouseText))
        {
            ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoTickLabels, ImPlotAxisFlags_NoTickLabels);
            ImPlot::SetupLegend(ImPlotLocation_East, ImPlotLegendFlags_Outside);
            ImPlot::SetupAxisTicks(ImAxis_X1, xs, stepCount, labels);

            if (!m_goResults.empty())
                ImPlot::PlotLine("GameObject3D", m_goResults.data(), stepCount);
            if (!m_goAltResults.empty())
                ImPlot::PlotLine("GameObject3DAlt", m_goAltResults.data(), stepCount);

            if (ImPlot::IsPlotHovered())
            {
                ImPlotPoint mouse = ImPlot::GetPlotMousePos();
                int idx = static_cast<int>(std::round(mouse.x));
                idx = std::clamp(idx, 0, stepCount - 1);

                float px = static_cast<float>(idx);

                // Draw dots INSIDE the plot, BEFORE EndTooltip
                if (!m_goResults.empty())
                {
                    ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 6, ImVec4(0.2f, 0.6f, 1.f, 1.f), 1);
                    ImPlot::PlotScatter("##dotGO", &px, &m_goResults[idx], 1);
                }
                if (!m_goAltResults.empty())
                {
                    ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 6, ImVec4(1.f, 0.6f, 0.2f, 1.f), 1);
                    ImPlot::PlotScatter("##dotGOAlt", &px, &m_goAltResults[idx], 1);
                }

                // Tooltip is ONLY for text
                ImGui::BeginTooltip();
                if (!m_goResults.empty())
                    ImGui::Text("GameObject3D    x=%.2f, y=%.2f", std::pow(2.0, idx), m_goResults[idx]);
                if (!m_goAltResults.empty())
                    ImGui::Text("GameObject3DAlt x=%.2f, y=%.2f", std::pow(2.0, idx), m_goAltResults[idx]);
                ImGui::EndTooltip();
            }

            ImPlot::EndPlot();
        }
    }

    ImGui::End();
}