//
// Created by dominik on 7/31/25.
//

#ifndef SOURCE_LOCATION_HPP
#define SOURCE_LOCATION_HPP

#include <cstddef>
#include <string_view>

/**
 * Class that contains all information to reference a specific location of the source code. The file name is kept track
 * of to uniquely identify included files. The offset is the global offset from the start of the source code. For
 * convenience, the line and column number are stored as well, starting at 1.
 */
struct Source_location {
    std::string_view file_name; ///< name of the file
    std::size_t offset = 0;     ///< global offset from the beginning of the source code
    std::size_t line = 1;       ///< line number
    std::size_t column = 1;     ///< column number
};

#endif //SOURCE_LOCATION_HPP
