// Parsing of very simple functional language
// Additionally we use lexy's visualization to print the AST.

#include <iostream>
#include <vector>
#include <memory>

// Import lexy stuff
#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy/input/string_input.hpp>

// Utils for collecting ast
#include <lexy/parse_tree.hpp>
#include <lexy/action/parse_as_tree.hpp>

// Shell and error reporting
#include <lexy_ext/report_error.hpp>
#include <lexy_ext/shell.hpp>


// definitions of data structures
namespace lang
{
    // Any expression that can be evaluated
    struct expr
    {
        virtual ~expr() = default;
        virtual std::string to_str() const = 0;
    };

    using expr_ptr = std::shared_ptr<expr>;

    // Anonymous function
    // Defined as fn arg => body
    struct lambda : expr
    {
        std::string arg_name;
        expr_ptr body;
        lambda(std::string arg_name, expr_ptr body) : arg_name(arg_name), body(body) {}
        std::string to_str() const override
        {
            return "(fn " + arg_name + " => " + body->to_str() + ")";
        }
    };

    struct identifier : expr
    {
        std::string name;
        identifier(std::string name) : name(name) {}
        std::string to_str() const override
        {
            return name;
        }
    };

    // Function call
    // We use haskell syntax: f x y z
    // But for better readability we print it as f(x, y, z)
    struct call : expr
    {
        std::vector<expr_ptr> args;
        call(std::vector<expr_ptr> args) : args(args) {}
        std::string to_str() const override
        {
            if (args.size() == 1)
                return args[0]->to_str();
            else
                return many_args_to_str();
        }
        
        // If there are many args display in parenthesis
        std::string many_args_to_str() const 
        {
            std::string result = args[0]->to_str() + "(";
            for (int i = 1; i < args.size(); i++) 
            {
                result += args[i]->to_str();
                if (i != args.size() - 1)
                    result += ", ";
            }
            result += ")";
            return result;
        }
    };

    // top-level expression - here we allow only let definitions
    struct statement
    {
        std::string name;
        expr_ptr value;
        statement(std::string name, expr_ptr value) : name(name), value(value) {}
        std::string to_str() const
        {
            return "let " + name + " = " + value->to_str();
        }

    };
}

namespace grammar
{
    namespace dsl = lexy::dsl;


    // forward declaration for recursive grammar
    struct expr;

    namespace keywords
    {
        // For lexy keyword is an identifier that matches exactly without trailing characters
        // Therefore we have to specify how to parse identifier
        // Instead of doing that every time we can use macro
        #define KEYWORD(str) LEXY_KEYWORD(str, dsl::identifier(dsl::ascii::word))

        static constexpr auto fn = KEYWORD("fn");
        static constexpr auto let = KEYWORD("let");
    }


    struct identifier
    {
        static constexpr auto rule = dsl::identifier(dsl::ascii::word);
        static constexpr auto value = lexy::as_string<std::string>;
    };

    struct lambda
    {
        static constexpr auto rule = keywords::fn + dsl::p<identifier> + LEXY_LIT("=>") + dsl::recurse<expr>;
        static constexpr auto value = lexy::new_<lang::lambda, lang::expr_ptr>;
    };

    struct term
    {
        // choice requires branch-rule so to parse lambda we first peek for fn and then commit to it
        static constexpr auto parse_lambda = dsl::peek(keywords::fn) >> dsl::p<lambda>;
        static constexpr auto rule = parse_lambda | dsl::p<identifier> | dsl::parenthesized(dsl::recurse<expr>);

        // choose the best constructor for parsed term
        static constexpr auto value = lexy::callback<lang::expr_ptr>(
            lexy::forward<lang::expr_ptr>,
            lexy::new_<lang::identifier, lang::expr_ptr>
        );
    };

    struct expr
    {
        // Because we allow newlines in the middle of expression
        // we have to use branch-rule inside list.
        // Therefore we define a condition that peeks for term
        static constexpr auto term_start = dsl::peek(keywords::fn) | dsl::peek(dsl::p<identifier>) | dsl::peek(LEXY_LIT("("));
        static constexpr auto rule = dsl::list(term_start >> dsl::p<term>);
        
        // Composition of callbacks - we first convert parsed list into vector and then construct call
        static constexpr auto value = lexy::as_list<std::vector<lang::expr_ptr>> >> lexy::new_<lang::call, lang::expr_ptr>;
    };

    struct statement
    {
        // Allow spaces between tokens
        static constexpr auto whitespace = dsl::ascii::space;
        static constexpr auto rule = keywords::let + dsl::p<identifier> + LEXY_LIT("=") + dsl::p<expr> + dsl::eof;
        static constexpr auto value = lexy::construct<lang::statement>;
    };
}


int main()
{
    while (true)
    {
        std::cout << ">>> ";
        std::string line;
        std::getline(std::cin, line);
        auto input = lexy::string_input(line);

        lexy::parse_tree_for<decltype(input)> tree;
        auto result = lexy::parse_as_tree<grammar::statement>(tree, input, lexy_ext::report_error);
        lexy::visualize(stdout, tree, {lexy::visualize_fancy});

        auto parsed = lexy::parse<grammar::statement>(input, lexy::noop);
        if (parsed.has_value())
        {
            std::cout << parsed.value().to_str() << "\n";
        }

    }
}