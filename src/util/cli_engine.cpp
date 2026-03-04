#include "cli_engine.hpp"
#include "config.hpp"

#include <algorithm>
#include <stdexcept>

// wrap into anonymous namespace to not expose the option API
namespace {
    /**
     * New command line arguments are added here and in 'config.hpp'.
     * Here, the arguments are configured depending on their needs.
     */
    std::vector<Option> make_options() {
        return {
            // --parse
            {
                "--parse", '\0',
                false, false, // takes_value, allow_attached
                [](Config& c, std::string_view) { c.stop_after_parser = true; }
            },

            // --tac
            {
                "--tac", '\0',
                false, false, // takes_value, allow_attached
                [](Config& c, std::string_view) { c.stop_after_tac = true; }
            },

            // --codegen
            {
                "--codegen", '\0',
                false, false, // takes_value, allow_attached
                [](Config& c, std::string_view) { c.stop_after_codegen = true; }
            },

            // --emit-asm / -S
            {
                "--emit-asm", 'S',
                false, false, // takes_value, allow_attached
                [](Config& c, std::string_view) { c.stop_after_compilation = true; }
            },

            // --compile / -c
            {
                "--compile", 'c',
                false, false, // takes_value, allow_attached
                [](Config& c, std::string_view) { c.stop_after_assembly = true; }
            },

            // --verbose
            {
                "--verbose", '\0',
                false, false, // takes_value, allow_attached
                [](Config& c, std::string_view) { c.verbose = true; }
            },

            // --verbose-level
            {
                "--verbose-level", '\0',
                true, false, // takes_value, allow_attached
                [](Config& c, std::string_view sv) { c.verbose_level = stoi(std::string(sv)); }
            },

            // --output / -o
            {
                "--output", 'o',
                true, false, // takes_value, allow_attached
                [](Config& c, std::string_view sv) { c.output = std::string(sv); }
            },

            // --version / -v
            {
                "--version", 'v',
                false, false, // takes_value, allow_attached
                [](Config& c, std::string_view) { c.print_version = true; }
            },

            // --help / -h
            {
                "--help", 'h',
                false, false, // takes_value, allow_attached
                [](Config& c, std::string_view) { c.print_help = true; }
            },
        };
    }
}

/**
 * Actual command line argument parsing function.
 */
Config parse_cli_arguments(const int argc, const char* argv[]) {
    Config config;
    auto options = make_options();
    
    // loop over all arguments given in the command line
    for (int i=1; i<argc; ++i) {
        std::string_view arg_name = argv[i];
        
        // long option, e.g. --version
        if (arg_name.starts_with("--")) {
            // iterator to find first match in options
            const auto it = std::find_if(options.begin(), options.end(),
                [&](const Option& o) {return o.long_name == arg_name; }
            );

            // no match
            if (it == options.end())
                throw std::runtime_error("Unknown option: " + std::string(arg_name));

            // get value if needed
            std::string_view value;
            if (it->takes_value) {
                if (i+1 >= argc)
                    throw std::runtime_error("Missing value for: " + std::string(arg_name));
                value = argv[++i];
            }

            // perform action specified for this option
            it->action(config, value);


        // short option, e.g. -v
        } else if (arg_name.size() >= 2 and arg_name[0] =='-') {
            char arg_short_name = arg_name[1];
            
            // iterator to find first match
            const auto it = std::find_if(options.begin(), options.end(),
                [&](const Option& o) {
                    return o.short_name == arg_short_name
                           and o.short_name != '\0';
                }
            );

            // no match
            if (it == options.end())
                throw std::runtime_error("Unknown option: -" + std::string(1, arg_short_name));

            // get value if needed
            std::string_view value;
            if (it->takes_value) {
                // attached value?
                if (it->allow_attached and arg_name.size() > 2) value = arg_name.substr(2);
                else {
                    if (i+1 >= argc)
                        throw std::runtime_error("Missing value for:  -" + std::string(1, arg_short_name));
                    value = argv[++i];
                }
            }

            // perform action specified for this option
            it->action(config, value);

        // positional arguement
        } else {
            config.input_files.push_back(std::string(arg_name));
        }
    }

    return config;
}
