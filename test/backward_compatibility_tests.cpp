#include <gtest/gtest.h>

#include "../include/combustion_event_controller.h"
#include "../include/engine.h"
#include "../include/fuel.h"
#include "../include/gas_system.h"
#include "../include/units.h"

TEST(BackwardCompatibilityTests, EngineUsesSparkIgnitionByDefault) {
    Engine engine;

    ASSERT_NE(engine.getCombustionEventController(), nullptr);
    EXPECT_EQ(
        engine.getCombustionEventController()->getType(),
        CombustionEventController::Type::SparkIgnition);
}

TEST(BackwardCompatibilityTests, FuelKeepsOriginalGasolineChemistryDefaults) {
    Fuel::Parameters parameters;
    Fuel fuel;
    fuel.initialize(parameters);

    EXPECT_DOUBLE_EQ(fuel.getMolecularAfr(), 25.0 / 2.0);
    EXPECT_DOUBLE_EQ(fuel.getMolecularOxygenRatio(), 25.0 / 2.0);
    EXPECT_DOUBLE_EQ(
        fuel.getProductMoleRatio(),
        (16.0 + 18.0) / (25.0 + 2.0));
}

TEST(BackwardCompatibilityTests, DefaultReactionMatchesGasolineParameters) {
    GasSystem::Mix mixture;
    mixture.p_fuel = 1.0 / 13.5;
    mixture.p_inert = 0.0;
    mixture.p_o2 = 12.5 / 13.5;

    GasSystem defaultReaction;
    GasSystem explicitReaction;
    defaultReaction.initialize(
        units::pressure(1.0, units::atm),
        units::volume(1.0, units::L),
        units::kelvin(700.0),
        mixture);
    explicitReaction.initialize(
        units::pressure(1.0, units::atm),
        units::volume(1.0, units::L),
        units::kelvin(700.0),
        mixture);

    const double defaultFuelBurned =
        defaultReaction.react(defaultReaction.n(), mixture);
    const double explicitFuelBurned = explicitReaction.react(
        explicitReaction.n(),
        mixture,
        25.0 / 2.0,
        (16.0 + 18.0) / (25.0 + 2.0));

    EXPECT_NEAR(defaultFuelBurned, explicitFuelBurned, 1E-12);
    EXPECT_NEAR(defaultReaction.n(), explicitReaction.n(), 1E-12);
    EXPECT_NEAR(defaultReaction.n_fuel(), explicitReaction.n_fuel(), 1E-12);
    EXPECT_NEAR(defaultReaction.n_o2(), explicitReaction.n_o2(), 1E-12);
    EXPECT_NEAR(defaultReaction.n_inert(), explicitReaction.n_inert(), 1E-12);
}
