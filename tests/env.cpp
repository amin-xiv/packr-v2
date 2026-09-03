#include <packr/types.hpp>
#include <packr/entry.hpp>

#include <gtest/gtest.h>
#include "shared_test_data.hpp"

using namespace packr;

class Environment : public ::testing::Environment, public packingAndUnpackingTestdata {
  public:
    void SetUp() override {
    }
};
