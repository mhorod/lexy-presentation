// Parsing of very simple functional language

#include <iostream>
#include <vector>
#include <memory>

// Import lexy stuff
#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy/input/string_input.hpp>
#include <lexy/parse_tree.hpp>
#include <lexy/action/parse_as_tree.hpp>

#include <lexy_ext/report_error.hpp>
#include <lexy_ext/shell.hpp>

namespace lang
{
    struct expr
    {
        virtual ~expr() = default;
        virtual std::string to_str() const = 0;
    };

    using expr_ptr = std::shared_ptr<expr>;

    struct lambda : expr
    {
        std::string arg_name;
        expr_ptr body;
        lambda(std::string arg_name, expr_ptr body) : arg_name(arg_name), body(body) {}
        std::string to_str() const override
        {
            return "fn " + arg_name + " => " + body->to_str();
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

    struct call : expr
    {
        std::vector<expr_ptr> args;
        call(std::vector<expr_ptr> args) : args(args) {}
        std::string to_str() const override
        {
            std::string result = "call:";
            for (auto& arg : args)
                result += " " + arg->to_str();
            return result;
        }
    };

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

    struct identifier
    {
        static constexpr auto rule = dsl::identifier(dsl::ascii::word);
        static constexpr auto value = lexy::as_string<std::string>;
    };

    struct lambda
    {
        static constexpr auto rule = LEXY_LIT("fn") + dsl::p<identifier> + LEXY_LIT("=>") + dsl::recurse<struct expr>;
        static constexpr auto value = lexy::new_<lang::lambda, lang::expr_ptr>;
    };

    struct term
    {
        static constexpr auto whitespace = dsl::ascii::blank;
        static constexpr auto parse_lambda = (dsl::peek(LEXY_LIT("fn") + dsl::ascii::blank) >> dsl::p<lambda>) ;
        static constexpr auto rule = parse_lambda | dsl::p<identifier> | dsl::parenthesized(dsl::recurse<struct expr>);
        static constexpr auto value = lexy::callback<lang::expr_ptr>(
            lexy::forward<lang::expr_ptr>,
            lexy::new_<lang::identifier, lang::expr_ptr>
        );
    };

    struct expr
    {
        static constexpr auto term_start = dsl::peek(LEXY_LIT("fn") + dsl::ascii::blank) | dsl::peek(dsl::p<identifier>) | dsl::peek(LEXY_LIT("("));
        static constexpr auto whitespace = dsl::newline;
        static constexpr auto rule = dsl::list(term_start >> dsl::p<term>);
        static constexpr auto value = lexy::as_list<std::vector<lang::expr_ptr>> >> lexy::new_<lang::call, lang::expr_ptr>;
    };

    struct statement
    {
        static constexpr auto whitespace = dsl::ascii::blank;
        static constexpr auto rule = LEXY_LIT("let") + dsl::p<identifier> + LEXY_LIT("=") + dsl::p<expr> + dsl::eof;
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