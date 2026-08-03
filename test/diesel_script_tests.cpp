#include <gtest/gtest.h>

#include "../include/constants.h"
#include "../include/units.h"
#include "../scripting/include/compiler.h"

#include <cmath>

TEST(DieselScriptTests, ReferenceConfigurationCompilesAndBuildsDieselEngine) {
    es_script::Compiler compiler;
    compiler.initialize();

    const bool compiled = compiler.compile("../assets/diesel-main.mr");
    EXPECT_TRUE(compiled);
    if (!compiled) {
        compiler.destroy();
        return;
    }

    const es_script::Compiler::Output output = compiler.execute();
    EXPECT_NE(output.engine, nullptr);
    EXPECT_NE(output.transmission, nullptr);
    EXPECT_NE(output.vehicle, nullptr);
    if (output.engine != nullptr) {
        EXPECT_EQ(output.engine->getCylinderCount(), 1);
        EXPECT_EQ(
            output.engine->getCombustionEventController()->getType(),
            CombustionEventController::Type::CompressionIgnition);
        EXPECT_TRUE(output.engine->getIntake(0)->m_directInjection);
        EXPECT_TRUE(output.engine->getIntake(0)->isForcedInductionEnabled());

        Engine *engine = output.engine;
        Crankshaft *crankshaft = engine->getOutputCrankshaft();
        CombustionChamber *chamber = engine->getChamber(0);
        CombustionEventController *controller =
            engine->getCombustionEventController();

        GasSystem::Mix compressedAir;
        compressedAir.p_inert = 0.75;
        compressedAir.p_o2 = 0.25;
        chamber->m_system.initialize(
            units::pressure(30.0, units::atm),
            chamber->getVolume(),
            units::kelvin(900.0),
            compressedAir);
        crankshaft->m_body.v_theta = -units::rpm(1200.0);
        engine->setThrottle(0.0);
        controller->m_enabled = true;

        const double fourPi = 4.0 * constants::pi;
        double injectionAngle = std::fmod(
            -controller->getTimingAdvance(),
            fourPi);
        if (injectionAngle < 0.0) injectionAngle += fourPi;

        const double beforeInjection =
            injectionAngle - units::angle(1.0, units::deg);
        crankshaft->m_body.theta +=
            crankshaft->getCycleAngle() - beforeInjection;
        controller->reset();
        crankshaft->m_body.theta -= units::angle(2.0, units::deg);
        controller->update(0.001);

        const CombustionEventController::Event event =
            controller->getCombustionEvent(0);
        EXPECT_TRUE(event.active);
        EXPECT_EQ(
            event.kind,
            CombustionEventController::Event::Kind::DieselInjection);
        EXPECT_GT(event.fuelMass, 0.0);
        EXPECT_GT(event.combustionNoise, 0.0);

        if (
            event.active
            && event.kind
                == CombustionEventController::Event::Kind::DieselInjection)
        {
            chamber->beginDieselInjection(event);
            for (int i = 0; i < 200; ++i) {
                chamber->flow(0.0001);
            }

            EXPECT_NEAR(
                chamber->getTotalInjectedFuelMass(),
                event.fuelMass,
                event.fuelMass * 1E-6);
            EXPECT_GT(chamber->m_nBurntFuel, 0.0);
            EXPECT_GT(chamber->getLastTimestepDieselPressureRise(), 0.0);
            EXPECT_TRUE(std::isfinite(chamber->m_system.pressure()));
        }
    }

    compiler.destroy();
    if (output.engine != nullptr) {
        output.engine->destroy();
        delete output.engine;
    }
    delete output.transmission;
    delete output.vehicle;
}

TEST(DieselScriptTests, StockGasolineConfigurationStillCompiles) {
    es_script::Compiler compiler;
    compiler.initialize();

    const bool compiled = compiler.compile("../assets/main.mr");
    EXPECT_TRUE(compiled);
    if (!compiled) {
        compiler.destroy();
        return;
    }

    const es_script::Compiler::Output output = compiler.execute();
    EXPECT_NE(output.engine, nullptr);
    EXPECT_NE(output.transmission, nullptr);
    EXPECT_NE(output.vehicle, nullptr);
    if (output.engine != nullptr) {
        EXPECT_EQ(
            output.engine->getCombustionEventController()->getType(),
            CombustionEventController::Type::SparkIgnition);
        EXPECT_FALSE(output.engine->getIntake(0)->m_directInjection);
        EXPECT_FALSE(output.engine->getIntake(0)->isForcedInductionEnabled());
    }

    compiler.destroy();
    if (output.engine != nullptr) {
        output.engine->destroy();
        delete output.engine;
    }
    delete output.transmission;
    delete output.vehicle;
}
