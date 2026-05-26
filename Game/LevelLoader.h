#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace dae
{
    class Scene;

    // Result of loading a level: everything the caller needs to finish
    // setting up a round. The loader creates the ice blocks itself (and
    // registers them with the LevelGrid), but leaves player/enemy creation
    // to the caller, since that depends on the game mode.
    struct LevelData
    {
        std::vector<glm::ivec2> pengoSpawns;   // 'P' tiles, in file order
        std::vector<glm::ivec2> snoBeeSpawns;  // 'S' tiles, in file order
        std::vector<glm::ivec2> wallTiles;     // '#' tiles (optional use)
        int eggBlockCount{ 0 };                // number of 'E' tiles created
        int diamondCount{ 0 };                 // number of 'D' tiles created
        bool loaded{ false };                  // false if the file failed to load
    };

    class LevelLoader final
    {
    public:
        // Fixed Pengo board dimensions (matches LevelGrid::Init in Main.cpp).
        static constexpr int GRID_WIDTH = 13;
        static constexpr int GRID_HEIGHT = 15;

        // Reads the level file at `filePath`, (re)initialises the LevelGrid,
        // and adds an IceBlock GameObject to `scene` for each B/D/E tile.
        // Returns spawn data; LevelData::loaded is false on failure.
        //
        // NOTE: this assumes the grid has been cleared / the scene does not
        // already contain stale blocks. Call on a fresh scene per level.
        static LevelData Load(const std::string& filePath, Scene& scene);

    private:
        static char TileAt(const std::vector<std::string>& rows, int x, int y);
    };
}