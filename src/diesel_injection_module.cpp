#include "../include/diesel_injection_module.h"

#include "../include/constants.h"
#include "../include/engine.h"
#include "../include/utilities.h"

#include <cassert>
#include <cmath>

DieselInjectionModule::DieselInjectionModule() {
    m_engine = nullptr;
    m_crankshaft = nullptr;
    m_injectionTimingCurve = nullptr;
    m_injectors = nullptr;
    m_cylinderCount = 0;
    m_lastCrankshaftAngle = 0.0;
    m_revLimit = 0.0;
    m_revLimitTimer = 0.0;
    m_limiterDuration = 0.0;
    m_idleFuelMass = 0.0;
    m_maxFuelMass = 0.0;
    m_injectionDuration = 0.0;
    m_ignitionDelay = 0.0;
    m_cetaneNumber = 50.0;
    m_minimumIgnitionTemperature = 0.0;
    m_premixedBurnFraction = 0.0;
    m_premixedBurnDuration = 0.0;
    m_diffusionBurnDuration = 0.0;
    m_combustionNoise = 0.0;
}

DieselInjectionModule::~DieselInjectionModule() {
    assert(m_injectors == nullptr);
}

void DieselInjectionModule::initialize(const Parameters &params) {
    assert(m_injectors == nullptr);

    m_engine = params.engine;
    m_crankshaft = params.crankshaft;
    m_injectionTimingCurve = params.injectionTimingCurve;
    m_cylinderCount = params.cylinderCount;
    m_revLimit = params.revLimit;
    m_limiterDuration = params.limiterDuration;
    m_idleFuelMass = params.idleFuelMass;
    m_maxFuelMass = params.maxFuelMass;
    m_injectionDuration = params.injectionDuration;
    m_ignitionDelay = params.ignitionDelay;
    m_cetaneNumber = params.cetaneNumber;
    m_minimumIgnitionTemperature = params.minimumIgnitionTemperature;
    m_premixedBurnFraction = clamp(params.premixedBurnFraction);
    m_premixedBurnDuration = params.premixedBurnDuration;
    m_diffusionBurnDuration = params.diffusionBurnDuration;
    m_combustionNoise = params.combustionNoise;

    m_injectors = new Injector[m_cylinderCount];
}

void DieselInjectionModule::destroy() {
    if (m_injectors != nullptr) delete[] m_injectors;

    m_injectors = nullptr;
    m_cylinderCount = 0;
    m_engine = nullptr;
    m_crankshaft = nullptr;
    m_injectionTimingCurve = nullptr;
}

void DieselInjectionModule::setFiringOrder(int cylinderIndex, double angle) {
    assert(cylinderIndex >= 0 && cylinderIndex < m_cylinderCount);

    m_injectors[cylinderIndex].angle = angle;
    m_injectors[cylinderIndex].enabled = true;
}

void DieselInjectionModule::reset() {
    m_lastCrankshaftAngle = m_crankshaft->getCycleAngle();
    m_revLimitTimer = 0.0;
    resetCombustionEvents();
}

void DieselInjectionModule::update(double dt) {
    const double cycleAngle = m_crankshaft->getCycleAngle();

    if (m_enabled && m_revLimitTimer == 0.0) {
        const double fourPi = 4.0 * constants::pi;
        const double advance = getTimingAdvance();
        const double driverDemand = clamp(1.0 - m_engine->getThrottle());
        const double fuelMass =
            m_idleFuelMass + driverDemand * (m_maxFuelMass - m_idleFuelMass);

        for (int i = 0; i < m_cylinderCount; ++i) {
            Injector &injector = m_injectors[i];
            double adjustedAngle = positiveMod(injector.angle - advance, fourPi);
            const double r0 = m_lastCrankshaftAngle;
            double r1 = cycleAngle;
            bool event = false;

            if (m_crankshaft->m_body.v_theta < 0.0) {
                if (r1 < r0) {
                    r1 += fourPi;
                    adjustedAngle += fourPi;
                }

                event = adjustedAngle >= r0 && adjustedAngle < r1;
            }
            else {
                if (r1 > r0) {
                    r1 -= fourPi;
                    adjustedAngle -= fourPi;
                }

                event = adjustedAngle >= r1 && adjustedAngle < r0;
            }

            if (event && injector.enabled) {
                injector.event.kind = Event::Kind::DieselInjection;
                injector.event.active = true;
                injector.event.fuelMass = fuelMass;
                injector.event.injectionDuration = m_injectionDuration;
                injector.event.ignitionDelay = m_ignitionDelay;
                injector.event.cetaneNumber = m_cetaneNumber;
                injector.event.minimumIgnitionTemperature = m_minimumIgnitionTemperature;
                injector.event.premixedBurnFraction = m_premixedBurnFraction;
                injector.event.premixedBurnDuration = m_premixedBurnDuration;
                injector.event.diffusionBurnDuration = m_diffusionBurnDuration;
                injector.event.combustionNoise = m_combustionNoise;
            }
        }
    }

    m_revLimitTimer -= dt;
    if (std::fabs(m_crankshaft->m_body.v_theta) > m_revLimit) {
        m_revLimitTimer = m_limiterDuration;
    }
    if (m_revLimitTimer < 0.0) m_revLimitTimer = 0.0;

    m_lastCrankshaftAngle = cycleAngle;
}

CombustionEventController::Event DieselInjectionModule::getCombustionEvent(
    int cylinderIndex) const
{
    assert(cylinderIndex >= 0 && cylinderIndex < m_cylinderCount);
    return m_injectors[cylinderIndex].event;
}

void DieselInjectionModule::resetCombustionEvents() {
    for (int i = 0; i < m_cylinderCount; ++i) {
        m_injectors[i].event.active = false;
    }
}

double DieselInjectionModule::getTimingAdvance() {
    if (m_injectionTimingCurve == nullptr) return 0.0;
    return m_injectionTimingCurve->sampleTriangle(-m_crankshaft->m_body.v_theta);
}
