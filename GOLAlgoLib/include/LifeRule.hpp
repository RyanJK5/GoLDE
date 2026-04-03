#ifndef LifeRule_hpp_
#define LifeRule_hpp_
#include <array>
#include <expected>
#include <string_view>

namespace gol {
class LifeRule {
  public:
    constexpr static uint32_t NumLeafPatterns = 1 << 16;

    constexpr static std::expected<LifeRule, std::string_view>
    Make(std::string_view ruleString);

    constexpr LifeRule(int32_t birthMask, int32_t surviveMask);
    constexpr auto RuleTable() const;

  private:
    constexpr auto BuildRuleTable(int32_t birthMask, int32_t surviveMask);

  private:
    std::array<uint16_t, NumLeafPatterns> m_RuleTable;
};

constexpr std::expected<LifeRule, std::string_view>
LifeRule::Make(std::string_view ruleString) {
    auto birthMask = 0;
    auto surviveMask = 0;

    const auto slash = ruleString.find('/');
    if (slash == std::string_view::npos || ruleString[0] != 'B' ||
        ruleString[slash + 1] != 'S')
        return std::unexpected{
            std::string_view{"Rule string must be in B.../S... format"}};

    for (char c : ruleString.substr(1, slash - 1)) {
        if (c < '0' || c > '8') {
            return std::unexpected{
                std::string_view{"Invalid birth neighbor count"}};
        }
        birthMask |= (1 << (c - '0'));
    }
    for (char c : ruleString.substr(slash + 2)) {
        if (c < '0' || c > '8') {
            return std::unexpected{
                std::string_view{"Invalid survive neighbor count"}};
        }
        surviveMask |= (1 << (c - '0'));
    }

    return LifeRule{birthMask, surviveMask};
}

constexpr auto LifeRule::BuildRuleTable(int32_t birthMask,
                                        int32_t surviveMask) {
    // The four center cells of a 4x4 grid whose next state we compute.
    constexpr std::array centerCol{1, 2, 1, 2};
    constexpr std::array centerRow{1, 1, 2, 2};
    // Where each center cell's result is placed in the output.
    constexpr std::array resultBit{5, 4, 1, 0};
    // Returns the bit position (0-15) for a cell at (col, row) in a 4x4 grid.
    constexpr auto bitPosition = [](int32_t col, int32_t row) {
        return (3 - row) * 4 + (3 - col);
    };
    std::array<uint16_t, NumLeafPatterns> table{};
    for (uint32_t pattern = 0; pattern < NumLeafPatterns; ++pattern) {
        uint16_t result = 0;
        for (int cell = 0; cell < 4; ++cell) {
            const int col = centerCol[cell];
            const int row = centerRow[cell];
            int neighborCount = 0;
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    if (dx == 0 && dy == 0)
                        continue;
                    neighborCount += static_cast<int>(
                        (pattern >> bitPosition(col + dx, row + dy)) & 1);
                }
            }
            const bool isAlive = ((pattern >> bitPosition(col, row)) & 1) != 0;
            const bool survives =
                isAlive && ((surviveMask >> neighborCount) & 1);
            const bool born = !isAlive && ((birthMask >> neighborCount) & 1);
            if (survives || born)
                result |= static_cast<uint16_t>(1 << resultBit[cell]);
        }
        table[pattern] = result;
    }
    return table;
}

constexpr auto LifeRule::RuleTable() const { return m_RuleTable; }

constexpr LifeRule::LifeRule(int32_t birthMask, int32_t surviveMask)
    : m_RuleTable(BuildRuleTable(birthMask, surviveMask)) {}

} // namespace gol
#endif