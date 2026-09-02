#pragma once

class Engine;

class AgentCli
{
public:
    static void bind(Engine& engine);
    static void start();
    static void stop();
    static void drain();
    static void afterPresent();
};
