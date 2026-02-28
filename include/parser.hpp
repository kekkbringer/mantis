#ifndef PARSER_HPP
#define PARSER_HPP

#include <set>

#include "ast.hpp"
#include "source_manager.hpp"
#include "lexer.hpp"
#include "symbol_table.hpp"

/**
 * The Parser class is initialized with a source manager that keeps the whole source code as a string, the manager is
 * then passes to a lexer. Then, the parser will continuously request new tokens from the lexer and build the abstract
 * syntax tree from the source code. Whenever encountering a syntactically invalid construct, it will report an error
 * and try to resume parsing at the next safe place do to so.
 * The implementation resembles a recursive descent parser with precedence climbing to parse nested expressions.
 */
class Parser {
private:
    int error_dist;                 ///< distance to last error occurrence in tokens
    int error_count;                ///< tracks the number of errors encountered by the parser
    bool in_error_recovery;         ///< indicates whether parser is in error recovery mode
    Token current;                  ///< current token
    Token look_ahead;               ///< look ahead (next) token
    Token_type la_type;             ///< type of the look ahead token
    Source_manager& source_manager; ///< manages the source code for the lexer and diagnostics
    Diagnostics_engine& diag;       ///< responsible for error, warning and note reporting
    Lexer lexer;                    ///< lexer to provide tokens
    size_t mangle_counter = 0;      ///< for unique variable names
    Type_pool type_pool;            ///< allocator for different types
    std::string current_func;       ///< name of the functions that is currently parsed

    struct Label_info {
        bool defined;
        bool used;
        Source_location loc;
    };
    std::map<std::string, Label_info> label_table; ///< function scope table of all labels

    struct Control_target {
        enum class Kind {Loop, Switch} kind;
        std::string tag;
    };
    std::vector<Control_target> control_stack; ///< keeps track of loops and switches for labeling purposes

    using switch_cases = std::set<int>;
    std::vector<switch_cases> current_switch_cases;
    std::vector<bool> current_switch_has_default;

    Scope* current_scope = nullptr; ///< symbol table of the current scope (contains older scopes as parents)

    /**
     * This function requests a new token from the lexer and sets the internal values 'current', 'look_ahead' and
     * 'la_type'.
     */
    void scan() {
        current = look_ahead;
        look_ahead = lexer.next();
        la_type = look_ahead.type;
    }

    /**
     * Matches the type of the look ahead token with the given type, returns true and consumes the token in case of a
     * match. Otherwise, just returns false without consuming the token.
     * @param expected type of the token that is expected
     * @return 'true' if types match (also consumes token in this case), 'false' otherwise
     */
    bool match(const Token_type& expected) {
        if (la_type == expected) {
            scan();
            return true;
        }
        return false;
    }

    /**
     * Checks if the look ahead token is of the expected type, if it is, the function returns true and consumes the
     * token. Otherwise, an error is reported and the function returns false without consuming the token.
     * @param expected expected type of the look ahead token
     * @param ek kind of error to be reported in case of mismatch
     * @param args arguments for the precise error message
     * @return true if match (also consumes token), false otherwise (token not consumed, error reported)
     */
    template <typename... Args>
    bool expect(const Token_type& expected, const Error_kind& ek, Args&&... args) {
        if (la_type != expected) {
            diag.report_issue(Severity::Error, look_ahead, ek, std::forward<Args>(args)...);
            return false;
        }
        scan();
        return true;
    }

    /**
     * Enters error recovery mode.
     */
    void enter_error_recovery() {
        in_error_recovery = true;
    }

    /**
     * Exits error recovery mode.
     */
    void exit_error_recovery() {
        in_error_recovery = true;
    }

    /**
     * Enters a new scope by creating a new scope, setting its parent to the current scope and then moving the
     * current scope pointer to the newly created one.
     */
    void enter_new_scope() {
        auto new_scope = std::make_unique<Scope>(current_scope);
        current_scope->children.push_back(std::move(new_scope));
        current_scope = current_scope->children.back().get();
    }

    /**
     * Leaves the current scope by moving the current scope pointer to the parent of the current scope.
     */
    void leave_scope() {
        current_scope = current_scope->parent;
    }

    // methods to parse each syntax construct
    ast::Program parse_program();
    ast::Expression_ptr parse_expression(int min_prec);
    ast::Expression_ptr parse_factor();
    std::vector<ast::Variable_declaration_ptr> parse_param_list(std::vector<std::shared_ptr<Type>>& type_list);
    ast::Block parse_block(bool new_scope);
    ast::Statement_ptr parse_statement();
    ast::Statement_ptr parse_statement_inner();
    ast::For_init parse_for_init();
    std::vector<ast::Expression_ptr> parse_argument_list();
    void synchronize_stmt();
    ast::Declaration_ptr parse_declaration();
    ast::Declaration_ptr parse_declaration_inner();
    void synchronize_declaration();

    /**
     * This is the parser level wrapper function to report a syntax error and enter error recovery mode. Output and
     * internal processes like error-report throttling are dictated by the 'level' that indicates the severity of the
     * error. Internally handles error recovery mode management.
     * @tparam Args template for variadic arguments for format string
     * @param level indicates severity of issue: Note, Warning or Error
     * @param token Token that will be marked with a caret
     * @param ek error kind, specifies the content and format of the message that is reported
     * @param args variadic arguments for format string
     * @return 'true' if the error was reported (and not suppressed)
     */
    template<typename... Args>
    bool report_syntax_error(const Severity level, const Token& token, const Error_kind& ek, Args&&... args) {
        // if in error recovery mode, immediately leave and indicate that error was not reported
        if (in_error_recovery) return false;

        // enter recovery mode if necessary and call diagnostics engine
        if (level == Severity::Error) in_error_recovery = true;
        return diag.report_issue(level, token, ek, std::forward<Args>(args)...);
    }

    /**
     * This is the parser level wrapper function to report a syntax error and enter error recovery mode. Output and
     * internal processes like error-report throttling are dictated by the 'level' that indicates the severity of the
     * error. Internally handles error recovery mode management.
     * @tparam Args template for variadic arguments for format string
     * @param level indicates severity of issue: Note, Warning or Error
     * @param loc Location of the token that will be marked with a caret
     * @param ek error kind, specifies the content and format of the message that is reported
     * @param args variadic arguments for format string
     * @return 'true' if the error was reported (and not suppressed)
     */
    template<typename... Args>
    bool report_syntax_error(const Severity level, const Source_location& loc, const Error_kind& ek, Args&&... args) {
        return report_issue(level, Token(Token_type::None, loc), ek, std::forward<Args>(args)...);
    }

public:
    /**
     * This constructor takes the source code, initializes the lexer and all internal variables. After the constructor
     * is called 'current' will hold the first token of the input.
     * @param sm source manager that keeps the source code
     * @param de diagnostics engine to report notes, warnings and errors
     * @param fs pointer to the global/file scope
     */
    explicit Parser(Source_manager& sm, Diagnostics_engine& de, Scope* fs)
        : error_dist(100), error_count(0), in_error_recovery(false), la_type(Token_type::None),
          source_manager(sm), diag(de), lexer(source_manager, de), current_scope(fs) {
        scan();
    }

    /**
     * The main parser routine, this will start to parse a C program.
     */
    ast::Program parse();

    /**
     * Debug routine that prints the whole AST to std::cout
     */
    void print_AST();
    static void print_program(const ast::Program& prog);
};

#endif