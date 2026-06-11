#include "SoundSystemImpl.h"
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <iostream>
#include <unordered_map>
#include <vector>

#ifndef __EMSCRIPTEN__
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#endif

namespace dae {

    struct SoundRequest
    {
        sound_id id;
        float volume;
    };

    struct SoundSystemImpl::Impl
    {
        MIX_Mixer* m_mixer = nullptr;
        std::unordered_map<sound_id, MIX_Audio*> m_loadedSounds;
        std::vector<MIX_Track*> m_tracks;

        MIX_Track* m_musicTrack = nullptr;
        sound_id   m_currentMusicId{ static_cast<sound_id>(-1) };
        bool       m_musicStarted{ false };
        bool       m_muted{ false };

#ifndef __EMSCRIPTEN__
        std::queue<SoundRequest> m_requestQueue;
        mutable std::mutex m_mutex;
        std::condition_variable m_cv;
        std::thread m_thread;
        bool m_running{ true };
#endif
    };

    SoundSystemImpl::SoundSystemImpl()
        : m_pImpl(std::make_unique<Impl>())
    {
        if (MIX_Init() == false) {
            std::cerr << "MIX_Init failed: " << SDL_GetError() << std::endl;
        }

        m_pImpl->m_mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
        if (m_pImpl->m_mixer == nullptr) {
            std::cerr << "MIX_CreateMixerDevice failed: " << SDL_GetError() << std::endl;
        }
        else {
            constexpr int NUM_EFFECT_TRACKS = 16;
            for (int i = 0; i < NUM_EFFECT_TRACKS; ++i) {
                MIX_Track* track = MIX_CreateTrack(m_pImpl->m_mixer);
                if (track == nullptr) {
                    std::cerr << "MIX_CreateTrack failed: " << SDL_GetError() << std::endl;
                }
                else {
                    m_pImpl->m_tracks.push_back(track);
                }
            }

            // Dedicated music track, separate from the effect pool.
            m_pImpl->m_musicTrack = MIX_CreateTrack(m_pImpl->m_mixer);
            if (m_pImpl->m_musicTrack == nullptr) {
                std::cerr << "MIX_CreateTrack (music) failed: " << SDL_GetError() << std::endl;
            }
        }

#ifndef __EMSCRIPTEN__
        m_pImpl->m_thread = std::thread(&SoundSystemImpl::Run, this);
#endif
    }

    SoundSystemImpl::~SoundSystemImpl()
    {
#ifndef __EMSCRIPTEN__
        {
            std::lock_guard<std::mutex> lock(m_pImpl->m_mutex);
            m_pImpl->m_running = false;
        }
        m_pImpl->m_cv.notify_one();
        if (m_pImpl->m_thread.joinable()) m_pImpl->m_thread.join();
#endif

        if (m_pImpl->m_musicTrack) {
            MIX_DestroyTrack(m_pImpl->m_musicTrack);
            m_pImpl->m_musicTrack = nullptr;
        }

        for (auto track : m_pImpl->m_tracks) {
            if (track) {
                MIX_DestroyTrack(track);
            }
        }
        m_pImpl->m_tracks.clear();

        for (auto& pair : m_pImpl->m_loadedSounds) {
            MIX_DestroyAudio(pair.second);
        }

        if (m_pImpl->m_mixer) {
            MIX_DestroyMixer(m_pImpl->m_mixer);
        }
        MIX_Quit();
    }

    void SoundSystemImpl::LoadSound(sound_id id, const std::string& filePath)
    {
        if (m_pImpl->m_mixer == nullptr) {
            std::cerr << "Cannot load sound: mixer not initialized" << std::endl;
            return;
        }

        MIX_Audio* audio = MIX_LoadAudio(m_pImpl->m_mixer, filePath.c_str(), true);
        if (audio == nullptr) {
            std::cerr << "Failed to load " << filePath << ": " << SDL_GetError() << std::endl;
            return;
        }

#ifndef __EMSCRIPTEN__
        std::lock_guard<std::mutex> lock(m_pImpl->m_mutex);
#endif
        auto it = m_pImpl->m_loadedSounds.find(id);
        if (it != m_pImpl->m_loadedSounds.end()) {
            MIX_DestroyAudio(it->second);
        }
        m_pImpl->m_loadedSounds[id] = audio;
    }

    // ---------------- Music ----------------

