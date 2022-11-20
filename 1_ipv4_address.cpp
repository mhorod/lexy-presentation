// Example of parsing IPv4 address with lexy

#include <iostream>

#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy/input/string_input.hpp>
#include <lexy_ext/report_error.hpp>

struct ipv4_address 
{
    unsigned char octets[4];

    ipv4_address(unsigned char a, unsigned char b, unsigned char c, unsigned char d)
    {
        octets[0] = a;
        octets[1] = b;
        octets[2] = c;
        octets[3] = d;
    }
};

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
        // dsl::p parses a rule contained in a struct and produces its value
        // dsl::times can be used to produce a tuple of values
        // it takes a rule and optional separator
        static constexpr auto rule = dsl::times<4>(dsl::p<octet>, dsl::sep(dsl::period));

        // Here we can't use callback because we can't take reference to constructor
        // Thankfully lexy provides built-in way for doing that
        static constexpr auto value = lexy::construct<::ipv4_address>;
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