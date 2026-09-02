#include "Core/AgentCli.hpp"
#include "Core/AgentProtocol.hpp"
#include "Core/Engine.hpp"
#include "Core/Input.hpp"
#include "Core/Logger.hpp"
#include "Core/Scene.hpp"
#include "ECS/Component.hpp"
#include "ECS/Entity.hpp"
#include "Renderer/Sprout.hpp"

#include "sokol/sokol_app.h"

#include <nlohmann/json.hpp>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
using sock_t = SOCKET;
static const sock_t kInvalidSock = INVALID_SOCKET;
static void closeSock(sock_t fd) { closesocket(fd); }
static bool sockValid(sock_t fd) { return fd != INVALID_SOCKET; }
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
using sock_t = int;
static const sock_t kInvalidSock = -1;
static void closeSock(sock_t fd) { close(fd); }
static bool sockValid(sock_t fd) { return fd >= 0; }
#endif

namespace
{
    struct AgentJob
    {
        AgentRequest request;
        std::promise<AgentReply> done;
    };

    using VerbHandler = std::function<AgentReply(Engine&, const AgentRequest&)>;

    AgentReply makeError(const AgentRequest& request, const std::string& error)
    {
        AgentReply reply;
        reply.id = request.id;
        reply.ok = false;
        reply.error = error;
        return reply;
    }

    AgentReply makeOk(const AgentRequest& request, nlohmann::json payload = nlohmann::json::object())
    {
        AgentReply reply;
        reply.id = request.id;
        reply.ok = true;
        reply.payload = std::move(payload);
        return reply;
    }

    auto edgeFromArgs(const nlohmann::json& args, std::string& error) -> std::string
    {
        std::string edge = "press";
        if (args.contains("edge"))
        {
            if (!args["edge"].is_string())
            {
                error = "args.edge must be a string";
                return {};
            }
            edge = args["edge"].get<std::string>();
        }
        if (edge != "down" && edge != "up" && edge != "press")
        {
            error = "args.edge must be down, up, or press";
            return {};
        }
        return edge;
    }

    void applyEdge(int keyCode, const std::string& edge)
    {
        if (edge == "down")
        {
            Input::injectKey(keyCode, true);
        }
        else if (edge == "up")
        {
            Input::injectKey(keyCode, false);
        }
        else
        {
            Input::injectKey(keyCode, true);
            Input::injectKey(keyCode, false);
        }
    }

    AgentReply handlePing(Engine&, const AgentRequest& request)
    {
        return makeOk(request, {{"pong", true}});
    }

    AgentReply handleKey(Engine&, const AgentRequest& request)
    {
        if (!request.args.contains("name") || !request.args["name"].is_string())
        {
            return makeError(request, "args.name must be a key name string");
        }
        std::string edgeError;
        const std::string edge = edgeFromArgs(request.args, edgeError);
        if (edge.empty())
        {
            return makeError(request, edgeError);
        }
        int keyCode = 0;
        const std::string name = request.args["name"].get<std::string>();
        if (!Input::keyCodeFromName(name, keyCode))
        {
            return makeError(request, "unknown key '" + name + "'");
        }
        applyEdge(keyCode, edge);
        return makeOk(request, {{"key", name}, {"edge", edge}});
    }

    AgentReply handleAction(Engine&, const AgentRequest& request)
    {
        if (!request.args.contains("name") || !request.args["name"].is_string())
        {
            return makeError(request, "args.name must be an action name string");
        }
        std::string edgeError;
        const std::string edge = edgeFromArgs(request.args, edgeError);
        if (edge.empty())
        {
            return makeError(request, edgeError);
        }
        const std::string name = request.args["name"].get<std::string>();
        const std::vector<int>* keys = Input::getActionKeys(name);
        if (!keys || keys->empty())
        {
            return makeError(request, "unknown action '" + name + "'");
        }
        applyEdge(keys->front(), edge);
        return makeOk(request, {{"action", name}, {"edge", edge}});
    }

    AgentReply handleState(Engine& engine, const AgentRequest& request)
    {
        nlohmann::json payload;
        payload["scene"] = engine.getCurrentSceneName();
        payload["frame"] = engine.getCurrentFrame();
        payload["scenes"] = engine.sceneNames();

        nlohmann::json entities = nlohmann::json::array();
        auto scene = engine.getCurrentScene();
        if (scene)
        {
            for (const auto& entity : scene->getEntities())
            {
                nlohmann::json item;
                item["id"] = entity->getId();
                item["name"] = entity->getName();
                item["active"] = entity->isActive();
                item["tags"] = entity->getTags();
                nlohmann::json components = nlohmann::json::array();
                if (entity->hasComponent<Comp::Transform>())
                {
                    components.push_back("Transform");
                    const auto& transform = entity->getComponent<Comp::Transform>();
                    item["transform"] = {
                        {"x", transform.position.x},
                        {"y", transform.position.y},
                        {"z", transform.position.z},
                    };
                }
                if (entity->hasComponent<Comp::Sprite>())
                {
                    components.push_back("Sprite");
                }
                if (entity->hasComponent<Comp::Text>())
                {
                    components.push_back("Text");
                    item["text"] = entity->getComponent<Comp::Text>().text;
                }
                if (entity->hasComponent<Comp::BBox>())
                {
                    components.push_back("BBox");
                }
                item["components"] = components;
                entities.push_back(std::move(item));
            }
        }
        payload["entities"] = std::move(entities);
        return makeOk(request, std::move(payload));
    }

