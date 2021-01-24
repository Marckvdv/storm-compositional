#include "test/storm_gtest.h"
#include "storm-config.h"

#include <storm/utility/random.h>

namespace {

    TEST(SamplingTest, SampleExponential) {
        boost::mt19937 gen(5u);
        storm::utility::ExponentialDistributionGenerator dist(5);
        
        // Ensure that pseudo random numbers are the same on all machines
        double reference[] = {0.18241937154, 0.0522078772595, 0.0949721368604, 0.246869315378, 0.765000791199, 0.0177096648877, 0.225167598601, 0.23538530391, 1.01605360643, 0.138846355094};
        for (int i =0; i < 10; ++i) {
            EXPECT_FLOAT_EQ(dist.random(gen), reference[i]);
        }
    }

}
