// Author: Dr. Mazharuddin Mohammed
#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <functional>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

namespace SemiPRO {

/**
 * @brief HTTP request structure
 */
struct HttpRequest {
    std::string method;
    std::string path;
    std::unordered_map<std::string, std::string> headers;
    std::unordered_map<std::string, std::string> query_params;
    std::string body;
    std::string client_ip;
};

/**
 * @brief HTTP response structure
 */
struct HttpResponse {
    int status_code = 200;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
    std::string content_type = "application/json";
};

/**
 * @brief Request handler function type
 */
using RequestHandler = std::function<HttpResponse(const HttpRequest&)>;

/**
 * @brief Middleware function type
 */
using Middleware = std::function<bool(HttpRequest&, HttpResponse&)>;

/**
 * @brief WebSocket message structure
 */
struct WebSocketMessage {
    enum Type { TEXT, BINARY, PING, PONG, CLOSE };
    Type type;
    std::string data;
    std::string client_id;
};

/**
 * @brief WebSocket handler function type
 */
using WebSocketHandler = std::function<void(const WebSocketMessage&)>;

/**
 * @brief Rate limiting configuration
 */
struct RateLimitConfig {
    int requests_per_minute = 60;
    int burst_size = 10;
    bool enabled = true;
};

/**
 * @brief Authentication configuration
 */
struct AuthConfig {
    bool enabled = false;
    std::string secret_key;
    int token_expiry_hours = 24;
    std::unordered_map<std::string, std::string> api_keys;
};

/**
 * @brief REST API server
 */
class RestServer {
private:
    struct Route {
        std::string method;
        std::string pattern;
        RequestHandler handler;
    };
    
    std::vector<Route> routes_;
    std::vector<Middleware> middlewares_;
    std::unordered_map<std::string, WebSocketHandler> websocket_handlers_;
    
    // Server configuration
    std::string host_;
    int port_;
    int max_connections_;
    bool running_;
    
    // Threading
    std::thread server_thread_;
    std::vector<std::thread> worker_threads_;
    std::queue<std::pair<int, HttpRequest>> request_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    
    // Rate limiting
    RateLimitConfig rate_limit_config_;
    std::unordered_map<std::string, std::vector<std::chrono::steady_clock::time_point>> client_requests_;
    std::mutex rate_limit_mutex_;
    
    // Authentication
    AuthConfig auth_config_;
    
    // WebSocket connections
    std::unordered_map<std::string, int> websocket_connections_;
    std::mutex websocket_mutex_;
    
public:
    RestServer(const std::string& host = "localhost", int port = 8080, int max_connections = 100);
    ~RestServer();
    
    // Server lifecycle
    bool start();
    void stop();
    bool isRunning() const { return running_; }
    
    // Route registration
    void addRoute(const std::string& method, const std::string& pattern, RequestHandler handler);
    void get(const std::string& pattern, RequestHandler handler);
    void post(const std::string& pattern, RequestHandler handler);
    void put(const std::string& pattern, RequestHandler handler);
    void delete_(const std::string& pattern, RequestHandler handler);
    
    // Middleware
    void addMiddleware(Middleware middleware);
    void enableCORS(const std::string& origin = "*");
    void enableLogging();
    void enableCompression();
    
    // WebSocket support
    void addWebSocketHandler(const std::string& path, WebSocketHandler handler);
    void broadcastWebSocket(const std::string& path, const std::string& message);
    void sendWebSocketMessage(const std::string& client_id, const std::string& message);
    
    // Rate limiting
    void setRateLimit(const RateLimitConfig& config);
    bool checkRateLimit(const std::string& client_ip);
    
    // Authentication
    void setAuthConfig(const AuthConfig& config);
    bool authenticateRequest(const HttpRequest& request);
    std::string generateToken(const std::string& user_id);
    bool validateToken(const std::string& token);
    
    // Static file serving
    void serveStaticFiles(const std::string& path, const std::string& directory);
    
    // Error handling
    void setErrorHandler(int status_code, RequestHandler handler);
    
private:
    void serverLoop();
    void workerLoop();
    void handleRequest(int client_socket, const HttpRequest& request);
    HttpRequest parseRequest(const std::string& raw_request);
    std::string formatResponse(const HttpResponse& response);
    bool matchRoute(const std::string& pattern, const std::string& path);
    std::unordered_map<std::string, std::string> extractPathParams(const std::string& pattern, const std::string& path);
    
    // WebSocket handling
    bool isWebSocketUpgrade(const HttpRequest& request);
    void handleWebSocketUpgrade(int client_socket, const HttpRequest& request);
    void handleWebSocketMessage(const std::string& client_id, const std::string& message);
    
    // Utility functions
    std::string urlDecode(const std::string& encoded);
    std::string urlEncode(const std::string& decoded);
    std::string base64Encode(const std::string& input);
    std::string base64Decode(const std::string& encoded);
};

/**
 * @brief JSON utilities for API responses
 */
class JsonUtils {
public:
    static std::string serialize(const std::unordered_map<std::string, std::string>& data);
    static std::string serialize(const std::vector<std::string>& data);
    static std::string serialize(const std::unordered_map<std::string, double>& data);
    static std::unordered_map<std::string, std::string> parseObject(const std::string& json);
    static std::vector<std::string> parseArray(const std::string& json);
    
    static std::string createErrorResponse(int code, const std::string& message);
    static std::string createSuccessResponse(const std::string& data = "");
    static std::string createSimulationResponse(const std::string& simulation_id, 
                                               const std::string& status,
                                               const std::unordered_map<std::string, double>& results = {});
};

/**
 * @brief API endpoint handlers
 */
class ApiHandlers {
public:
    // Simulation endpoints
    static HttpResponse createSimulation(const HttpRequest& request);
    static HttpResponse getSimulation(const HttpRequest& request);
    static HttpResponse listSimulations(const HttpRequest& request);
    static HttpResponse deleteSimulation(const HttpRequest& request);
    static HttpResponse getSimulationResults(const HttpRequest& request);
    
    // Process endpoints
    static HttpResponse runOxidation(const HttpRequest& request);
    static HttpResponse runDoping(const HttpRequest& request);
    static HttpResponse runDeposition(const HttpRequest& request);
    static HttpResponse runEtching(const HttpRequest& request);
    static HttpResponse runLithography(const HttpRequest& request);
    
    // Analysis endpoints
    static HttpResponse analyzeWafer(const HttpRequest& request);
    static HttpResponse generateReport(const HttpRequest& request);
    static HttpResponse exportResults(const HttpRequest& request);
    
    // System endpoints
    static HttpResponse getSystemStatus(const HttpRequest& request);
    static HttpResponse getSystemMetrics(const HttpRequest& request);
    static HttpResponse getPluginList(const HttpRequest& request);
    
    // Utility endpoints
    static HttpResponse healthCheck(const HttpRequest& request);
    static HttpResponse getApiVersion(const HttpRequest& request);
    static HttpResponse getDocumentation(const HttpRequest& request);
};

} // namespace SemiPRO
