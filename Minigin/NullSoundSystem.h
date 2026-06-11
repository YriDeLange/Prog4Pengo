#pragma once
#include "SoundSystem.h"

namespace dae {

    class NullSoundSystem final : public SoundSystem
    {
    public:
        void Play(sound_id /*id*/, float /*volume*/) override {}
        void LoadSound(sound_id /*id*/, const std::string& /*filePath*/) override {}

        void PlayMusic(sound_id /*id*/, float /*volume*/) override {}
        void PauseMusic() override {}
        void ResumeMusic() override {}
        void StopMusic() override {}

        void SetMuted(bool /*muted*/) override {}
        bool IsMuted() const override { return false; }
    };

}