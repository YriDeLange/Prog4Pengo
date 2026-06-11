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

        void PlayMusic(sound_id id, float volume) override;
        void PauseMusic() override;
        void ResumeMusic() override;
        void StopMusic() override;

        void SetMuted(bool muted) override;
        bool IsMuted() const override;

    private:
#ifndef __EMSCRIPTEN__
        void Run();
#endif

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };

}