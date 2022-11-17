// Parsing ipv4 address
// They are in in format of four 8-bit numbers separated by dots

#include <iostream>

// Import lexy stuff
#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy/input/string_input.hpp>
#include <lexy_ext/report_error.hpp>

struct ipv4_address 
{
    unsigned char octets[4];
};

// We have to define separate method because constructors can't be referenced
ipv4_address make_ipv4_address(unsigned char a, unsigned char b, unsigned char c, unsigned char d)
{
    ipv4_address addr = {a, b, c, d};
    return addr;
}

namespace grammar
{
    namespace dsl = lexy::dsl;

    struct octet 
    {
        static constexpr auto rule = dsl::integer<unsigned char>;
        static constexpr auto value = lexy::as_integer<unsigned char>;
    };


    struct ipv4_address
    {
        static constexpr auto rule = dsl::times<4>(dsl::p<octet>, dsl::sep(dsl::period));
        static constexpr auto value = lexy::callback<::ipv4_address>(&make_ipv4_address);
    };
}

auto parse_ipv4_address_from_string(std::string s)
{
    auto input = lexy::string_input(s);
    return lexy::parse<grammar::ipv4_address>(input, lexy_ext::report_error);
}

std::ostream& operator<<(std::ostream& stream, const ipv4_address& address)
{
    for (size_t i = 0; i < 4; i++)
        stream << static_cast<int>(address.octets[i]) << (i < 3 ? "." : "");
    return stream;
}

int main()
{
    while (true)
    {
        std::string s;
        std::cout << "Enter an ipv4 address: ";
        std::cin >> s;
        auto parsed = parse_ipv4_address_from_string(s);
        if (parsed.has_value())
            std::cout << "Successfully parsed: " << parsed.value() << "\n";
    }
}