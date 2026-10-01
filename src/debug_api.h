#pragma once
#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace chq {

// v0.60.1 localhost-only control plane for the Research Workbench.
// The wire protocol is intentionally tiny: one UTF-8 command line in, one
// UTF-8 response terminated by \n. chqctl.exe is the human-facing client.
class DebugApiServer {
public:
    using Handler = std::function<std::string(const std::string&)>;

    DebugApiServer();
    ~DebugApiServer();
    DebugApiServer(const DebugApiServer&) = delete;
    DebugApiServer& operator=(const DebugApiServer&) = delete;

    bool start(std::uint16_t port, std::string& error);
    void stop();
    bool running() const;
    std::uint16_t port() const;
    void poll(const Handler& handler);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace chq