    void SoundSystemImpl::PlayMusic(sound_id id, float volume)
    {
        if (!m_pImpl->m_musicTrack) return;

        MIX_Audio* audio = nullptr;
        {
#ifndef __EMSCRIPTEN__
            std::lock_guard<std::mutex> lock(m_pImpl->m_mutex);
#endif
            // Idempotent: same song already started -> leave it playing.
            if (m_pImpl->m_musicStarted && m_pImpl->m_currentMusicId == id
                && MIX_TrackPlaying(m_pImpl->m_musicTrack))
                return;

            auto it = m_pImpl->m_loadedSounds.find(id);
            if (it == m_pImpl->m_loadedSounds.end()) return;
            audio = it->second;

            m_pImpl->m_currentMusicId = id;
            m_pImpl->m_musicStarted = true;
        }

        MIX_SetTrackAudio(m_pImpl->m_musicTrack, audio);
        MIX_SetTrackGain(m_pImpl->m_musicTrack, volume);

        SDL_PropertiesID props = SDL_CreateProperties();
        SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, -1); // infinite loop
        MIX_PlayTrack(m_pImpl->m_musicTrack, props);
        SDL_DestroyProperties(props);
    }

    void SoundSystemImpl::PauseMusic()
    {
        if (m_pImpl->m_musicTrack)
            MIX_PauseTrack(m_pImpl->m_musicTrack);
    }

    void SoundSystemImpl::ResumeMusic()
    {
        if (m_pImpl->m_musicTrack)
            MIX_ResumeTrack(m_pImpl->m_musicTrack);
    }

    void SoundSystemImpl::StopMusic()
    {
        if (m_pImpl->m_musicTrack)
        {
            MIX_StopTrack(m_pImpl->m_musicTrack, 0);
#ifndef __EMSCRIPTEN__
            std::lock_guard<std::mutex> lock(m_pImpl->m_mutex);
#endif
            m_pImpl->m_musicStarted = false;
        }
    }

    // ---------------- Mute ----------------

    void SoundSystemImpl::SetMuted(bool muted)
    {
        {
#ifndef __EMSCRIPTEN__
            std::lock_guard<std::mutex> lock(m_pImpl->m_mutex);
#endif
            m_pImpl->m_muted = muted;
        }
        if (m_pImpl->m_mixer)
            MIX_SetMixerGain(m_pImpl->m_mixer, muted ? 0.0f : 1.0f);
    }

    bool SoundSystemImpl::IsMuted() const
    {
#ifndef __EMSCRIPTEN__
        std::lock_guard<std::mutex> lock(m_pImpl->m_mutex);
#endif
        return m_pImpl->m_muted;
    }

    // ---------------- Effects ----------------

#ifdef __EMSCRIPTEN__
    void SoundSystemImpl::Play(sound_id id, float volume)
    {
        auto it = m_pImpl->m_loadedSounds.find(id);
        if (it == m_pImpl->m_loadedSounds.end())
            return;

        MIX_Audio* audioToPlay = it->second;
        MIX_Track* chosenTrack = nullptr;

        for (auto track : m_pImpl->m_tracks) {
            if (track && !MIX_TrackPlaying(track)) {
                chosenTrack = track;
                break;
            }
        }

        if (audioToPlay && chosenTrack) {
            MIX_SetTrackAudio(chosenTrack, audioToPlay);
            MIX_SetTrackGain(chosenTrack, volume);
            MIX_PlayTrack(chosenTrack, 0);
        }
    }
#else
    void SoundSystemImpl::Play(sound_id id, float volume)
    {
        std::lock_guard<std::mutex> lock(m_pImpl->m_mutex);
        m_pImpl->m_requestQueue.push({ id, volume });
        m_pImpl->m_cv.notify_one();
    }

    void SoundSystemImpl::Run()
    {
        while (true) {
            SoundRequest req{};
            MIX_Audio* audioToPlay = nullptr;
            MIX_Track* chosenTrack = nullptr;

            {
                std::unique_lock<std::mutex> lock(m_pImpl->m_mutex);
                m_pImpl->m_cv.wait(lock, [this] {
                    return !m_pImpl->m_requestQueue.empty() || !m_pImpl->m_running;
                    });

                if (!m_pImpl->m_running && m_pImpl->m_requestQueue.empty())
                    break;

                if (!m_pImpl->m_requestQueue.empty()) {
                    req = m_pImpl->m_requestQueue.front();
                    m_pImpl->m_requestQueue.pop();

                    auto it = m_pImpl->m_loadedSounds.find(req.id);
                    if (it != m_pImpl->m_loadedSounds.end()) {
                        audioToPlay = it->second;

                        for (auto track : m_pImpl->m_tracks) {
                            if (track && !MIX_TrackPlaying(track)) {
                                chosenTrack = track;
                                break;
                            }
                        }
                    }
                }
            }

            if (audioToPlay && chosenTrack) {
                MIX_SetTrackAudio(chosenTrack, audioToPlay);
                MIX_SetTrackGain(chosenTrack, req.volume);
                MIX_PlayTrack(chosenTrack, 0);
            }
        }
    }
#endif

}