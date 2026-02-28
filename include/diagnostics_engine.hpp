//
// Created by dominik on 7/31/25.
//

#ifndef DIAGNOSTICS_ENGINE_HPP
#define DIAGNOSTICS_ENGINE_HPP

#include <cstdlib>
#include <string>
#include <unistd.h>
#include <format>

#include "source_manager.hpp"
#include "error_kinds.hpp"
#include "token.hpp"

enum class Severity {
    Note = 0,
    Warning = 1,
    Error = 2,
};

/**
 * Engine that handles all diagnostics that are printed to the stream specified during creation of the engine.
 * It can only report about kinds of diagnostics that are listed in 'error_kinds.hpp', where both the name of the
 * error/warning/note and a format string to the respective message is specified.
 * If possible, the output will be color coded and include information like line numbers and a caret showing the exact
 * position in the line where the issue was reported.
 * The Engine keeps track of the distance to the last reported error to suppress a cascade of errors when the parser
 * encounters syntactical errors.
 */
class Diagnostics_engine {
private:
    Source_manager& source_manager; ///< source manager that keeps a copy of the source code
    std::ostream& os;               ///< outstream to print diagnostics to
    int note_count;                 ///< tracks total number of notes encountered
    int warning_count;              ///< tracks total number of warnings encountered
    int error_count;                ///< tracks total number of errors encountered

    int consecutive_errors = 100;   ///< tracks number os consecutive errors to suppress cascades
    int throttle_limit = 3;         ///< how many consecutive errors are suppressed

    std::string red;                ///< creates red output (if possible)
    std::string bold_red;           ///< creates bold red output (if possible)
    std::string green;              ///< creates green output (if possible)
    std::string bold_green;         ///< creates bold green output (if possible)
    std::string yellow;             ///< creates yellow output (if possible)
    std::string bold_yellow;        ///< creates bold yellow output (if possible)
    std::string blue;               ///< creates blue output (if possible)
    std::string bold_blue;          ///< creates bold blue output (if possible)
    std::string bold;               ///< creates bold output (if possible)
    std::string ansi_reset;         ///< resets any formating of the output

public:
    /**
     * Takes a source manager for the corresponding file and initializes colored output if the terminal supports it.
     * @param sm source manager with source code
     */
    explicit Diagnostics_engine(Source_manager& sm)
        : source_manager(sm), os(std::cerr) {

        note_count = 0;
        warning_count = 0;
        error_count = 0;

        // check whether to use ansi colors for output
        const bool use_ansi = support_ansi_color(os);
        red         = use_ansi ? "\033[0;31m" : "";
        bold_red    = use_ansi ? "\033[1;31m" : "";
        green       = use_ansi ? "\033[0;32m" : "";
        bold_green  = use_ansi ? "\033[1;32m" : "";
        yellow      = use_ansi ? "\033[0;33m" : "";
        bold_yellow = use_ansi ? "\033[1;33m" : "";
        blue        = use_ansi ? "\033[0;34m" : "";
        bold_blue   = use_ansi ? "\033[1;34m" : "";
        bold        = use_ansi ? "\033[1m" : "";
        ansi_reset  = use_ansi ? "\033[0;0m" : "";
    }

    /**
     * Helper function that determines whether the output stream supports the use of ANSI color codes.
     * @return true if output supports ANSI color codes
     */
    [[nodiscard]] static bool support_ansi_color(const std::ostream& os) {
        // terminal or not (maybe pipe into file)
        if (os.rdbuf() == std::cerr.rdbuf()) return isatty(STDERR_FILENO);
        if (os.rdbuf() == std::cout.rdbuf()) return isatty(STDOUT_FILENO);
        return false;
    }

    /**
     * Main function to report any issue. Output and internal processes like error-report throttling are dictated by the
     * 'level' that indicates the severity of the error.
     * @tparam Args template for variadic arguments for format string
     * @param level indicates severity of issue: Note, Warning or Error
     * @param loc Location of the token that will be marked with a caret
     * @param ek error kind, specifies the content and format of the message that is reported
     * @param args variadic arguments for format string
     * @return 'true' if the error was reported (and not suppressed)
     */
    template<typename... Args>
    bool report_issue(const Severity level, const Source_location& loc, const Error_kind& ek, Args&&... args) {
        return report_issue(level, Token(Token_type::None, loc), ek, std::forward<Args>(args)...);
    }

    /**
     * Main function to report any issue. Output and internal processes like error-report throttling are dictated by the
     * 'level' that indicates the severity of the error.
     * @tparam Args template for variadic arguments for format string
     * @param level indicates severity of issue: Note, Warning or Error
     * @param token Token that will be marked with a caret
     * @param ek error kind, specifies the content and format of the message that is reported
     * @param args variadic arguments for format string
     * @return 'true' if the error was reported (and not suppressed)
     */
    template<typename... Args>
    bool report_issue(const Severity level, const Token& token, const Error_kind& ek, Args&&... args) {
        // set level dependent values
        std::string color, bold_color, name;
        if (level == Severity::Note) { name="note", color = blue; bold_color = bold_blue; ++note_count; }
        else if (level == Severity::Warning) { name="warning", color = yellow; bold_color = bold_yellow; ++warning_count; }
        else if (level == Severity::Error) { name="error", color = red; bold_color = bold_red; ++error_count; }

        // don't report cascading errors back to back, throttle them instead
        //if (level == Severity::Error) {
        //    if (consecutive_errors >= throttle_limit) consecutive_errors = 0;
        //    else { ++consecutive_errors; return false; }
        //}

        // format and print error message
        os << bold_color << name << ": " << ansi_reset;
        const std::string& fmt = error_map.at(ek);
        os << std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...)) << "\n";

        // print location information
        os << "in file " << bold << token.location.file_name << ansi_reset
           << " at " << bold << token.location.line << ":" << token.location.column << ansi_reset << std::endl;

        // print line where error occurred
        const auto& error_line = source_manager.get_line(token.location.line);
        os << std::format("  {:6} | ", token.location.line);
        const auto tok_loc_col = std::max(1UL, token.location.column) - 1;
        os << error_line.substr(0, tok_loc_col)
           << color << error_line.substr(tok_loc_col, token.val.size()) << ansi_reset
           << error_line.substr(tok_loc_col + token.val.size()) << "\n";

        // add caret under token
        std::string caret = "           ";
        for (size_t i=0; i<tok_loc_col; i++) caret += " ";
        caret += "~";
        for (size_t i=1; i<token.val.size(); i++) caret += "~";
        os << color << caret << ansi_reset << std::endl;

        return true;
    }

    /**
     * Returns the number of actual errors (level = 2) that were reported.
     * @return number of errors that were reported
     */
    [[nodiscard]] int status() const { return error_count; }
};

#endif //DIAGNOSTICS_ENGINE_HPP
