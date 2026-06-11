#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace dae
{
    class Scene;

    
    struct LevelData
    {
        std::vector<glm::ivec2> pengoSpawns;
        std::vector<glm::ivec2> snoBeeSpawns;
        int eggBlockCount{ 0 };
        int diamondCount{ 0 };
        bool loaded{ false };
    };

    class LevelLoader final
    {
    public:
        static constexpr int GRID_WIDTH = 13;
        static constexpr int GRID_HEIGHT = 15;

        static LevelData Load(const std::string& filePath, Scene& scene);

    private:
        static char TileAt(const std::vector<std::string>& rows, int x, int y);
    };
}