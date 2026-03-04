//
// Created by dominik on 7/22/25.
//

#include <iostream>
#include <sys/stat.h>

#include "util.hpp"
#include "version.hpp"

std::vector<File_info> parse_file_info(const std::vector<std::string>& input_files) {
    std::vector<File_info> file_info_vec;

    for (const auto& file_path: input_files) {
        File_info info;

        // find last occurrences of '/' and '.' to extract path, file name and extension
        const auto foundSlash = file_path.find_last_of('/');
        const auto foundDot   = file_path.find_last_of('.');
        if (foundSlash == std::string::npos) {
            info.path = "";
        } else {
            info.path = file_path.substr(0, foundSlash+1);
        }
        if (foundDot == std::string::npos) {
            info.extension = "";
        } else {
            info.extension = file_path.substr(foundDot);
        }

        info.file_name = file_path.substr(foundSlash+1, foundDot-foundSlash-1);

        if (info.file_name.empty()) {
            std::cout << "Invalid file name: " << file_path << std::endl;
            return file_info_vec;
        }

        if (info.extension != ".c" and info.extension != ".s" and info.extension != ".S") {
            std::cout << "Warning: Unknow file extension: " << info.extension << "\n";
            std::cout << "Treating it as a C-source file." << std::endl;
        }

        // check if the file actually exists
        struct stat buffer;
        if (stat(file_path.c_str(), &buffer) != 0) {
            std::cout << "Could not find file: " << file_path << std::endl;
            return file_info_vec;
        }

        file_info_vec.push_back(info);
        
    }

    return file_info_vec;
}
