#include "SoundSystemImpl.h"
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <iostream>

namespace dae {

    struct SoundSystemImpl::Impl
    {
        MIX_Mixer* m_mixer = nullptr;
        std::unordered_map<sound_id, MIX_Audio*> m_loadedSounds;
        std::vector<MIX_Track*> m_tracks;  // Pool of tracks for simultaneous sound effects (replaces SDL2 channels)
    };

    SoundSystemImpl::SoundSystemImpl()
        : m_pImpl(std::make_unique<Impl>())
    {
        if (MIX_Init() == false) {
            std::cerr << "MIX_Init failed: " << SDL_GetError() << std::endl;
        }

        // Create the new SDL3_mixer mixer (replaces Mix_OpenAudio)
        m_pImpl->m_mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
        if (m_pImpl->m_mixer == nullptr) {
            std::cerr << "MIX_CreateMixerDevice failed: " << SDL_GetError() << std::endl;
        }
        else {
            // Create a pool of tracks for overlapping sound effects (SDL2 used implicit channels via Mix_PlayChannel(-1))
            constexpr int NUM_EFFECT_TRACKS = 16;  // Plenty for most games; adjust if needed
            for (int i = 0; i < NUM_EFFECT_TRACKS; ++i) {
                MIX_Track* track = MIX_CreateTrack(m_pImpl->m_mixer);
                if (track == nullptr) {
                    std::cerr << "MIX_CreateTrack failed: " << SDL_GetError() << std::endl;
                }
                else {
                    m_pImpl->m_tracks.push_back(track);
                }
            }
        }

        m_thread = std::thread(&SoundSystemImpl::Run, this);
    }

    SoundSystemImpl::~SoundSystemImpl()
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_running = false;
        }
        m_cv.notify_one();
        if (m_thread.joinable()) m_thread.join();

        // Clean up tracks first
        for (auto track : m_pImpl->m_tracks) {
            if (track) {
                MIX_DestroyTrack(track);
            }
        }
        m_pImpl->m_tracks.clear();

        // Free loaded audio (replaces Mix_FreeChunk)
        for (auto& pair : m_pImpl->m_loadedSounds) {
            MIX_DestroyAudio(pair.second);
        }

        // Destroy the mixer and quit (good practice; SDL3_mixer is reference-counted)
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

        // Load audio (replaces Mix_LoadWAV). predecode=true is ideal for short sound effects.
        MIX_Audio* audio = MIX_LoadAudio(m_pImpl->m_mixer, filePath.c_str(), true);
        if (audio == nullptr) {
            std::cerr << "Failed to load " << filePath << ": " << SDL_GetError() << std::endl;
            return;
        }

        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_pImpl->m_loadedSounds.find(id);
        if (it != m_pImpl->m_loadedSounds.end()) {
            MIX_DestroyAudio(it->second);  // Replace existing sound
        }
        m_pImpl->m_loadedSounds[id] = audio;
    }

    void SoundSystemImpl::Play(sound_id id, float volume)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_requestQueue.push({ id, volume });
        m_cv.notify_one();
    }

    void SoundSystemImpl::Run()
    {
        while (true) {
            SoundRequest req{};
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_cv.wait(lock, [this] { return !m_requestQueue.empty() || !m_running; });

                if (!m_running && m_requestQueue.empty())
                    break;

                if (!m_requestQueue.empty()) {
                    req = m_requestQueue.front();
                    m_requestQueue.pop();
                }
            }

            // Process the request under lock (map is protected)
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = m_pImpl->m_loadedSounds.find(req.id);
            if (it != m_pImpl->m_loadedSounds.end()) {
                MIX_Audio* audio = it->second;
                if (audio) {
                    // Find a free track (replaces Mix_PlayChannel(-1))
                    MIX_Track* chosenTrack = nullptr;
                    for (auto track : m_pImpl->m_tracks) {
                        if (track && !MIX_TrackPlaying(track)) {
                            chosenTrack = track;
                            break;
                        }
                    }

                    if (chosenTrack) {
                        MIX_SetTrackAudio(chosenTrack, audio);
                        MIX_SetTrackGain(chosenTrack, req.volume);  // Volume is now a float 0.0f-1.0f (replaces Mix_VolumeChunk + MIX_MAX_VOLUME)
                        MIX_PlayTrack(chosenTrack, 0);             // 0 = default properties (one-shot, no loop)
                    }
                    // If no free track, we silently skip (original behavior when no channels were free).
                    // You can increase NUM_EFFECT_TRACKS if you hit this often.
                }
            }
        }
    }

} // namespace dae