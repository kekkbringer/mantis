//
// Created by dominik on 2/21/26.
//

#include "util.hpp"

#include <string>

/**
 * This function takes a 'name' and first identifies if it is mangled, in which case it returns the demangled name.
 * If the name is not mangled, it is simply returned unmodified.
 * @param mangled_name name to demangle
 * @return demangled name
 */
std::string demangle(const std::string& mangled_name) {
    // .ma.NUMBER.name
    if (mangled_name.substr(0, 4) == ".ma.") {
        const size_t pos = mangled_name.find('.', 4);
        if (pos != std::string::npos) {
            return mangled_name.substr(pos+1);
        }
    }
    return mangled_name;
}