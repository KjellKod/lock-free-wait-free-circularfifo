#ifndef G2_RANDOM_H_
#define G2_RANDOM_H_

#include <random>
#include <functional>
#include <ctime>

// This should be split into header/c++ implementation or otherwise proteted

namespace g2 {
typedef unsigned int  Number;
//typedef long long int               TimeValue;

// Generate a random number using the 'mersenne twister distribution'   http://en.wikipedia.org/wiki/Mersenne_twister
// Random numbers are chosen within the range limits of 'low' and 'high'
// A random number can be retrieved like this:
// Number random_nbr = RandomNumber(0, 99);
auto RandomNumber = [](const Number& low, const Number& high) -> Number {
    std::uniform_int_distribution<int> distribution(low, high);
    std::mt19937 engine((unsigned int)time(0)); // Mersenne twister MT19937
    auto generator = std::bind(distribution, engine); // old-school using bind
    return generator();
};


// Similar to @ref RandomNumber lambda above but this one returning the actual generator function
// subsequent random numbers can be retrieved like this. This is "possibly" faster since the generator
// does not need to be re-created for each new random number retrieved.
// auto generator = RandomGenerator(0, 99);
// Number random_nbr = generator(); // a random number createds
 auto RandomGenerator = [](const Number& low, const Number& high) -> std::function<Number()> {
    std::uniform_int_distribution<int> distribution(low, high);
    std::mt19937 engine((unsigned int)time(0)); // Mersenne twister MT19937
    auto generator = std::bind(distribution, engine);
    return generator;
};

} // g2s

#endif //G2_RANDOM_H_