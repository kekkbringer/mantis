//
// Created by dominik on 3/4/26.
//

#ifndef CLI_ENGINE_HPP
#define CLI_ENGINE_HPP

#include <string>
#include <functional>
#include <string_view>

#include "config.hpp"

/**
 * This structure is the template for any command line option. Using this struct, one can
 * define how each possible argument is parsed and initialized based on the cli.
 */
struct Option {
    std::string long_name;       ///< long name of the option, e.g. --version
    char short_name = '\0';      ///< short name of the option, e.g. -v
    bool takes_value = false;    ///< does it take a value or is it just a flag?
    bool allow_attached = false; ///< name with attachment, e.g. -O3
    std::function<void(Config&, std::string_view)> action;
                                 ///< function for the parsing of the cli argument
};

Config parse_cli_arguments(const int argc, const char* argv[]);

#endif //CLI_ENGINE_HPP
