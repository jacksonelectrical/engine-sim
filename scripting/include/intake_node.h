#ifndef ATG_ENGINE_SIM_INTAKE_NODE_H
#define ATG_ENGINE_SIM_INTAKE_NODE_H

#include "object_reference_node.h"

#include "engine_context.h"
#include "function_node.h"

#include "engine_sim.h"

#include <map>
#include <vector>

namespace es_script {

    class IntakeNode : public ObjectReferenceNode<IntakeNode> {
    public:
        IntakeNode() { /* void */ }
        virtual ~IntakeNode() { /* void */ }

        Intake *generate(EngineContext *context) {
            Intake *intake = context->getIntake(this);
            Intake::Parameters parameters = m_parameters;
            intake->initialize(parameters);

            return intake;
        }

    protected:
        virtual void registerInputs() {
            addInput("plenum_volume", &m_parameters.volume);
            addInput("plenum_cross_section_area", &m_parameters.CrossSectionArea);
            addInput("intake_flow_rate", &m_parameters.InputFlowK);
            addInput("idle_flow_rate", &m_parameters.IdleFlowK);
            addInput("runner_flow_rate", &m_parameters.RunnerFlowRate);
            addInput("molecular_afr", &m_parameters.MolecularAfr);
            addInput("idle_throttle_plate_position", &m_parameters.IdleThrottlePlatePosition);
            addInput("throttle_gamma", &m_throttleGammaUnused);
            addInput("runner_length", &m_parameters.RunnerLength);
            addInput("velocity_decay", &m_parameters.VelocityDecay);
            addInput("max_boost_pressure", &m_parameters.MaxBoostPressure);
            addInput("spool_start_rpm", &m_parameters.SpoolStartSpeed);
            addInput("spool_full_rpm", &m_parameters.SpoolFullSpeed);
            addInput("spool_time", &m_parameters.SpoolTime);
            addInput("compressor_efficiency", &m_parameters.CompressorEfficiency);
            addInput(
                "turbo_reference_exhaust_flow",
                &m_parameters.TurboReferenceExhaustFlow);
            addInput("turbo_sound_volume", &m_parameters.TurboSoundVolume);
            addInput(
                "wastegate_sound_volume",
                &m_parameters.WastegateSoundVolume);
            addInput(
                "turbo_whine_frequency",
                &m_parameters.TurboWhineFrequency);

            ObjectReferenceNode<IntakeNode>::registerInputs();
        }

        virtual void _evaluate() {
            setOutput(this);

            // Read inputs
            readAllInputs();
        }

        double m_throttleGammaUnused = 0.0; // Deprecated; to be removed in a future release
        Intake::Parameters m_parameters;
    };

} /* namespace es_script */

#endif /* ATG_ENGINE_SIM_INTAKE_NODE_H */
