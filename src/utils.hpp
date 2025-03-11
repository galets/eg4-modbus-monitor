#pragma once
#include <string>
#include <stdexcept>
#include <cstdlib>

static std::string envOrDefault(const char* env, const std::string& defaultValue) {
    const char* envValue = std::getenv(env);
    if (envValue != nullptr) {
        return envValue;
    }
    return defaultValue;
}

static std::string envOrError(const char* env) {
    const char* envValue = std::getenv(env);
    if (envValue != nullptr) {
        return envValue;
    }
    throw std::runtime_error("Could not find environment variable: " + std::string(env));
}

