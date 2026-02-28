//
// Created by dominik on 7/30/25.
//

#ifndef SOURCE_MANAGER_HPP
#define SOURCE_MANAGER_HPP

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cassert>
#include <vector>
#include <algorithm>

#include "source_location.hpp"
#include "util.hpp"

/**
 * The source manager is initialized with the path of the file it should manage. It will open the file and read in the
 * whole source code which is then stored as a string. The beginning of each line as a global offset from the start of
 * the source code will be precomputed for later lookup.
 */
class Source_manager {
private:
    std::string file_name;                    ///< name of the managed file
    std::string file_path;                    ///< full path to the file
    std::string file_extension;               ///< extension of the file
    std::string full_path;                    ///< path + name + extension
    std::string source_code;                  ///< complete copy of the source code
    std::vector<std::size_t> line_beginnings; ///< stores the offset of the beginning of each line
    int status_ = 0;                          ///< indicates potentially encountered problems

    /**
     * Precompute the offset of each line from the beginning of the source code.
     */
    void compute_line_offsets() {
        line_beginnings.push_back(0);
        for (std::size_t i = 0; i < source_code.size(); ++i)
            if (source_code[i] == '\n') line_beginnings.push_back(i + 1);
    }

public:
    /**
     * Takes the path to the file that should be managed, opens it, reads in the content, closes it ,and precomputes
     * offsets of each line. If something goes wrong, status is set to -1.
     * @param fi file information of the managed file
     */
    explicit Source_manager(const File_info& fi) {
        file_name = fi.file_name;
        file_path = fi.path;
        file_extension = fi.extension;
        full_path = file_path + file_name + file_extension;

        // preprocess file using the GCC preprocessor
        const std::string preprocessed_file = fi.path + fi.file_name + ".i";
        const std::string preprocess_cmd
            = "gcc -x c -E -P " + fi.path + fi.file_name + fi.extension + " -o " + preprocessed_file;
        int status = std::system(preprocess_cmd.c_str());
        if (status != 0) {
            std::cerr << "The source manager encountered an unexpected problem when preprocessing file at "
                      <<  fi.path << fi.file_name << fi.extension << std::endl;
        }

        std::ifstream infile(preprocessed_file);

        if (!infile.is_open() or infile.fail()) {
            std::cout << "Could not open file " << file_name << std::endl;
            status_ = -1;
        }

        std::stringstream buffer;
        buffer << infile.rdbuf();
        infile.close();

        // delete preprocessed file
        status = std::system(("rm -f " + preprocessed_file).c_str());
        if (status != 0) {
            std::cerr << "The source manager encountered an unexpected problem when deleting a preprocessed file at "
                      <<  preprocessed_file << std::endl;
        }

        source_code = buffer.str();

        compute_line_offsets();
    }

    /**
     * Returns a const reference to the source code.
     * @return const reference to the source code
     */
    [[nodiscard]] const std::string& get_source() const { return source_code; }

    /**
     * Returns the name of the managed file.
     * @return name of the managed file
     */
    [[nodiscard]] const std::string& get_file_name() const { return full_path; }

    /**
     * Returns the status of the source manager.
     * @return status of the source manager
     */
    [[nodiscard]] int status() const { return status_; }

    /**
     * Computes a Source_location from a given offset.
     * @param offset offset of the location from the start of the source code
     * @return source location object corresponding to the offset
     */
    [[nodiscard]] Source_location get_location(const std::size_t offset) const {
        assert(offset <= source_code.size());

        // binary search line start offsets
        const auto it = std::upper_bound(line_beginnings.begin(), line_beginnings.end(), offset);
        const std::size_t line = std::distance(line_beginnings.begin(), it);
        const std::size_t line_start = line_beginnings[line - 1];
        const std::size_t column = offset - line_start + 1;

        return Source_location{ file_name, offset, line, column };
    }

    /**
     * Fetches a string_view to the line with the given line number (1-based).
     * @param line line number of the line to be fetched
     * @return string_view of the line with the given line number
     */
    [[nodiscard]] std::string_view get_line(const std::size_t line) const {
        assert(line >= 1 && line <= line_beginnings.size());

        const std::size_t start = line_beginnings[line - 1];
        std::size_t end = (line < line_beginnings.size())
                          ? line_beginnings[line]
                          : source_code.size();

        // trim trailing newline if present
        if (end > start && source_code[end - 1] == '\n') --end;
        if (end > start && source_code[end - 1] == '\r') --end;

        return { &source_code[start], end - start };
    }

    /**
     * Returns the total number of lines in the source code.
     * @return total number of lines in the source code
     */
    [[nodiscard]] std::size_t total_lines() const {
        return line_beginnings.size();
    }
};

#endif //SOURCE_MANAGER_HPP
