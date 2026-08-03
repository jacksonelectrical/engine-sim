#include <gtest/gtest.h>

#include "../scripting/include/compiler.h"

TEST(DieselScriptTests, ReferenceConfigurationCompilesAndBuildsDieselEngine) {
    es_script::Compiler compiler;
    compiler.initialize();

    const bool compiled = compiler.compile("../assets/diesel-main.mr");
    EXPECT_TRUE(compiled);
    if (!compiled) {
        compiler.destroy();
        return;
    }

    const es_script::Compiler::Output output = compiler.execute();
    EXPECT_NE(output.engine, nullptr);
    EXPECT_NE(output.transmission, nullptr);
    EXPECT_NE(output.vehicle, nullptr);
    if (output.engine != nullptr) {
        EXPECT_EQ(output.engine->getCylinderCount(), 1);
        EXPECT_EQ(
            output.engine->getCombustionEventController()->getType(),
            CombustionEventController::Type::CompressionIgnition);
        EXPECT_TRUE(output.engine->getIntake(0)->m_directInjection);
    }

    compiler.destroy();
    if (output.engine != nullptr) {
        output.engine->destroy();
        delete output.engine;
    }
    delete output.transmission;
    delete output.vehicle;
}
