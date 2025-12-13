#include "utility/Utils.hpp"
#include "helper/RandomNumberGenerator.hpp"

#include "gtest/gtest.h"

#include <cstddef>
#include <limits>

namespace vv::test
{

class UtilsTestParameterized : public ::testing::TestWithParam<std::size_t>
{
public:
    UtilsTestParameterized() = default;
    ~UtilsTestParameterized() override = default;

    UtilsTestParameterized(const UtilsTestParameterized&) = delete;
    UtilsTestParameterized(UtilsTestParameterized&&) = delete;
    UtilsTestParameterized& operator=(const UtilsTestParameterized&) = delete;
    UtilsTestParameterized& operator=(UtilsTestParameterized&&) = delete;

    static constexpr std::size_t DEFAULT_SEED{ 0 };
    static constexpr std::size_t MAX_SEED{ std::numeric_limits<std::size_t>::max() };
    static constexpr std::size_t MIN_SEED{ std::numeric_limits<std::size_t>::min() };

protected:
    static constexpr auto VAL_1{ 1.f };
    static constexpr auto VAL_2{ 2.f };
    static constexpr auto VAL_3{ 3.f };
};

TEST_P(UtilsTestParameterized, HashCombine)
{
    std::size_t seed1{ GetParam() };
    hashCombine(seed1, VAL_1, VAL_2, VAL_3);

    std::size_t seed2{ DEFAULT_SEED };
    hashCombine(seed2, VAL_1, VAL_2, VAL_3);

    ASSERT_NE(seed1, seed2);

    std::size_t seed1Duplicate{ GetParam() };
    hashCombine(seed1Duplicate, VAL_1, VAL_2, VAL_3);

    ASSERT_EQ(seed1Duplicate, seed1);
}

INSTANTIATE_TEST_SUITE_P(
    HashCombine,
    UtilsTestParameterized,
    ::testing::Values(
        generateRandom<std::size_t>(
            UtilsTestParameterized::MIN_SEED, UtilsTestParameterized::MAX_SEED, { UtilsTestParameterized::DEFAULT_SEED }
        ),
        generateRandom<std::size_t>(
            UtilsTestParameterized::MIN_SEED, UtilsTestParameterized::MAX_SEED, { UtilsTestParameterized::DEFAULT_SEED }
        ),
        generateRandom<std::size_t>(
            UtilsTestParameterized::MIN_SEED, UtilsTestParameterized::MAX_SEED, { UtilsTestParameterized::DEFAULT_SEED }
        )
    )
);

} // namespace vv::test
