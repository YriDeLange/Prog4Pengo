#include "ServiceLocator.h"
#include "NullSoundSystem.h"
#include "SoundSystem.h"

namespace dae {

    std::unique_ptr<SoundSystem> ServiceLocator::m_pSoundSystem = nullptr;
    SoundSystem* ServiceLocator::m_pDefaultSoundSystem = nullptr;

    void ServiceLocator::RegisterSoundSystem(std::unique_ptr<SoundSystem> soundSystem)
    {
        m_pSoundSystem = std::move(soundSystem);
    }

    SoundSystem& ServiceLocator::GetSoundSystem()
    {
        if (m_pSoundSystem)
            return *m_pSoundSystem;

        // Lazy-create null service once
        if (!m_pDefaultSoundSystem)
            m_pDefaultSoundSystem = new NullSoundSystem();
        return *m_pDefaultSoundSystem;
    }

    void ServiceLocator::Shutdown()
    {
        delete m_pDefaultSoundSystem;
        m_pDefaultSoundSystem = nullptr;
        m_pSoundSystem.reset();
    }
}