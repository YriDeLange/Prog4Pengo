#pragma once
#include <string>

namespace dae {

    using sound_id = unsigned short;

    class SoundSystem
    {
    public:
        virtual ~SoundSystem() = default;

        virtual void Play(sound_id id, float volume) = 0;
        virtual void LoadSound(sound_id id, const std::string& filePath) = 0;
    };

}