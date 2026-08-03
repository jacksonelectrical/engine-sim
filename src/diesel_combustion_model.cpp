#include "../include/diesel_combustion_model.h"

#include "../include/units.h"
#include "../include/utilities.h"

#include <cmath>

double DieselCombustionModel::ignitionDelay(
    double referenceDelay,
    double temperature,
    double pressure,
    double cetaneNumber)
{
    const double T = std::fmax(temperature, units::kelvin(300.0));
    const double P = std::fmax(pressure, units::pressure(1.0, units::atm));
    const double cetane = std::fmax(cetaneNumber, 1.0);

    const double temperatureFactor =
        std::exp(2200.0 * (1.0 / T - 1.0 / units::kelvin(850.0)));
    const double pressureFactor = std::pow(
        units::pressure(40.0, units::atm) / P,
        0.7);
    const double cetaneFactor = 50.0 / cetane;

    return clamp(
        referenceDelay * temperatureFactor * pressureFactor * cetaneFactor,
        0.0001 * units::sec,
        0.0500 * units::sec);
}

double DieselCombustionModel::wiebeFraction(double normalizedProgress) {
    const double x = clamp(normalizedProgress);
    return 1.0 - std::exp(-6.9 * std::pow(x, 3.0));
}

double DieselCombustionModel::stagedBurnFraction(
    double burnAngle,
    double premixedBurnFraction,
    double premixedBurnDuration,
    double diffusionBurnDuration)
{
    const double premixed = premixedBurnDuration > 0.0
        ? wiebeFraction(burnAngle / premixedBurnDuration)
        : 1.0;
    const double diffusion = diffusionBurnDuration > 0.0
        ? wiebeFraction(burnAngle / diffusionBurnDuration)
        : 1.0;
    const double premixedWeight = clamp(premixedBurnFraction);

    return
        premixedWeight * premixed
        + (1.0 - premixedWeight) * diffusion;
}
