#include "LevelGrid.h"
#include <algorithm>

namespace dae
{
    void LevelGrid::Init(int width, int height, int cellSize)
    {
        m_width = width;
        m_height = height;
        m_cellSize = cellSize;

        m_grid.clear();
        m_grid.resize(height, std::vector<IceBlock*>(width, nullptr));
    }

    void LevelGrid::Clear()
    {
        for (auto& row : m_grid)
            std::fill(row.begin(), row.end(), nullptr);
    }

    glm::vec2 LevelGrid::GridToWorld(int gridX, int gridY) const
    {
        return glm::vec2(
            static_cast<float>(gridX * m_cellSize),
            static_cast<float>(gridY * m_cellSize)
        );
    }

    glm::ivec2 LevelGrid::WorldToGrid(float worldX, float worldY) const
    {
        return glm::ivec2(
            static_cast<int>(worldX / m_cellSize),
            static_cast<int>(worldY / m_cellSize)
        );
    }

    glm::vec2 LevelGrid::SnapToGrid(float worldX, float worldY) const
    {
        auto gridPos = WorldToGrid(worldX, worldY);
        return GridToWorld(gridPos.x, gridPos.y);
    }

    bool LevelGrid::IsInBounds(int gridX, int gridY) const
    {
        return gridX >= 0 && gridX < m_width &&
            gridY >= 0 && gridY < m_height;
    }

    bool LevelGrid::IsPositionBlocked(int gridX, int gridY) const
    {
        if (!IsInBounds(gridX, gridY))
            return true;

        return m_grid[gridY][gridX] != nullptr;
    }

    IceBlock* LevelGrid::GetBlockAt(int gridX, int gridY) const
    {
        if (!IsInBounds(gridX, gridY))
            return nullptr;

        return m_grid[gridY][gridX];
    }

    void LevelGrid::RegisterBlock(IceBlock* block, int gridX, int gridY)
    {
        if (!IsInBounds(gridX, gridY))
            return;

        m_grid[gridY][gridX] = block;
    }

    void LevelGrid::UnregisterBlock(int gridX, int gridY)
    {
        if (!IsInBounds(gridX, gridY))
            return;

        m_grid[gridY][gridX] = nullptr;
    }

    void LevelGrid::MoveBlock(IceBlock* block, int fromX, int fromY, int toX, int toY)
    {
        if (!IsInBounds(fromX, fromY) || !IsInBounds(toX, toY))
            return;

        m_grid[fromY][fromX] = nullptr;
        m_grid[toY][toX] = block;
    }
}