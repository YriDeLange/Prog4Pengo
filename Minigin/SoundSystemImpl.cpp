#include "SoundSystemImpl.h"
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <iostream>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <unordered_map>

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

        std::queue<SoundRequest> m_requestQueue;
        std::mutex m_mutex;
        std::condition_variable m_cv;
        std::thread m_thread;
        bool m_running{ true };
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
        }

        m_pImpl->m_thread = std::thread(&SoundSystemImpl::Run, this);
    }

    SoundSystemImpl::~SoundSystemImpl()
    {
        {
            std::lock_guard<std::mutex> lock(m_pImpl->m_mutex);
            m_pImpl->m_running = false;
        }
        m_pImpl->m_cv.notify_one();
        if (m_pImpl->m_thread.joinable()) m_pImpl->m_thread.join();

        // Clean up tracks first
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

        // Load audio
        MIX_Audio* audio = MIX_LoadAudio(m_pImpl->m_mixer, filePath.c_str(), true);
        if (audio == nullptr) {
            std::cerr << "Failed to load " << filePath << ": " << SDL_GetError() << std::endl;
            return;
        }

        std::lock_guard<std::mutex> lock(m_pImpl->m_mutex);
        auto it = m_pImpl->m_loadedSounds.find(id);
        if (it != m_pImpl->m_loadedSounds.end()) {
            MIX_DestroyAudio(it->second);
        }
        m_pImpl->m_loadedSounds[id] = audio;
    }

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

}