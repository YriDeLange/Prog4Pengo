#include "LevelLoader.h"
#include "LevelGrid.h"
#include "Scene.h"
#include "GameObject.h"
#include "RenderComponent.h"
#include "IceBlock.h"

#include <fstream>
#include <iostream>

namespace dae
{
    char LevelLoader::TileAt(const std::vector<std::string>& rows, int x, int y)
    {
        if (y < 0 || y >= static_cast<int>(rows.size())) return '.';
        const std::string& row = rows[y];
        if (x < 0 || x >= static_cast<int>(row.size()))  return '.';
        return row[x];
    }

    LevelData LevelLoader::Load(const std::string& filePath, Scene& scene)
    {
        LevelData data{};

        std::ifstream file(filePath);
        if (!file.is_open())
        {
            std::cerr << "LevelLoader: failed to open '" << filePath << "'\n";
            return data;
        }

        std::vector<std::string> rows;
        std::string line;
        while (rows.size() < static_cast<size_t>(GRID_HEIGHT) && std::getline(file, line))
        {
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            rows.push_back(line);
        }

        if (rows.empty())
        {
            std::cerr << "LevelLoader: '" << filePath << "' contained no rows\n";
            return data;
        }

        auto& grid = LevelGrid::GetInstance();
        grid.Init(GRID_WIDTH, GRID_HEIGHT, 16, 8.f, 41.f);

        for (int y = 0; y < GRID_HEIGHT; ++y)
        {
            for (int x = 0; x < GRID_WIDTH; ++x)
            {
                const char tile = TileAt(rows, x, y);

                switch (tile)
                {
                case 'B':
                case 'D':
                case 'E':
                {
                    BlockType type = BlockType::Normal;
                    if (tile == 'D') { type = BlockType::Diamond; ++data.diamondCount; }
                    else if (tile == 'E') { type = BlockType::Egg; ++data.eggBlockCount; }

                    auto block = std::make_unique<GameObject>();
                    block->AddComponent<RenderComponent>();
                    block->AddComponent<IceBlock>(x, y, type);
                    scene.Add(std::move(block));
                    break;
                }
                case 'P':
                    data.pengoSpawns.push_back({ x, y });
                    break;
                case 'S':
                    data.snoBeeSpawns.push_back({ x, y });
                    break;
                case '.':
                default:
                    break;
                }
            }
        }

        data.loaded = true;
        return data;
    }
}