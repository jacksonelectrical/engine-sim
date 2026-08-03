#ifndef ATG_ENGINE_SIM_DIESEL_COMBUSTION_MODEL_H
#define ATG_ENGINE_SIM_DIESEL_COMBUSTION_MODEL_H

class DieselCombustionModel {
    public:
        static double ignitionDelay(
            double referenceDelay,
            double temperature,
            double pressure,
            double cetaneNumber);

        static double wiebeFraction(double normalizedProgress);

        static double stagedBurnFraction(
            double burnAngle,
            double premixedBurnFraction,
            double premixedBurnDuration,
            double diffusionBurnDuration);
};

#endif /* ATG_ENGINE_SIM_DIESEL_COMBUSTION_MODEL_H */
