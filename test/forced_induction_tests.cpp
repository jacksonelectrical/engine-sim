#include <gtest/gtest.h>

#include "../include/intake.h"
#include "../include/units.h"

namespace {

Intake::Parameters baseIntakeParameters() {
    Intake::Parameters parameters;
    parameters.volume = units::volume(2.0, units::L);
    parameters.CrossSectionArea = units::area(100.0, units::cm2);
    parameters.InputFlowK = 0.0;
    parameters.IdleFlowK = 0.0;
    parameters.RunnerFlowRate = 0.0;
    return parameters;
}

} // namespace

TEST(ForcedInductionTests, NaturallyAspiratedIntakeKeepsAmbientSupply) {
    Intake::Parameters parameters = baseIntakeParameters();
    Intake intake;
    intake.initialize(parameters);
    intake.setEngineSpeed(units::rpm(4000.0));
    intake.setBoostCommand(1.0);

    intake.process(1.0);

    EXPECT_FALSE(intake.isForcedInductionEnabled());
    EXPECT_DOUBLE_EQ(intake.getBoostPressure(), 0.0);
    EXPECT_DOUBLE_EQ(
        intake.getCompressorOutletPressure(),
        units::pressure(1.0, units::atm));
}

TEST(ForcedInductionTests, BoostSpoolsAndStaysBelowWastegateLimit) {
    Intake::Parameters parameters = baseIntakeParameters();
    parameters.MaxBoostPressure = units::pressure(100.0, units::kPa);
    parameters.SpoolStartSpeed = units::rpm(1000.0);
    parameters.SpoolFullSpeed = units::rpm(2000.0);
    parameters.SpoolTime = 0.1;
    parameters.CompressorEfficiency = 0.72;

    Intake intake;
    intake.initialize(parameters);
    intake.setEngineSpeed(units::rpm(2500.0));
    intake.setBoostCommand(1.0);

    for (int i = 0; i < 100; ++i) {
        intake.process(0.01);
    }

    EXPECT_TRUE(intake.isForcedInductionEnabled());
    EXPECT_GT(
        intake.getBoostPressure(),
        units::pressure(99.0, units::kPa));
    EXPECT_LE(intake.getBoostPressure(), parameters.MaxBoostPressure);
    EXPECT_GT(
        intake.getCompressorOutletTemperature(),
        units::celcius(25.0));
}

TEST(ForcedInductionTests, BoostRequiresSpeedAndLoad) {
    Intake::Parameters parameters = baseIntakeParameters();
    parameters.MaxBoostPressure = units::pressure(100.0, units::kPa);
    parameters.SpoolStartSpeed = units::rpm(1000.0);
    parameters.SpoolFullSpeed = units::rpm(2000.0);
    parameters.SpoolTime = 0.0;

    Intake intake;
    intake.initialize(parameters);
    intake.setBoostCommand(1.0);
    intake.setEngineSpeed(units::rpm(500.0));
    intake.process(0.01);
    EXPECT_DOUBLE_EQ(intake.getBoostPressure(), 0.0);

    intake.setEngineSpeed(units::rpm(2000.0));
    intake.setBoostCommand(0.0);
    intake.process(0.01);
    EXPECT_DOUBLE_EQ(intake.getBoostPressure(), 0.0);
}
