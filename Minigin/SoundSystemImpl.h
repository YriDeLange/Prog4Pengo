#pragma once
#include "SoundSystem.h"
#include <memory>

namespace dae {

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
    };

}