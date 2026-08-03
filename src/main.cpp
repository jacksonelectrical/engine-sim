#include "../include/engine_sim_application.h"

#include <iostream>
#include <string>

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow)
{
    (void)nCmdShow;
    (void)hPrevInstance;

    EngineSimApplication application;
    std::string scriptPath = lpCmdLine != nullptr ? lpCmdLine : "";
    const std::string whitespace = " \t\r\n";
    const std::size_t first = scriptPath.find_first_not_of(whitespace);
    const std::size_t last = scriptPath.find_last_not_of(whitespace);
    scriptPath = first == std::string::npos
        ? ""
        : scriptPath.substr(first, last - first + 1);
    if (
        scriptPath.size() >= 2
        && scriptPath.front() == '"'
        && scriptPath.back() == '"')
    {
        scriptPath = scriptPath.substr(1, scriptPath.size() - 2);
    }
    if (!scriptPath.empty()) {
        application.setScriptPath(scriptPath);
    }
    application.initialize((void *)&hInstance, ysContextObject::DeviceAPI::DirectX11);
    application.run();
    application.destroy();

    return 0;
}
