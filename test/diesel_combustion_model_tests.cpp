#include <gtest/gtest.h>

#include "../include/diesel_combustion_model.h"
#include "../include/units.h"

TEST(DieselCombustionModelTests, ReferenceConditionsReturnReferenceDelay) {
    const double referenceDelay = 0.0012 * units::sec;

    EXPECT_NEAR(
        DieselCombustionModel::ignitionDelay(
            referenceDelay,
            units::kelvin(850.0),
            units::pressure(40.0, units::atm),
            50.0),
        referenceDelay,
        1E-12);
}

TEST(DieselCombustionModelTests, HotterDenserHigherCetaneChargeIgnitesSooner) {
    const double referenceDelay = 0.0012 * units::sec;
    const double slowDelay = DieselCombustionModel::ignitionDelay(
        referenceDelay,
        units::kelvin(700.0),
        units::pressure(20.0, units::atm),
        40.0);
    const double fastDelay = DieselCombustionModel::ignitionDelay(
        referenceDelay,
        units::kelvin(950.0),
        units::pressure(60.0, units::atm),
        60.0);

    EXPECT_GT(slowDelay, referenceDelay);
    EXPECT_LT(fastDelay, referenceDelay);
    EXPECT_GT(slowDelay, fastDelay);
}

TEST(DieselCombustionModelTests, StagedHeatReleaseIsBoundedAndMonotonic) {
    const double premixedFraction = 0.28;
    const double premixedDuration = units::angle(9.0, units::deg);
    const double diffusionDuration = units::angle(48.0, units::deg);

    double previous = 0.0;
    for (int angleDegrees = 0; angleDegrees <= 48; ++angleDegrees) {
        const double fraction = DieselCombustionModel::stagedBurnFraction(
            units::angle(angleDegrees, units::deg),
            premixedFraction,
            premixedDuration,
            diffusionDuration);

        EXPECT_GE(fraction, previous);
        EXPECT_GE(fraction, 0.0);
        EXPECT_LE(fraction, 1.0);
        previous = fraction;
    }

    EXPECT_NEAR(
        DieselCombustionModel::stagedBurnFraction(
            0.0,
            premixedFraction,
            premixedDuration,
            diffusionDuration),
        0.0,
        1E-12);
    EXPECT_GT(previous, 0.99);
}
