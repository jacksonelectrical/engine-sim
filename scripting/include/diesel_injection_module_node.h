#ifndef ATG_ENGINE_SIM_DIESEL_INJECTION_MODULE_NODE_H
#define ATG_ENGINE_SIM_DIESEL_INJECTION_MODULE_NODE_H

#include "object_reference_node.h"

#include "engine_context.h"
#include "function_node.h"
#include "ignition_wire_node.h"

#include "engine_sim.h"

#include <set>
#include <vector>

namespace es_script {

    class DieselInjectionModuleNode
        : public ObjectReferenceNode<DieselInjectionModuleNode>
    {
    public:
        struct Post {
            IgnitionWireNode *wire;
            double angle;
        };

    public:
        DieselInjectionModuleNode() { /* void */ }
        virtual ~DieselInjectionModuleNode() { /* void */ }

        void generate(Engine *engine, EngineContext *context) const {
            DieselInjectionModule::Parameters params = m_parameters;
            params.engine = engine;
            params.crankshaft = engine->getCrankshaft(0);
            params.cylinderCount = engine->getCylinderCount();
            params.injectionTimingCurve =
                m_injectionTimingCurve->generate(context);

            DieselInjectionModule *module = engine->getDieselInjectionModule();
            module->initialize(params);

            for (const Post &post : m_posts) {
                const std::set<IgnitionWireNode::Connection> connections =
                    post.wire->getConnections();
                for (const IgnitionWireNode::Connection &connection : connections) {
                    const int index = context->getCylinderIndex(
                        connection.first,
                        connection.second);
                    module->setFiringOrder(index, post.angle);
                }
            }

            engine->useDieselInjectionModule();
        }

        void connect(IgnitionWireNode *wire, double angle) {
            m_posts.push_back({ wire, angle });
        }

    protected:
        virtual void registerInputs() {
            addInput("injection_timing_curve", &m_injectionTimingCurve);
            addInput("rev_limit", &m_parameters.revLimit);
            addInput("limiter_duration", &m_parameters.limiterDuration);
            addInput("idle_fuel_mass", &m_parameters.idleFuelMass);
            addInput("max_fuel_mass", &m_parameters.maxFuelMass);
            addInput("injection_duration", &m_parameters.injectionDuration);
            addInput("ignition_delay", &m_parameters.ignitionDelay);
            addInput("cetane_number", &m_parameters.cetaneNumber);
            addInput(
                "minimum_ignition_temperature",
                &m_parameters.minimumIgnitionTemperature);
            addInput(
                "premixed_burn_fraction",
                &m_parameters.premixedBurnFraction);
            addInput(
                "premixed_burn_duration",
                &m_parameters.premixedBurnDuration);
            addInput(
                "diffusion_burn_duration",
                &m_parameters.diffusionBurnDuration);
            addInput("combustion_noise", &m_parameters.combustionNoise);

            ObjectReferenceNode<DieselInjectionModuleNode>::registerInputs();
        }

        virtual void _evaluate() {
            setOutput(this);
            readAllInputs();
        }

        FunctionNode *m_injectionTimingCurve = nullptr;
        DieselInjectionModule::Parameters m_parameters;
        std::vector<Post> m_posts;
    };

} /* namespace es_script */

#endif /* ATG_ENGINE_SIM_DIESEL_INJECTION_MODULE_NODE_H */
