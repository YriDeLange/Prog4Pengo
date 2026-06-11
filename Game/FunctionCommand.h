#pragma once
#include "Command.h"
#include <functional>

namespace dae
{
    class FunctionCommand final : public Command
    {
        std::function<void()> m_func;
    public:
        explicit FunctionCommand(std::function<void()> func)
            : m_func(std::move(func)) {}

        void Execute(float /*deltaTime*/) override
        {
            if (m_func) m_func();
        }
    };
}