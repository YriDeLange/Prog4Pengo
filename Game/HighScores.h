#pragma once
#include <string>
#include <vector>

namespace dae
{
    class HighScores final
    {
    public:
        struct Entry
        {
            std::string name;
            int score{ 0 };
        };

        static constexpr int MAX_ENTRIES = 10;
        static constexpr int NAME_LENGTH = 3;

        void Load(const std::string& filePath);
        bool Save(const std::string& filePath) const;

        bool Qualifies(int score) const;

        void Insert(const std::string& name, int score);

        const std::vector<Entry>& GetEntries() const { return m_entries; }

    private:
        std::vector<Entry> m_entries;
    };
}