    AgentReply handleScene(Engine& engine, const AgentRequest& request)
    {
        if (!request.args.contains("name") || !request.args["name"].is_string())
        {
            return makeError(request, "args.name must be a scene name string");
        }
        const std::string name = request.args["name"].get<std::string>();
        if (!engine.hasScene(name))
        {
            return makeError(request, "unknown scene '" + name + "'");
        }
        engine.changeScene(name);
        return makeOk(request, {{"scene", name}});
    }

    AgentReply handleQuit(Engine&, const AgentRequest& request)
    {
        sapp_request_quit();
        return makeOk(request, {{"quit", true}});
    }

    class Runtime
    {
    public:
        Engine* engine = nullptr;
        std::mutex mutex;
        std::deque<std::shared_ptr<AgentJob>> inbound;
        std::shared_ptr<AgentJob> pendingScreenshot;
        std::unordered_map<std::string, VerbHandler> verbs;
        std::atomic<bool> running{false};
        sock_t listenFd = kInvalidSock;
        std::thread acceptThread;
        std::vector<std::thread> clients;
        std::mutex clientsMutex;

        Runtime()
        {
            verbs.emplace("ping", handlePing);
            verbs.emplace("key", handleKey);
            verbs.emplace("action", handleAction);
            verbs.emplace("state", handleState);
            verbs.emplace("scene", handleScene);
            verbs.emplace("quit", handleQuit);
        }

        int port() const
        {
            const char* env = std::getenv("SAPLING_AGENT_PORT");
            if (!env || !*env)
            {
                return kAgentDefaultPort;
            }
            return std::atoi(env);
        }

        void fulfill(const std::shared_ptr<AgentJob>& job, AgentReply reply)
        {
            try
            {
                job->done.set_value(std::move(reply));
            }
            catch (const std::future_error&)
            {
            }
        }
    };

    Runtime g_runtime;

    bool readLine(sock_t fd, std::string& buffer, std::string& line)
    {
        while (true)
        {
            const auto pos = buffer.find('\n');
            if (pos != std::string::npos)
            {
                line = buffer.substr(0, pos);
                if (!line.empty() && line.back() == '\r')
                {
                    line.pop_back();
                }
                buffer.erase(0, pos + 1);
                return true;
            }
            char chunk[1024];
            const int n = recv(fd, chunk, sizeof(chunk), 0);
            if (n <= 0)
            {
                return false;
            }
            buffer.append(chunk, chunk + n);
        }
    }

    bool writeAll(sock_t fd, const std::string& data)
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

    void clientLoop(sock_t fd)
    {
        std::string buffer;
        while (g_runtime.running.load())
        {
            std::string line;
            if (!readLine(fd, buffer, line))
            {
                break;
            }
            if (line.empty())
            {
                continue;
            }
            AgentReply reply;
            try
            {
                const nlohmann::json body = nlohmann::json::parse(line);
                std::string parseError;
                AgentRequest request = parseAgentRequest(body, parseError);
                if (!parseError.empty())
                {
                    reply.ok = false;
                    reply.id = request.id;
                    reply.error = parseError;
                }
                else if (request.verb == "ping")
                {
                    reply = handlePing(*g_runtime.engine, request);
                }
                else
                {
                    auto job = std::make_shared<AgentJob>();
                    job->request = std::move(request);
                    auto future = job->done.get_future();
                    {
                        std::lock_guard<std::mutex> lock(g_runtime.mutex);
                        g_runtime.inbound.push_back(job);
                    }
                    if (future.wait_for(std::chrono::seconds(10)) != std::future_status::ready)
                    {
                        reply = makeError(job->request, "timed out waiting for game thread");
                    }
                    else
                    {
                        reply = future.get();
                    }
                }
            }
            catch (const std::exception& ex)
            {
                reply.ok = false;
                reply.error = ex.what();
            }
            const std::string out = agentReplyJson(reply).dump() + "\n";
            if (!writeAll(fd, out))
            {
                break;
            }
        }
        closeSock(fd);
    }

    void acceptLoop()
    {
        while (g_runtime.running.load())
        {
            sockaddr_in addr{};
            socklen_t len = sizeof(addr);
            const sock_t client = accept(g_runtime.listenFd, reinterpret_cast<sockaddr*>(&addr), &len);
            if (!sockValid(client))
            {
                if (!g_runtime.running.load())
                {
                    break;
                }
                continue;
            }
            std::lock_guard<std::mutex> lock(g_runtime.clientsMutex);
            g_runtime.clients.emplace_back(clientLoop, client);
        }
    }
}

