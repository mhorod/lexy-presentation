// Example of parsing byte objects with lexy

#include <iostream>
#include <vector>

#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy_ext/report_error.hpp>
#include <lexy/input/buffer.hpp>

// Very simplified Ethernet frame.
// addresses are only one byte
// there is no checksum
struct ethernet_frame
{
    std::uint8_t  destination;
    std::uint8_t  source;
    std::uint16_t length;
    std::uint8_t  payload[1500];

    ethernet_frame(std::uint8_t destination, std::uint8_t source, std::vector<std::uint8_t> payload)
        : destination(destination), source(source), length(payload.size())
    {
        std::copy(payload.begin(), payload.end(), this->payload);
        this->payload[payload.size()] = 0;
    }
};

namespace grammar
{
    namespace dsl = lexy::dsl;

    struct address
    {
        static constexpr auto rule = dsl::bint8;
        static constexpr auto value = lexy::as_integer<std::uint8_t>;
    };

    struct length
    {
        static constexpr auto rule = dsl::big_bint16;
        static constexpr auto value = lexy::as_integer<std::uint16_t>;
    };

    struct payload
    {
        static constexpr auto rule = dsl::repeat(dsl::p<length>).list(dsl::bint8);
        static constexpr auto value = lexy::as_list<std::vector<std::uint8_t>>;
    };

    struct ethernet_frame
    {
        static constexpr auto rule = dsl::p<address> + dsl::p<address> + dsl::p<payload>;
        static constexpr auto value = lexy::construct<::ethernet_frame>;
    };

}

int main()
{
    // Buffer with "Hello, World" message
    std::uint8_t buffer[] = {42, 7, 0, 13, 'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!'};
    auto size = sizeof(buffer);

    auto input = lexy::make_buffer_from_raw<lexy::byte_encoding, lexy::encoding_endianness::big>(buffer, size);

    // Parse the input.
    auto result = lexy::parse<grammar::ethernet_frame>(input, lexy_ext::report_error);

    // Print the result.
    if (result.has_value())
    {
        auto frame = result.value();
        std::cout << "Destination: " << (int)frame.destination << "\n";
        std::cout << "Source: " << (int)frame.source << "\n";
        std::cout << "Length: " << frame.length << "\n";
        std::cout << "Payload: " << std::string((char*)frame.payload, frame.length) << "\n";
    }
}
