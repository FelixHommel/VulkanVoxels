#ifndef SRC_TESTS_HELPER_RANDOM_NUMBER_GENERATOR_HPP
#define SRC_TESTS_HELPER_RANDOM_NUMBER_GENERATOR_HPP

#include <chrono>
#include <limits>
#include <random>
#include <type_traits>
#include <unordered_set>

namespace vv::test
{

/// \brief Simple helper function to avoid constantly recreating empty sets as default parameter for generateRandom()
///
/// \author Felix Hommel
/// \date 12/2/2025
template<typename T>
const std::unordered_set<T>& emptySetProvider()
{
    static const std::unordered_set<T> empty{};
    return empty;
}

/// \brief Generate a random number in [min, max], can generate for integer and floating point types.
///
/// When the exclusion list is used, be careful to not exclude all possible values. This *can* cause an infinite loop.
///
/// \param min (optional) the minimum value the random number has to have
/// \param max (optional) the maximum value the random number has to have
/// \param exclusions (optional) exclude specific numbers. Careful when using with floating point values
///
/// \author Felix Hommel
/// \date 12/2/2025
template<typename T>
    requires(std::is_integral_v<T> || std::is_floating_point_v<T>)
T generateRandom(
    T min = std::numeric_limits<T>::min(),
    T max = std::numeric_limits<T>::max(),
    const std::unordered_set<T>& exclusions = emptySetProvider<T>()
)
{
    thread_local std::mt19937 mt{
        static_cast<std::seed_seq::result_type>(std::chrono::steady_clock::now().time_since_epoch().count())
    };

    if constexpr(std::is_integral_v<T>)
    {
        std::uniform_int_distribution<T> dist(min, max);
        while(true)
        {
            if(T val{ dist(mt) }; !exclusions.contains(val))
                return val;
        }
    }
    else
    {
        std::uniform_real_distribution<T> dist(min, max);
        while(true)
        {
            // NOTE: For floating point values this is only limited useful since the exact value needs to be excluded
            if(T val{ dist(mt) }; !exclusions.contains(val))
                return val;
        }
    }
}

} // namespace vv::test

#endif // !SRC_TESTS_HELPER_RANDOM_NUMBER_GENERATOR_HPP
