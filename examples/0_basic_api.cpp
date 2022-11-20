// Example of basic API structure
// This includes defining simple grammar rules and using them to parse a simple input.

#include <iostream>


#include <lexy/dsl.hpp> // lexy::dsl::* for the grammar rules
#include <lexy/callback.hpp> // lexy::callback for value construction
#include <lexy/input/string_input.hpp> // lexy::string_input for reading from a string
#include <lexy_ext/report_error.hpp> // lexy::report_error for error reporting
#include <lexy/action/parse.hpp> // lexy::parse for the parsing

// First we define structures that will store the parsed data.
enum class Direction
{
    Left, 
    Right,
};

// Callback that will be used to transform parsed data
int dir_to_int(Direction dir)
{
    switch (dir)
    {
        case Direction::Left: return -1;
        case Direction::Right: return 1;
    }
    return 0;
}

// Then we define the grammar rules - to avoid colllisions with other structures
// we put them in a `grammar` namespace. 
namespace grammar
{
    // in grammar we define production rules
    struct left
    {
        // Every production has to define a `rule` member that tells lexy how to parse it.
        static constexpr auto rule = LEXY_LIT("left");
        // In addition we have to define a `value` member that tells lexy what to do with parsed data
        static constexpr auto value = lexy::constant(Direction::Left);
    };

    struct right
    {
        static constexpr auto rule = LEXY_LIT("right");
        static constexpr auto value = lexy::constant(Direction::Right);
    };

    struct direction
    {
        // Rules can be combined using provided combinators - for example alternative
        static constexpr auto rule = lexy::dsl::p<left> | lexy::dsl::p<right>;
        // And value can be constructed using callback
        static constexpr auto value = lexy::callback<int>(&dir_to_int);
    };
};

void parse_and_print(std::string s)
{
    // To parse data we have to create an input object
    // In our case we tell lexy that our input is std::string
    auto input = lexy::string_input(s);
    // Then we parse, specyfing grammar rule, provide the input and error handler
    auto result = lexy::parse<grammar::direction>(input, lexy_ext::report_error);
    // Finally, if parsing was successful we can print the result
    if (result.has_value())
        std::cout << "Parsed `" << s << "` to " << result.value() << std::endl;
    else
        std::cout << "Failed to parse direction `" << s << "`" << std::endl;
}

int main()
{
    parse_and_print("left");
    parse_and_print("right");
    parse_and_print("up");
}