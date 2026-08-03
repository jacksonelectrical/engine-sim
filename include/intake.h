#ifndef ATG_ENGINE_SIM_INTAKE_H
#define ATG_ENGINE_SIM_INTAKE_H

#include "part.h"

#include "gas_system.h"

class Intake : public Part {
    public:
        struct Parameters {
            // Plenum volume
            double volume;

            // Plenum dimensions
            double CrossSectionArea;

            // Input flow constant
            double InputFlowK;

            // Idle-circuit flow constant
            double IdleFlowK;

            // Flow rate from plenum to runner
            double RunnerFlowRate;

            // Molecular air fuel ratio (defaults to ideal for octane)
            double MolecularAfr = (25.0 / 2.0);

            // Throttle plate position at idle
            double IdleThrottlePlatePosition = 0.975;

            // Runner volume
            double RunnerLength = units::distance(4.0, units::inch);

            // Velocity decay factor
            double VelocityDecay = 0.5;

            // Maximum compressor outlet pressure above atmosphere
            double MaxBoostPressure = 0.0;

            // Engine speed range over which boost becomes available
            double SpoolStartSpeed = units::rpm(1000.0);
            double SpoolFullSpeed = units::rpm(3000.0);

            // First-order compressor response time
            double SpoolTime = units::sec * 0.4;

            // Adiabatic compressor efficiency
            double CompressorEfficiency = 0.72;
        };

    public:
        Intake();
        virtual ~Intake();

        void initialize(Parameters &params);
        virtual void destroy();

        void process(double dt);
        void setEngineSpeed(double speed) { m_engineSpeed = speed; }
        void setBoostCommand(double command);

        inline double getRunnerFlowRate() const { return m_runnerFlowRate; }
        inline double getThrottlePlatePosition() const { return m_idleThrottlePlatePosition * m_throttle; }
        inline double getRunnerLength() const { return m_runnerLength; }
        inline double getPlenumCrossSectionArea() const { return m_crossSectionArea; }
        inline double getVelocityDecay() const { return m_velocityDecay; }
        inline double getBoostPressure() const { return m_boostPressure; }
        inline double getCompressorOutletPressure() const {
            return units::pressure(1.0, units::atm) + m_boostPressure;
        }
        inline double getCompressorOutletTemperature() const {
            return m_compressorOutletTemperature;
        }
        inline bool isForcedInductionEnabled() const {
            return m_maxBoostPressure > 0.0;
        }

        GasSystem m_system;
        double m_throttle;
        bool m_directInjection;

        double m_flow;
        double m_flowRate;
        double m_totalFuelInjected;

    protected:
        double m_crossSectionArea;
        double m_inputFlowK;
        double m_idleFlowK;
        double m_runnerFlowRate;
        double m_molecularAfr;
        double m_idleThrottlePlatePosition;
        double m_runnerLength;
        double m_velocityDecay;

        double m_maxBoostPressure;
        double m_spoolStartSpeed;
        double m_spoolFullSpeed;
        double m_spoolTime;
        double m_compressorEfficiency;
        double m_engineSpeed;
        double m_boostCommand;
        double m_boostPressure;
        double m_compressorOutletTemperature;

        GasSystem m_atmosphere;
};

#endif /* ATG_ENGINE_SIM_INTAKE_H */
