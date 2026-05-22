#pragma once
#include "Singleton.h"
#include <vector>
#include <glm/glm.hpp>

namespace dae
{
    class IceBlock;

    class LevelGrid final : public Singleton<LevelGrid>
    {
    public:
        void Init(int width, int height, int cellSize = 16, float offsetX = 0.f, float offsetY = 0.f);
        void Clear();

        glm::vec2 GridToWorld(int gridX, int gridY) const;
        glm::ivec2 WorldToGrid(float worldX, float worldY) const;
        glm::vec2 SnapToGrid(float worldX, float worldY) const;

        bool IsInBounds(int gridX, int gridY) const;
        bool IsPositionBlocked(int gridX, int gridY) const;
        IceBlock* GetBlockAt(int gridX, int gridY) const;

        void RegisterBlock(IceBlock* block, int gridX, int gridY);
        void UnregisterBlock(int gridX, int gridY);
        void MoveBlock(IceBlock* block, int fromX, int fromY, int toX, int toY);

        int GetWidth() const { return m_width; }
        int GetHeight() const { return m_height; }
        int GetCellSize() const { return m_cellSize; }
        float GetOffsetX() const { return m_offsetX; }
        float GetOffsetY() const { return m_offsetY; }

    private:
        friend class Singleton<LevelGrid>;
        LevelGrid() = default;

        int m_width = 0;
        int m_height = 0;
        int m_cellSize = 16;
        float m_offsetX = 0.f;
        float m_offsetY = 0.f;
        std::vector<std::vector<IceBlock*>> m_grid;
    };
}