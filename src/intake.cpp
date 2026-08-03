#include "../include/intake.h"

#include "../include/units.h"

#include <algorithm>
#include <cmath>

Intake::Intake() {
    m_inputFlowK = 0;
    m_idleFlowK = 0;
    m_flow = 0;
    m_throttle = 1.0;
    m_directInjection = false;
    m_idleThrottlePlatePosition = 0.0;
    m_crossSectionArea = 0.0;
    m_flowRate = 0;
    m_totalFuelInjected = 0;
    m_molecularAfr = 0;
    m_runnerLength = 0;
    m_velocityDecay = 0;
    m_maxBoostPressure = 0;
    m_spoolStartSpeed = 0;
    m_spoolFullSpeed = 0;
    m_spoolTime = 0;
    m_compressorEfficiency = 1.0;
    m_engineSpeed = 0;
    m_boostCommand = 0;
    m_boostPressure = 0;
    m_compressorOutletTemperature = units::celcius(25.0);
    m_turboReferenceExhaustFlow = 0;
    m_turboSoundVolume = 0;
    m_wastegateSoundVolume = 0;
    m_turboWhineFrequency = 5000;
    m_exhaustFlowRate = 0;
    m_turboShaftSpeed = 0;
    m_wastegatePosition = 0;
    m_turboSoundPhase = 0;
    m_boostReleaseEnvelope = 0;
    m_previousBoostCommand = 0;
    m_turboNoiseState = 0x6d2b79f5u;
}

Intake::~Intake() {
    /* void */
}

void Intake::initialize(Parameters &params) {
    const double width = std::sqrt(params.CrossSectionArea);
    m_system.initialize(
        units::pressure(1.0, units::atm),
        params.volume,
        units::celcius(25.0));
    m_system.setGeometry(
        width,
        params.volume / params.CrossSectionArea,
        1.0,
        0.0);

    m_atmosphere.initialize(
        units::pressure(1.0, units::atm),
        units::volume(1000.0, units::m3),
        units::celcius(25.0));
    m_atmosphere.setGeometry(
        units::distance(100.0, units::m),
        units::distance(100.0, units::m),
        1.0,
        0.0);

    m_inputFlowK = params.InputFlowK;
    m_molecularAfr = params.MolecularAfr;
    m_idleFlowK = params.IdleFlowK;
    m_idleThrottlePlatePosition = params.IdleThrottlePlatePosition;
    m_runnerLength = params.RunnerLength;
    m_crossSectionArea = params.CrossSectionArea;
    m_velocityDecay = params.VelocityDecay;
    m_runnerFlowRate = params.RunnerFlowRate;
    m_maxBoostPressure = std::max(0.0, params.MaxBoostPressure);
    m_spoolStartSpeed = std::max(0.0, params.SpoolStartSpeed);
    m_spoolFullSpeed = std::max(m_spoolStartSpeed, params.SpoolFullSpeed);
    m_spoolTime = std::max(0.0, params.SpoolTime);
    m_compressorEfficiency = std::clamp(
        params.CompressorEfficiency,
        0.01,
        1.0);
    m_turboReferenceExhaustFlow =
        std::max(0.0, params.TurboReferenceExhaustFlow);
    m_turboSoundVolume = std::max(0.0, params.TurboSoundVolume);
    m_wastegateSoundVolume =
        std::max(0.0, params.WastegateSoundVolume);
    m_turboWhineFrequency =
        std::max(100.0, params.TurboWhineFrequency);
    m_exhaustFlowRate = 0.0;
    m_turboShaftSpeed = 0.0;
    m_wastegatePosition = 0.0;
    m_turboSoundPhase = 0.0;
    m_boostReleaseEnvelope = 0.0;
    m_previousBoostCommand = 0.0;
    m_boostPressure = 0.0;
    m_compressorOutletTemperature = units::celcius(25.0);
}

void Intake::destroy() {
    /* void */
}

void Intake::setBoostCommand(double command) {
    const double nextCommand = std::clamp(command, 0.0, 1.0);
    const double commandDrop = m_previousBoostCommand - nextCommand;
    if (commandDrop > 0.05 && m_boostPressure > 0.0) {
        m_boostReleaseEnvelope = std::max(
            m_boostReleaseEnvelope,
            commandDrop * m_boostPressure / std::max(m_maxBoostPressure, 1.0));
    }
    m_previousBoostCommand = nextCommand;
    m_boostCommand = nextCommand;
}

