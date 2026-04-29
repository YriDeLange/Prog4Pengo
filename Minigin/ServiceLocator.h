#pragma once
#include <memory>

namespace dae {

    class SoundSystem;

    class ServiceLocator final
    {
    public:
        static void RegisterSoundSystem(std::unique_ptr<SoundSystem> soundSystem);
        static SoundSystem& GetSoundSystem();
        static void Shutdown();

    private:
        static std::unique_ptr<SoundSystem> m_pSoundSystem;
        static SoundSystem* m_pDefaultSoundSystem;
    };

}