void AgentCli::bind(Engine& engine)
{
    g_runtime.engine = &engine;
}

void AgentCli::start()
{
    if (!g_runtime.engine)
    {
        Logger::error("AgentCli has no engine");
        return;
    }
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        Logger::error("AgentCli WSAStartup failed");
        return;
    }
#endif
    g_runtime.listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (!sockValid(g_runtime.listenFd))
    {
        Logger::error("AgentCli socket failed");
        return;
    }
    int yes = 1;
#ifdef _WIN32
    setsockopt(g_runtime.listenFd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&yes), sizeof(yes));
#else
    setsockopt(g_runtime.listenFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
#endif
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(g_runtime.port()));
    inet_pton(AF_INET, kAgentDefaultHost, &addr.sin_addr);
    if (::bind(g_runtime.listenFd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0)
    {
        Logger::error("AgentCli bind failed");
        closeSock(g_runtime.listenFd);
        g_runtime.listenFd = kInvalidSock;
        return;
    }
    if (listen(g_runtime.listenFd, 8) != 0)
    {
        Logger::error("AgentCli listen failed");
        closeSock(g_runtime.listenFd);
        g_runtime.listenFd = kInvalidSock;
        return;
    }
    g_runtime.running = true;
    g_runtime.acceptThread = std::thread(acceptLoop);
    Logger::info("AgentCli listening on " + std::string(kAgentDefaultHost) + ":" + std::to_string(g_runtime.port()));
}

void AgentCli::stop()
{
    g_runtime.running = false;
    if (sockValid(g_runtime.listenFd))
    {
#ifdef _WIN32
        shutdown(g_runtime.listenFd, SD_BOTH);
#else
        shutdown(g_runtime.listenFd, SHUT_RDWR);
#endif
        closeSock(g_runtime.listenFd);
        g_runtime.listenFd = kInvalidSock;
    }
    if (g_runtime.acceptThread.joinable())
    {
        g_runtime.acceptThread.join();
    }
    {
        std::lock_guard<std::mutex> lock(g_runtime.clientsMutex);
        for (auto& thread : g_runtime.clients)
        {
            if (thread.joinable())
            {
                thread.join();
            }
        }
        g_runtime.clients.clear();
    }
    std::lock_guard<std::mutex> lock(g_runtime.mutex);
    while (!g_runtime.inbound.empty())
    {
        g_runtime.fulfill(g_runtime.inbound.front(), makeError(g_runtime.inbound.front()->request, "agent stopping"));
        g_runtime.inbound.pop_front();
    }
    if (g_runtime.pendingScreenshot)
    {
        g_runtime.fulfill(g_runtime.pendingScreenshot, makeError(g_runtime.pendingScreenshot->request, "agent stopping"));
        g_runtime.pendingScreenshot.reset();
    }
#ifdef _WIN32
    WSACleanup();
#endif
}

void AgentCli::drain()
{
    if (!g_runtime.engine)
    {
        return;
    }
    std::deque<std::shared_ptr<AgentJob>> jobs;
    {
        std::lock_guard<std::mutex> lock(g_runtime.mutex);
        jobs.swap(g_runtime.inbound);
    }
    for (auto& job : jobs)
    {
        if (job->request.verb == "screenshot")
        {
            std::lock_guard<std::mutex> lock(g_runtime.mutex);
            if (g_runtime.pendingScreenshot)
            {
                g_runtime.fulfill(job, makeError(job->request, "screenshot already pending"));
            }
            else
            {
                g_runtime.pendingScreenshot = job;
            }
            continue;
        }
        const auto handler = g_runtime.verbs.find(job->request.verb);
        if (handler == g_runtime.verbs.end())
        {
            g_runtime.fulfill(job, makeError(job->request, "unknown verb '" + job->request.verb + "'"));
            continue;
        }
        try
        {
            g_runtime.fulfill(job, handler->second(*g_runtime.engine, job->request));
        }
        catch (const std::exception& ex)
        {
            g_runtime.fulfill(job, makeError(job->request, ex.what()));
        }
    }
}

void AgentCli::afterPresent()
{
    std::shared_ptr<AgentJob> job;
    {
        std::lock_guard<std::mutex> lock(g_runtime.mutex);
        job = g_runtime.pendingScreenshot;
        g_runtime.pendingScreenshot.reset();
    }
    if (!job)
    {
        return;
    }
    std::string path = "screenshot.png";
    if (job->request.args.contains("path"))
    {
        if (!job->request.args["path"].is_string() || job->request.args["path"].get<std::string>().empty())
        {
            g_runtime.fulfill(job, makeError(job->request, "args.path must be a non-empty string"));
            return;
        }
        path = job->request.args["path"].get<std::string>();
    }
    std::string captureError;
    if (!Sprout::Window::getInstance()->captureFramebufferPng(path, captureError))
    {
        g_runtime.fulfill(job, makeError(job->request, captureError));
        return;
    }
    g_runtime.fulfill(job, makeOk(job->request, {{"path", path}}));
}
