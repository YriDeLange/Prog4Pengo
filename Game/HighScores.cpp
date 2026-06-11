#include "HighScores.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace dae
{
    void HighScores::Load(const std::string& filePath)
    {
        m_entries.clear();

        std::ifstream file(filePath);
        if (!file.is_open())
            return;

        std::string line;
        while (std::getline(file, line) &&
            m_entries.size() < static_cast<size_t>(MAX_ENTRIES))
        {
            std::istringstream iss(line);
            Entry e;
            if (!(iss >> e.name >> e.score))
                continue;

            if (e.name.size() > static_cast<size_t>(NAME_LENGTH))
                e.name.resize(static_cast<size_t>(NAME_LENGTH));
            while (e.name.size() < static_cast<size_t>(NAME_LENGTH))
                e.name.push_back('-');

            m_entries.push_back(std::move(e));
        }

        std::sort(m_entries.begin(), m_entries.end(),
            [](const Entry& a, const Entry& b) { return a.score > b.score; });
    }

    bool HighScores::Save(const std::string& filePath) const
    {
        std::ofstream file(filePath, std::ios::trunc);
        if (!file.is_open())
            return false;

        for (const auto& e : m_entries)
            file << e.name << ' ' << e.score << '\n';

        return static_cast<bool>(file);
    }

    bool HighScores::Qualifies(int score) const
    {
        if (m_entries.size() < static_cast<size_t>(MAX_ENTRIES))
            return true;
        return score > m_entries.back().score;
    }

    void HighScores::Insert(const std::string& name, int score)
    {
        Entry e{ name, score };
        auto pos = std::upper_bound(m_entries.begin(), m_entries.end(), e,
            [](const Entry& a, const Entry& b) { return a.score > b.score; });
        m_entries.insert(pos, std::move(e));

        if (m_entries.size() > static_cast<size_t>(MAX_ENTRIES))
            m_entries.resize(static_cast<size_t>(MAX_ENTRIES));
    }
}