#ifndef EDUTECH_API_H
#define EDUTECH_API_H

#include <Arduino.h>
#include <Stream.h>

struct EdutechApiResponse {
    int httpCode = 0;
    bool success = false;
    String payload;
    String message;
};

enum class EdutechApiEnvironment {
    Homolog,
    Production
};

namespace EdutechRoutes {
namespace Api {
    static constexpr const char* STATUS = "/api/status";
    static constexpr const char* VERSION = "/api/version";
    static constexpr const char* TIME = "/api/time";
    static constexpr const char* LOGIN = "/api/login";
    static constexpr const char* USER = "/api/user";
    static constexpr const char* DEVICES = "/api/devices";
    static constexpr const char* DEVICE = "/api/devices/{publicKey}";
    static constexpr const char* DEVICE_RESOURCES = "/api/devices/{publicKey}/resources";
    static constexpr const char* DEVICE_RESOURCE = "/api/devices/{publicKey}/resources/{resource}";
    static constexpr const char* DEVICE_RESOURCE_FEEDS = "/api/devices/{publicKey}/resources/{resource}/feeds";
    static constexpr const char* DEVICE_RESOURCE_LAST_FEED = "/api/devices/{publicKey}/resources/{resource}/feeds/last";
    static constexpr const char* DEVICE_ALIAS = "/api/device/{publicKey}";
    static constexpr const char* DEVICE_ALIAS_RESOURCES = "/api/device/{publicKey}/resources";
    static constexpr const char* DEVICE_ALIAS_RESOURCE = "/api/device/{publicKey}/resources/{resource}";
    static constexpr const char* DEVICE_ALIAS_RESOURCE_FEEDS = "/api/device/{publicKey}/resources/{resource}/feeds";
    static constexpr const char* DEVICE_ALIAS_RESOURCE_LAST_FEED = "/api/device/{publicKey}/resources/{resource}/feeds/last";
}
}

class EdutechAPI {
public:
    EdutechAPI();
    explicit EdutechAPI(EdutechApiEnvironment environment);
    EdutechAPI(const String& protocol, const String& host);

    void useEnvironment(EdutechApiEnvironment environment);
    void useHomolog();
    void useProduction();
    void begin(const String& url);
    void begin(const String& protocol, const String& host);
    void setDeviceKeys(const String& publicKey, const String& secretKey);
    void setBearerToken(const String& token);
    void setTimeout(uint16_t timeoutMs);
    void setDebug(Stream& debugPort);
    void clearDebug();

    String baseUrl() const;
    String url(const String& routeValue) const;
    String route(const String& pattern) const;
    String route(const String& pattern, const String& publicKey) const;
    String route(const String& pattern, const String& publicKey, const String& resource) const;

    EdutechApiResponse request(const String& method, const String& routeValue, const String& body = String(), bool useDeviceSecret = false);
    EdutechApiResponse get(const String& routeValue, bool useDeviceSecret = false);
    EdutechApiResponse post(const String& routeValue, const String& body = String(), bool useDeviceSecret = false);
    EdutechApiResponse put(const String& routeValue, const String& body = String(), bool useDeviceSecret = false);
    EdutechApiResponse patch(const String& routeValue, const String& body = String(), bool useDeviceSecret = false);
    EdutechApiResponse del(const String& routeValue, bool useDeviceSecret = false);

    EdutechApiResponse status();
    EdutechApiResponse version();
    EdutechApiResponse time();
    EdutechApiResponse login(const String& email, const String& password);
    EdutechApiResponse user();

    EdutechApiResponse listDevices();
    EdutechApiResponse createDevice(const String& jsonBody);
    EdutechApiResponse showDevice(const String& publicKey = String());
    EdutechApiResponse updateDevice(const String& jsonBody, const String& publicKey = String());
    EdutechApiResponse deleteDevice(const String& publicKey = String());
    String getDeviceLatitude(const String& publicKey = String());
    String getDeviceLongitude(const String& publicKey = String());
    String getDeviceIp(const String& publicKey = String());
    EdutechApiResponse lastResponse() const;

    EdutechApiResponse listResources(const String& publicKey = String());
    EdutechApiResponse createResource(const String& jsonBody, const String& publicKey = String());
    EdutechApiResponse showResource(const String& resource, const String& publicKey = String());
    EdutechApiResponse updateResource(const String& resource, const String& jsonBody, const String& publicKey = String());
    EdutechApiResponse deleteResource(const String& resource, const String& publicKey = String());
    EdutechApiResponse lastResourceFeed(const String& resource, const String& publicKey = String());
    EdutechApiResponse createResourceFeed(const String& resource, const String& jsonBody, const String& publicKey = String());
    EdutechApiResponse createResourceFeed(const String& resource, float value, const String& publicKey = String());

private:
    String _protocol;
    String _host;
    String _publicKey;
    String _secretKey;
    String _bearerToken;
    uint16_t _timeoutMs;
    Stream* _debug;
    EdutechApiResponse _lastResponse;

    String activePublicKey(const String& publicKey) const;
    String replaceToken(String value, const String& token, const String& replacement) const;
    String devicePayloadValue(const String& field, const String& publicKey = String());
    void debugLine(const String& message) const;
    EdutechApiResponse parseResponse(int httpCode, const String& payload) const;
};

#endif
