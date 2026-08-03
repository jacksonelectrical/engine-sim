#include <gtest/gtest.h>

#include "../include/constants.h"
#include "../include/diesel_injection_module.h"
#include "../include/engine.h"
#include "../include/function.h"
#include "../include/units.h"

TEST(DieselInjectionModuleTests, SchedulesFuelByCylinderAndDriverDemand) {
    Engine engine;
    engine.setThrottle(0.0);

    Crankshaft crankshaft;
    Crankshaft::Parameters crankshaftParameters;
    crankshaftParameters.mass = 1.0;
    crankshaftParameters.flywheelMass = 1.0;
    crankshaftParameters.momentOfInertia = 1.0;
    crankshaftParameters.crankThrow = 1.0;
    crankshaftParameters.rodJournals = 0;
    crankshaftParameters.tdc = 0.0;
    crankshaft.initialize(crankshaftParameters);
    crankshaft.m_body.theta = 0.0;
    crankshaft.m_body.v_theta = -units::rpm(1200.0);

    Function timing;
    timing.initialize(2, 1.0);
    timing.addSample(0.0, 0.0);
    timing.addSample(units::rpm(5000.0), 0.0);

    DieselInjectionModule module;
    DieselInjectionModule::Parameters parameters;
    parameters.engine = &engine;
    parameters.crankshaft = &crankshaft;
    parameters.injectionTimingCurve = &timing;
    parameters.cylinderCount = 1;
    parameters.idleFuelMass = units::mass(0.002, units::g);
    parameters.maxFuelMass = units::mass(0.060, units::g);
    module.initialize(parameters);
    module.setFiringOrder(0, units::angle(5.0, units::deg));
    module.m_enabled = true;
    module.reset();

    crankshaft.m_body.theta = -units::angle(10.0, units::deg);
    module.update(0.001);

    const CombustionEventController::Event event =
        module.getCombustionEvent(0);
    EXPECT_TRUE(event.active);
    EXPECT_EQ(
        event.kind,
        CombustionEventController::Event::Kind::DieselInjection);
    EXPECT_NEAR(event.fuelMass, parameters.maxFuelMass, 1E-12);

    module.resetCombustionEvents();
    EXPECT_FALSE(module.getCombustionEvent(0).active);

    module.destroy();
    timing.destroy();
    crankshaft.destroy();
}

TEST(DieselInjectionModuleTests, FuelCutStopsEventsAboveRevLimit) {
    Engine engine;
    engine.setThrottle(0.0);

    Crankshaft crankshaft;
    Crankshaft::Parameters crankshaftParameters;
    crankshaftParameters.mass = 1.0;
    crankshaftParameters.flywheelMass = 1.0;
    crankshaftParameters.momentOfInertia = 1.0;
    crankshaftParameters.crankThrow = 1.0;
    crankshaftParameters.rodJournals = 0;
    crankshaft.initialize(crankshaftParameters);
    crankshaft.m_body.v_theta = -units::rpm(5000.0);

    Function timing;
    timing.initialize(2, 1.0);
    timing.addSample(0.0, 0.0);
    timing.addSample(units::rpm(6000.0), 0.0);

    DieselInjectionModule module;
    DieselInjectionModule::Parameters parameters;
    parameters.engine = &engine;
    parameters.crankshaft = &crankshaft;
    parameters.injectionTimingCurve = &timing;
    parameters.cylinderCount = 1;
    parameters.revLimit = units::rpm(4000.0);
    module.initialize(parameters);
    module.setFiringOrder(0, units::angle(5.0, units::deg));
    module.m_enabled = true;
    module.reset();

    crankshaft.m_body.theta = -units::angle(10.0, units::deg);
    module.update(0.001);
    module.resetCombustionEvents();

    crankshaft.m_body.theta = -4.0 * constants::pi;
    module.update(0.001);
    EXPECT_FALSE(module.getCombustionEvent(0).active);

    module.destroy();
    timing.destroy();
    crankshaft.destroy();
}
