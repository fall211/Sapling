#pragma once

#include <nlohmann/json.hpp>
#include <string>

inline constexpr const char* kAgentDefaultHost = "127.0.0.1";
inline constexpr int kAgentDefaultPort = 17321;

struct AgentRequest
{
    std::string id;
    std::string verb;
    nlohmann::json args = nlohmann::json::object();
};

struct AgentReply
{
    std::string id;
    bool ok = true;
    std::string error;
    nlohmann::json payload = nlohmann::json::object();
};

inline auto parseAgentRequest(const nlohmann::json& body, std::string& error) -> AgentRequest
{
    AgentRequest request;
    if (!body.is_object())
    {
        error = "request must be a JSON object";
        return request;
    }
    if (!body.contains("id") || !body["id"].is_string() || body["id"].get<std::string>().empty())
    {
        error = "request.id must be a non-empty string";
        return request;
    }
    if (!body.contains("verb") || !body["verb"].is_string() || body["verb"].get<std::string>().empty())
    {
        error = "request.verb must be a non-empty string";
        return request;
    }
    request.id = body["id"].get<std::string>();
    request.verb = body["verb"].get<std::string>();
    if (body.contains("args"))
    {
        if (!body["args"].is_object())
        {
            error = "request.args must be an object";
            return request;
        }
        request.args = body["args"];
    }
    return request;
}

inline auto agentReplyJson(const AgentReply& reply) -> nlohmann::json
{
    nlohmann::json body = {
        {"id", reply.id},
        {"ok", reply.ok},
        {"payload", reply.payload.is_null() ? nlohmann::json::object() : reply.payload},
    };
    if (!reply.error.empty())
    {
        body["error"] = reply.error;
    }
    return body;
}
