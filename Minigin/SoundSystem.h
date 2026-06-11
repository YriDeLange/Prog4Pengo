#pragma once
#include <string>

namespace dae {

    using sound_id = unsigned short;

    class SoundSystem
    {
    public:
        virtual ~SoundSystem() = default;

        // One-shot effects
        virtual void Play(sound_id id, float volume) = 0;
        virtual void LoadSound(sound_id id, const std::string& filePath) = 0;

        // Music: a single dedicated looping track. PlayMusic is idempotent:
        // calling it with the id that is already playing does nothing, so
        // level transitions don't restart the song.
        virtual void PlayMusic(sound_id id, float volume) = 0;
        virtual void PauseMusic() = 0;
        virtual void ResumeMusic() = 0;
        virtual void StopMusic() = 0;

        // Global mute (music and effects). F2 toggles this.
        virtual void SetMuted(bool muted) = 0;
        virtual bool IsMuted() const = 0;
        void ToggleMute() { SetMuted(!IsMuted()); }
    };

}