#ifndef ATG_ENGINE_SIM_DIESEL_INJECTION_MODULE_H
#define ATG_ENGINE_SIM_DIESEL_INJECTION_MODULE_H

#include "combustion_event_controller.h"
#include "crankshaft.h"
#include "function.h"
#include "units.h"

class Engine;

class DieselInjectionModule : public CombustionEventController {
    public:
        struct Parameters {
            Engine *engine = nullptr;
            Crankshaft *crankshaft = nullptr;
            Function *injectionTimingCurve = nullptr;
            int cylinderCount = 0;

            double revLimit = units::rpm(4500.0);
            double limiterDuration = 0.1 * units::sec;
            double idleFuelMass = units::mass(0.002, units::g);
            double maxFuelMass = units::mass(0.060, units::g);
            double injectionDuration = units::angle(18.0, units::deg);
            double ignitionDelay = 0.001 * units::sec;
            double cetaneNumber = 50.0;
            double minimumIgnitionTemperature = units::kelvin(650.0);
            double premixedBurnFraction = 0.25;
            double premixedBurnDuration = units::angle(8.0, units::deg);
            double diffusionBurnDuration = units::angle(45.0, units::deg);
            double combustionNoise = 0.15;
        };

        struct Injector {
            double angle = 0.0;
            bool enabled = false;
            Event event;
        };

    public:
        DieselInjectionModule();
        virtual ~DieselInjectionModule();

        void initialize(const Parameters &params);
        virtual void destroy() override;

        void setFiringOrder(int cylinderIndex, double angle);
        virtual void reset() override;
        virtual void update(double dt) override;
        virtual Event getCombustionEvent(int cylinderIndex) const override;
        virtual void resetCombustionEvents() override;
        virtual double getTimingAdvance() override;
        virtual Type getType() const override { return Type::CompressionIgnition; }

    protected:
        Engine *m_engine;
        Crankshaft *m_crankshaft;
        Function *m_injectionTimingCurve;
        Injector *m_injectors;
        int m_cylinderCount;

        double m_lastCrankshaftAngle;
        double m_revLimit;
        double m_revLimitTimer;
        double m_limiterDuration;
        double m_idleFuelMass;
        double m_maxFuelMass;
        double m_injectionDuration;
        double m_ignitionDelay;
        double m_cetaneNumber;
        double m_minimumIgnitionTemperature;
        double m_premixedBurnFraction;
        double m_premixedBurnDuration;
        double m_diffusionBurnDuration;
        double m_combustionNoise;
};

#endif /* ATG_ENGINE_SIM_DIESEL_INJECTION_MODULE_H */
