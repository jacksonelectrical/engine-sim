#include "../include/engine_sim_application.h"

#include <iostream>
#include <filesystem>
#include <string>

namespace {

bool restartApplication(const std::string &scriptPath) {
    char executablePath[MAX_PATH] = {};
    if (GetModuleFileNameA(nullptr, executablePath, MAX_PATH) == 0) {
        return false;
    }

    const std::filesystem::path executable(executablePath);
    const std::string workingDirectory = executable.parent_path().string();
    std::string commandLine =
        "\"" + executable.string() + "\" \"" + scriptPath + "\"";

    STARTUPINFOA startupInfo = {};
    startupInfo.cb = sizeof(startupInfo);
    PROCESS_INFORMATION processInfo = {};
    const BOOL created = CreateProcessA(
        executablePath,
        commandLine.data(),
        nullptr,
        nullptr,
        FALSE,
        0,
        nullptr,
        workingDirectory.c_str(),
        &startupInfo,
        &processInfo);
    if (created != FALSE) {
        CloseHandle(processInfo.hThread);
        CloseHandle(processInfo.hProcess);
    }

    return created != FALSE;
}

} // namespace

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
    const std::string restartScriptPath = application.getRestartScriptPath();
    application.destroy();

    if (!restartScriptPath.empty()) {
        restartApplication(restartScriptPath);
    }

    return 0;
}
