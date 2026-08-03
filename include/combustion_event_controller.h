#ifndef ATG_ENGINE_SIM_COMBUSTION_EVENT_CONTROLLER_H
#define ATG_ENGINE_SIM_COMBUSTION_EVENT_CONTROLLER_H

#include "part.h"

class CombustionEventController : public Part {
    public:
        enum class Type {
            SparkIgnition,
            CompressionIgnition
        };

        struct Event {
            enum class Kind {
                Spark,
                DieselInjection
            };

            Kind kind = Kind::Spark;
            bool active = false;
            double fuelMass = 0.0;
            double injectionDuration = 0.0;
            double ignitionDelay = 0.0;
            double cetaneNumber = 50.0;
            double minimumIgnitionTemperature = 0.0;
            double premixedBurnFraction = 0.0;
            double premixedBurnDuration = 0.0;
            double diffusionBurnDuration = 0.0;
            double combustionNoise = 0.0;
        };

    public:
        CombustionEventController() : m_enabled(false) { /* void */ }
        virtual ~CombustionEventController() { /* void */ }

        virtual void reset() = 0;
        virtual void update(double dt) = 0;
        virtual Event getCombustionEvent(int cylinderIndex) const = 0;
        virtual void resetCombustionEvents() = 0;
        virtual double getTimingAdvance() = 0;
        virtual Type getType() const = 0;

        bool m_enabled;
};

#endif /* ATG_ENGINE_SIM_COMBUSTION_EVENT_CONTROLLER_H */
