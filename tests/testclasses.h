#pragma once
#include <gtest/gtest.h>
#include "../obs_ptr/obs_ptr.h"

class SimpleObsTargetTestClass : public IMemObservee {
    int a = 1;
};