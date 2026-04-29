#pragma once
#include "SoundSystem.h"
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <unordered_map>
#include <string>
#include <memory>

namespace dae {

    struct SoundRequest
    {
        sound_id id;
        float volume;
    };

    class SoundSystemImpl final : public SoundSystem
    {
    public:
        SoundSystemImpl();
        ~SoundSystemImpl() override;

        void Play(sound_id id, float volume) override;
        void LoadSound(sound_id id, const std::string& filePath) override;

    private:
        void Run();

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;

        std::queue<SoundRequest> m_requestQueue;
        std::mutex m_mutex;
        std::condition_variable m_cv;
        std::thread m_thread;
        bool m_running{ true };
    };

}