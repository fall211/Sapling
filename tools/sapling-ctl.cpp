#include "Core/AgentProtocol.hpp"

#include <nlohmann/json.hpp>

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
using sock_t = SOCKET;
static const sock_t kInvalidSock = INVALID_SOCKET;
static void closeSock(sock_t fd) { closesocket(fd); }
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
using sock_t = int;
static const sock_t kInvalidSock = -1;
static void closeSock(sock_t fd) { close(fd); }
#endif

static void usage()
{
    std::cerr << "usage: sapling-ctl [--host HOST] [--port PORT] <verb> [args]\n"
              << "  ping\n"
              << "  state\n"
              << "  screenshot [path]\n"
              << "  key <NAME> [down|up|press]\n"
              << "  action <NAME> [down|up|press]\n"
              << "  scene <NAME>\n"
              << "  quit\n";
}

static bool writeAll(sock_t fd, const std::string& data)
{
    size_t sent = 0;
    while (sent < data.size())
    {
        const int n = send(fd, data.data() + sent, static_cast<int>(data.size() - sent), 0);
        if (n <= 0)
        {
            return false;
        }
        sent += static_cast<size_t>(n);
    }
    return true;
}

static auto readLine(sock_t fd) -> std::string
{
    std::string buffer;
    char chunk[1024];
    while (buffer.find('\n') == std::string::npos)
    {
        const int n = recv(fd, chunk, sizeof(chunk), 0);
        if (n <= 0)
        {
            return {};
        }
        buffer.append(chunk, chunk + n);
    }
    const auto pos = buffer.find('\n');
    std::string line = buffer.substr(0, pos);
    if (!line.empty() && line.back() == '\r')
    {
        line.pop_back();
    }
    return line;
}

int main(int argc, char** argv)
{
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }
#endif
    std::string host = kAgentDefaultHost;
    int port = kAgentDefaultPort;
    int i = 1;
    while (i < argc && std::strncmp(argv[i], "--", 2) == 0)
    {
        const std::string flag = argv[i];
        if (flag == "--host" && i + 1 < argc)
        {
            host = argv[++i];
        }
        else if (flag == "--port" && i + 1 < argc)
        {
            port = std::atoi(argv[++i]);
        }
        else
        {
            usage();
            return 2;
        }
        ++i;
    }
    if (i >= argc)
    {
        usage();
        return 2;
    }

    AgentRequest request;
    request.id = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    request.verb = argv[i++];
    if (request.verb == "screenshot")
    {
        if (i < argc)
        {
            request.args["path"] = argv[i++];
        }
    }
    else if (request.verb == "key" || request.verb == "action")
    {
        if (i >= argc)
        {
            usage();
            return 2;
        }
        request.args["name"] = argv[i++];
        request.args["edge"] = (i < argc) ? argv[i++] : "press";
    }
    else if (request.verb == "scene")
    {
        if (i >= argc)
        {
            usage();
            return 2;
        }
        request.args["name"] = argv[i++];
    }
    else if (request.verb != "ping" && request.verb != "state" && request.verb != "quit")
    {
        usage();
        return 2;
    }
    if (i != argc)
    {
        usage();
        return 2;
    }

    sock_t fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == kInvalidSock)
    {
        std::cerr << "socket failed\n";
        return 1;
    }
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(port));
    if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) != 1)
    {
        std::cerr << "invalid host\n";
        closeSock(fd);
        return 1;
    }
    if (connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0)
    {
        std::cerr << "connect failed\n";
        closeSock(fd);
        return 1;
    }

    nlohmann::json body = {
        {"id", request.id},
        {"verb", request.verb},
        {"args", request.args},
    };
    if (!writeAll(fd, body.dump() + "\n"))
    {
        std::cerr << "send failed\n";
        closeSock(fd);
        return 1;
    }
    const std::string line = readLine(fd);
    closeSock(fd);
#ifdef _WIN32
    WSACleanup();
#endif
    if (line.empty())
    {
        std::cerr << "no reply\n";
        return 1;
    }
    std::cout << line << "\n";
    try
    {
        const nlohmann::json reply = nlohmann::json::parse(line);
        if (!reply.value("ok", false))
        {
            return 1;
        }
    }
    catch (const std::exception&)
    {
        return 1;
    }
    return 0;
}
