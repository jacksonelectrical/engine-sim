#ifndef ATG_ENGINE_SIM_IGNITION_MODULE_H
#define ATG_ENGINE_SIM_IGNITION_MODULE_H

#include "combustion_event_controller.h"

#include "crankshaft.h"
#include "function.h"
#include "units.h"

class IgnitionModule : public CombustionEventController {
    public:
        struct Parameters {
            int cylinderCount;
            Crankshaft *crankshaft;
            Function *timingCurve;
            double revLimit = units::rpm(6000.0);
            double limiterDuration = 0.5 * units::sec;
        };

        struct SparkPlug {
            double angle = 0;
            bool ignitionEvent = false;
            bool enabled = false;
        };

    public:
        IgnitionModule();
        virtual ~IgnitionModule();

        virtual void destroy() override;

        void initialize(const Parameters &params);
        void setFiringOrder(int cylinderIndex, double angle);
        virtual void reset() override;
        virtual void update(double dt) override;

        bool getIgnitionEvent(int index) const;
        void resetIgnitionEvents();

        virtual Event getCombustionEvent(int cylinderIndex) const override {
            Event event;
            event.kind = Event::Kind::Spark;
            event.active = getIgnitionEvent(cylinderIndex);
            return event;
        }
        virtual void resetCombustionEvents() override {
            resetIgnitionEvents();
        }

        virtual double getTimingAdvance() override;
        virtual Type getType() const override { return Type::SparkIgnition; }

    protected:
        SparkPlug *getPlug(int i);

        Function *m_timingCurve;
        SparkPlug *m_plugs;
        Crankshaft *m_crankshaft;
        int m_cylinderCount;

        double m_lastCrankshaftAngle;
        double m_revLimit;
        double m_revLimitTimer;
        double m_limiterDuration;
};

#endif /* ATG_ENGINE_SIM_IGNITION_MODULE_H */
