//
// Created by dominik on 2/25/26.
//

#ifndef UNTITLED_STRING_TABLE_HPP
#define UNTITLED_STRING_TABLE_HPP

#include <string>
#include <unordered_set>

class StringTable {
    std::unordered_set<std::string> table;

public:
    /**
     * This function interns a string in the table's storage and returns a stable string_view. The underlying data is
     * owned by the table, so the string_view is valid for the whole lifetime of the table.
     * @param strv string(_view) to intern
     * @return stable string_view to the entry in the table
     */
    std::string_view intern(const std::string_view strv) {
        // first, look for strv in the set
        if (const auto it = table.find(std::string(strv)); it != table.end())
            return std::string_view(*it);

        // not already known, add to set
        auto [it, _] = table.emplace(strv);
        return std::string_view(*it);
    }

    /**
     * This function returns the size of the internal table.
     * @return size of table
     */
    [[nodiscard]] size_t site() const { return table.size(); }
};

#endif //UNTITLED_STRING_TABLE_HPP