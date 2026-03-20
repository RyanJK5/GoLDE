#ifndef BigInt_hpp_
#define BigInt_hpp_

#include <boost/multiprecision/cpp_int.hpp>
#include <compare>

#include "Graphics2D.hpp"

namespace gol {
using BigInt = boost::multiprecision::cpp_int;

const inline BigInt BigZero{};
const inline BigInt BigOne{1};

inline std::strong_ordering operator<=>(const BigInt& lhs, const BigInt& rhs) {
    const auto cmp = lhs.compare(rhs);
    if (cmp < 0) {
        return std::strong_ordering::less;
    }
    if (cmp > 0) {
        return std::strong_ordering::greater;
    }
    return std::strong_ordering::equal;
}

using BigVec2 = GenericVec<BigInt>;
} // namespace gol

namespace std {
template <>
struct formatter<gol::BigInt> {
    bool UseLocale = false;

    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it == 'L') {
            UseLocale = true;
            ++it;
        }
        return it;
    }

    auto format(const gol::BigInt& num, auto& ctx) const {
        const std::string str{num.str()};
        std::string_view sv{str};

        const bool negative = !sv.empty() && sv[0] == '-';
        if (negative) {
            sv.remove_prefix(1);
        }

        const auto& loc = ctx.locale();
        const auto& punct = std::use_facet<std::numpunct<char>>(loc);
        const std::string grouping = punct.grouping();
        const auto seperator = punct.thousands_sep();

        auto out = ctx.out();
        if (negative) {
            *out++ = '-';
        }

        if (!UseLocale || grouping.empty()) {
            return std::copy(sv.begin(), sv.end(), out);
        }

        auto digitsToSeperator = sv.size() % 3UZ;
        if (digitsToSeperator == 0) {
            digitsToSeperator = 3;
        }

        for (auto i = 0UZ; i < sv.size(); i++) {
            if (i > 0UZ && digitsToSeperator == 0UZ) {
                *out++ = seperator;
                digitsToSeperator = 3UZ;
            }
            *out++ = sv[i];
            digitsToSeperator--;
        }

        return out;
    }
};
} // namespace std

#endif