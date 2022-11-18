// Simple calculator

#include <iostream>
#include <memory>
#include <vector>

// Import lexy stuff
#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy/input/string_input.hpp>

#include <lexy_ext/report_error.hpp>
#include <lexy_ext/shell.hpp>

// definitions of data structures
namespace ast
{
    struct expr
    {
        virtual int evaluate() = 0;
        virtual ~expr() = default;
    };

    using expr_ptr = std::shared_ptr<expr>;

    struct literal : expr
    {
        int value;
        literal(int value) : value(value) {}
        int evaluate() { return value; }
    };

    enum class bin_op
    {
        add,
        sub,
        mul,
        div,
        pow,
    };

    struct bin_expr : expr
    {
        bin_op op;
        expr_ptr lhs, rhs;

        bin_expr(expr_ptr lhs, bin_op op, expr_ptr rhs) : op(op),
                                                          lhs(move(lhs)),
                                                          rhs(move(rhs))
        {
        }

        int evaluate()
        {
            auto l = lhs->evaluate();
            auto r = rhs->evaluate();
            switch (op)
            {
            case bin_op::add:
                return l + r;
            case bin_op::sub:
                return l - r;
            case bin_op::mul:
                return l * r;
            case bin_op::div:
                return l / r;
            case bin_op::pow:
                return power(l, r);
            }
        }

        int power(int base, int exponent)
        {
            int result = 1;
            for (int i = 0; i < exponent; i++)
                result *= base;
            return result;
        }
    };
};

namespace grammar
{
    namespace dsl = lexy::dsl;

    struct nested_expr : lexy::transparent_production
    {
        // Changing whitespace from blank to space so shell doesn't stop on new line
        static constexpr auto whitespace = dsl::ascii::space;
        static constexpr auto rule = dsl::recurse<struct expr>;
        static constexpr auto value = lexy::forward<ast::expr_ptr>;
    };

    // deriving from lexy default expression model
    struct expr : lexy::expression_production
    {
        struct expected_operand
        {
            static constexpr auto name = "expected operand";
        };

        struct integer
        {
            // Conditional and choice
            static constexpr auto rule = LEXY_LIT("0x") >> dsl::integer<int, dsl::hex> | dsl::integer<int>;
            static constexpr auto value = lexy::forward<int>;
        };

        static constexpr auto atom =
            dsl::parenthesized(dsl::p<nested_expr>) |
            dsl::p<integer> |
            dsl::error<expected_operand>;

        struct pow : dsl::infix_op_right
        {
            static constexpr auto op = dsl::op<ast::bin_op::pow>(LEXY_LIT("^"));
            using operand = dsl::atom;
        };

        struct mul : dsl::infix_op_left
        {
            static constexpr auto op = dsl::op<ast::bin_op::mul>(LEXY_LIT("*")) /
                                       dsl::op<ast::bin_op::div>(LEXY_LIT("/"));
            using operand = pow;
        };

        struct add : dsl::infix_op_left
        {
            static constexpr auto op = dsl::op<ast::bin_op::mul>(LEXY_LIT("+")) /
                                       dsl::op<ast::bin_op::div>(LEXY_LIT("-"));
            using operand = mul;
        };

        // operation with lowest priority that should be checked first
        using operation = add;

        static constexpr auto value =
            lexy::callback<ast::expr_ptr>(
                // Subexpressions
                lexy::forward<ast::expr_ptr>,
                // Literals
                lexy::new_<ast::literal, ast::expr_ptr>,
                // Binary expressions
                lexy::new_<ast::bin_expr, ast::expr_ptr>);
    };

    struct statement
    {
        static constexpr auto whitespace = dsl::ascii::blank;

        static constexpr auto rule = []
        {
            auto at_eol = dsl::peek(dsl::eol);
            return dsl::terminator(at_eol).opt_list(dsl::p<expr>, dsl::sep(dsl::semicolon));
        }();

        static constexpr auto value = lexy::as_list<std::vector<ast::expr_ptr>>;
    };

}

int main()
{
    lexy_ext::shell<lexy_ext::default_prompt<lexy::utf8_encoding>> shell;
    while (shell.is_open())
    {
        auto input = shell.prompt_for_input();
        auto parsed = lexy::parse<grammar::statement>(input, lexy_ext::report_error);

        if (parsed.has_value() && !parsed.value().empty())
        {
            for (auto &expr : parsed.value())
                std::cout << expr->evaluate() << "\n";
        }
    }
}