void Intake::process(double dt) {
    const double ideal_afr = 0.8 * m_molecularAfr * 4;
    const double current_afr = (m_system.mix().p_o2 + m_system.mix().p_inert) / m_system.mix().p_fuel;

    const double p_air = ideal_afr / (1 + ideal_afr);
    GasSystem::Mix fuelAirMix;
    fuelAirMix.p_fuel = m_directInjection ? 0.0 : 1 - p_air;
    fuelAirMix.p_inert = m_directInjection ? 0.75 : p_air * 0.75;
    fuelAirMix.p_o2 = m_directInjection ? 0.25 : p_air * 0.25;

    const double idle_afr = 2.0;
    const double p_idle_air = idle_afr / (1 + idle_afr);
    GasSystem::Mix fuelMix;
    fuelMix.p_fuel = m_directInjection ? 0.0 : (1.0 - p_idle_air);
    fuelMix.p_inert = m_directInjection ? 0.75 : p_idle_air * 0.75;
    fuelMix.p_o2 = m_directInjection ? 0.25 : p_idle_air * 0.25;

    const double throttle = getThrottlePlatePosition();
    const double flowAttenuation = std::cos(throttle * constants::pi / 2);

    double spoolFraction = 0.0;
    if (isExhaustDrivenTurboEnabled()) {
        const double turbineDrive = std::clamp(
            m_exhaustFlowRate / m_turboReferenceExhaustFlow,
            0.0,
            1.35);
        const double targetShaftSpeed = std::sqrt(turbineDrive);
        if (m_spoolTime <= 0.0) {
            m_turboShaftSpeed = targetShaftSpeed;
        }
        else {
            const double response = 1.0 - std::exp(-dt / m_spoolTime);
            m_turboShaftSpeed +=
                (targetShaftSpeed - m_turboShaftSpeed) * response;
        }
        m_turboShaftSpeed = std::clamp(m_turboShaftSpeed, 0.0, 1.15);
        spoolFraction = m_turboShaftSpeed * m_turboShaftSpeed;
        m_wastegatePosition = std::clamp(
            (spoolFraction - 0.90) / 0.25,
            0.0,
            1.0);
    }
    else {
        if (m_spoolFullSpeed <= m_spoolStartSpeed) {
            spoolFraction = m_engineSpeed >= m_spoolStartSpeed ? 1.0 : 0.0;
        }
        else {
            spoolFraction = std::clamp(
                (m_engineSpeed - m_spoolStartSpeed)
                    / (m_spoolFullSpeed - m_spoolStartSpeed),
                0.0,
                1.0);
        }
        m_turboShaftSpeed = spoolFraction;
        m_wastegatePosition = 0.0;
    }

    const double targetBoost =
        m_maxBoostPressure * spoolFraction * m_boostCommand;
    if (m_spoolTime <= 0.0) {
        m_boostPressure = targetBoost;
    }
    else {
        const double response = 1.0 - std::exp(-dt / m_spoolTime);
        m_boostPressure += (targetBoost - m_boostPressure) * response;
    }
    m_boostPressure = std::clamp(
        m_boostPressure,
        0.0,
        m_maxBoostPressure);

    constexpr double heatCapacityRatio = 1.4;
    const double inletPressure = units::pressure(1.0, units::atm);
    const double inletTemperature = units::celcius(25.0);
    const double pressureRatio =
        (inletPressure + m_boostPressure) / inletPressure;
    m_compressorOutletTemperature = inletTemperature * (
        1.0
        + (std::pow(
            pressureRatio,
            (heatCapacityRatio - 1.0) / heatCapacityRatio) - 1.0)
            / m_compressorEfficiency);

    GasSystem::FlowParameters flowParams;
    flowParams.crossSectionArea_0 = units::area(10, units::m2);
    flowParams.crossSectionArea_1 = m_crossSectionArea;
    flowParams.direction_x = 0.0;
    flowParams.direction_y = -1.0;
    flowParams.dt = dt;

    m_atmosphere.reset(
        inletPressure + m_boostPressure,
        m_compressorOutletTemperature,
        fuelAirMix);
    flowParams.system_0 = &m_atmosphere;
    flowParams.system_1 = &m_system;
    flowParams.k_flow = flowAttenuation * m_inputFlowK;
    m_flow = m_system.flow(flowParams);

    m_atmosphere.reset(
        inletPressure + m_boostPressure,
        m_compressorOutletTemperature,
        fuelMix);
    flowParams.system_0 = &m_atmosphere;
    flowParams.system_1 = &m_system;
    flowParams.k_flow = m_idleFlowK;
    const double idleCircuitFlow = m_system.flow(flowParams);

    m_system.dissipateExcessVelocity();
    m_system.updateVelocity(dt, m_velocityDecay);

    if (m_flow > 0 && !m_directInjection) {
        m_totalFuelInjected += fuelAirMix.p_fuel * m_flow;
    }

    if (idleCircuitFlow > 0 && !m_directInjection) {
        m_totalFuelInjected += fuelMix.p_fuel * idleCircuitFlow;
    }
}

double Intake::sampleTurboSound(double dt) {
    if (!isForcedInductionEnabled() || dt <= 0.0) return 0.0;

    const double shaftSpeed = std::clamp(m_turboShaftSpeed, 0.0, 1.15);
    const double frequency = 250.0 + m_turboWhineFrequency * shaftSpeed;
    m_turboSoundPhase = std::fmod(
        m_turboSoundPhase + 2.0 * constants::pi * frequency * dt,
        2.0 * constants::pi);

    m_turboNoiseState ^= m_turboNoiseState << 13;
    m_turboNoiseState ^= m_turboNoiseState >> 17;
    m_turboNoiseState ^= m_turboNoiseState << 5;
    const double noise =
        2.0 * (static_cast<double>(m_turboNoiseState) / 4294967295.0) - 1.0;

    const double whine =
        (std::sin(m_turboSoundPhase)
            + 0.22 * std::sin(2.0 * m_turboSoundPhase))
        * m_turboSoundVolume * shaftSpeed * shaftSpeed;
    const double release = std::clamp(m_boostReleaseEnvelope, 0.0, 1.0);
    const double airNoise = noise * m_wastegateSoundVolume
        * std::clamp(m_wastegatePosition + release, 0.0, 1.0);

    m_boostReleaseEnvelope *= std::exp(-dt / 0.18);

    // Match the order of magnitude of the existing exhaust synthesizer input.
    return 9000.0 * whine + 14000.0 * airNoise;
}
