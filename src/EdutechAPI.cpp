#include "EdutechAPI.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>

EdutechAPI::EdutechAPI()
    : _protocol(),
      _host(),
      _timeoutMs(5000),
      _debug(nullptr) {
    useHomolog();
}

EdutechAPI::EdutechAPI(EdutechApiEnvironment environment)
    : _protocol(),
      _host(),
      _timeoutMs(5000),
      _debug(nullptr) {
    useEnvironment(environment);
}

EdutechAPI::EdutechAPI(const String& protocol, const String& host)
    : _protocol(protocol),
      _host(host),
      _timeoutMs(5000),
      _debug(nullptr) {
}

void EdutechAPI::useEnvironment(EdutechApiEnvironment environment) {
    if (environment == EdutechApiEnvironment::Production) {
        useProduction();
        return;
    }

    useHomolog();
}

void EdutechAPI::useHomolog() {
    begin("http", "10.1.1.22");
}

void EdutechAPI::useProduction() {
    begin("https", "www.edutechblocks.com.br");
}

void EdutechAPI::begin(const String& url) {
    String value = url;
    value.trim();

    if (value.endsWith("/")) {
        value.remove(value.length() - 1);
    }

    if (value.startsWith("http://")) {
        _protocol = "http";
        _host = value.substring(7);
        return;
    }

    if (value.startsWith("https://")) {
        _protocol = "https";
        _host = value.substring(8);
        return;
    }

    _protocol = "https";
    _host = value;
}

void EdutechAPI::begin(const String& protocol, const String& host) {
    _protocol = protocol;
    _host = host;
}

void EdutechAPI::setDeviceKeys(const String& publicKey, const String& secretKey) {
    _publicKey = publicKey;
    _secretKey = secretKey;
}

void EdutechAPI::setBearerToken(const String& token) {
    _bearerToken = token;
}

void EdutechAPI::setTimeout(uint16_t timeoutMs) {
    _timeoutMs = timeoutMs;
}

void EdutechAPI::setDebug(Stream& debugPort) {
    _debug = &debugPort;
}

void EdutechAPI::clearDebug() {
    _debug = nullptr;
}

String EdutechAPI::baseUrl() const {
    String normalizedHost = _host;
    normalizedHost.trim();

    if (normalizedHost.startsWith("http://") || normalizedHost.startsWith("https://")) {
        return normalizedHost;
    }

    return _protocol + "://" + normalizedHost;
}

String EdutechAPI::url(const String& routeValue) const {
    if (routeValue.startsWith("http://") || routeValue.startsWith("https://")) {
        return routeValue;
    }

    if (routeValue.startsWith("/")) {
        return baseUrl() + routeValue;
    }

    return baseUrl() + "/" + routeValue;
}

String EdutechAPI::route(const String& pattern) const {
    return pattern;
}

String EdutechAPI::route(const String& pattern, const String& publicKey) const {
    return replaceToken(pattern, "{publicKey}", activePublicKey(publicKey));
}

String EdutechAPI::route(const String& pattern, const String& publicKey, const String& resource) const {
    String value = route(pattern, publicKey);
    return replaceToken(value, "{resource}", resource);
}

EdutechApiResponse EdutechAPI::request(const String& method, const String& routeValue, const String& body, bool useDeviceSecret) {
    HTTPClient http;
    const String requestUrl = url(routeValue);
    debugLine("[EdutechAPI] " + method + " " + requestUrl);
    http.begin(requestUrl);
    http.setTimeout(_timeoutMs);
    http.addHeader("Content-Type", "application/json");

    if (useDeviceSecret && _secretKey.length() > 0) {
        http.addHeader("Authorization", "Bearer " + _secretKey);
        http.addHeader("X-Device-Secret", _secretKey);
    } else if (_bearerToken.length() > 0) {
        http.addHeader("Authorization", "Bearer " + _bearerToken);
    }

    int httpCode = 0;
    String methodUpper = method;
    methodUpper.toUpperCase();

    if (methodUpper == "GET") {
        httpCode = http.GET();
    } else if (methodUpper == "POST") {
        httpCode = http.POST(body);
    } else if (methodUpper == "PUT") {
        httpCode = http.PUT(body);
    } else if (methodUpper == "PATCH") {
        httpCode = http.PATCH(body);
    } else if (methodUpper == "DELETE") {
        httpCode = http.sendRequest("DELETE", body);
    } else {
        httpCode = http.sendRequest(methodUpper.c_str(), body);
    }

    String payload;
    if (httpCode > 0) {
        payload = http.getString();
    }
    http.end();

    debugLine("[EdutechAPI] HTTP code: " + String(httpCode));
    if (payload.length() > 0) {
        debugLine("[EdutechAPI] Payload: " + payload);
    } else {
        debugLine("[EdutechAPI] Payload vazio");
    }

    _lastResponse = parseResponse(httpCode, payload);
    return _lastResponse;
}

EdutechApiResponse EdutechAPI::get(const String& routeValue, bool useDeviceSecret) {
    return request("GET", routeValue, String(), useDeviceSecret);
}

EdutechApiResponse EdutechAPI::post(const String& routeValue, const String& body, bool useDeviceSecret) {
    return request("POST", routeValue, body, useDeviceSecret);
}

EdutechApiResponse EdutechAPI::put(const String& routeValue, const String& body, bool useDeviceSecret) {
    return request("PUT", routeValue, body, useDeviceSecret);
}

EdutechApiResponse EdutechAPI::patch(const String& routeValue, const String& body, bool useDeviceSecret) {
    return request("PATCH", routeValue, body, useDeviceSecret);
}

EdutechApiResponse EdutechAPI::del(const String& routeValue, bool useDeviceSecret) {
    return request("DELETE", routeValue, String(), useDeviceSecret);
}

EdutechApiResponse EdutechAPI::status() {
    return get(EdutechRoutes::Api::STATUS);
}

EdutechApiResponse EdutechAPI::version() {
    return post(EdutechRoutes::Api::VERSION);
}

EdutechApiResponse EdutechAPI::time() {
    return get(EdutechRoutes::Api::TIME);
}

EdutechApiResponse EdutechAPI::login(const String& email, const String& password) {
    DynamicJsonDocument doc(512);
    doc["email"] = email;
    doc["senha"] = password;

    String body;
    serializeJson(doc, body);
    EdutechApiResponse response = post(EdutechRoutes::Api::LOGIN, body);

    DynamicJsonDocument responseDoc(2048);
    if (deserializeJson(responseDoc, response.payload) == DeserializationError::Ok && responseDoc["token"].is<const char*>()) {
        setBearerToken(responseDoc["token"].as<String>());
    }

    return response;
}

EdutechApiResponse EdutechAPI::user() {
    return get(EdutechRoutes::Api::USER);
}

EdutechApiResponse EdutechAPI::listDevices() {
    return get(EdutechRoutes::Api::DEVICES);
}

EdutechApiResponse EdutechAPI::createDevice(const String& jsonBody) {
    return post(EdutechRoutes::Api::DEVICES, jsonBody);
}

EdutechApiResponse EdutechAPI::showDevice(const String& publicKey) {
    return get(route(EdutechRoutes::Api::DEVICE, publicKey), true);
}

EdutechApiResponse EdutechAPI::updateDevice(const String& jsonBody, const String& publicKey) {
    return patch(route(EdutechRoutes::Api::DEVICE, publicKey), jsonBody, true);
}

EdutechApiResponse EdutechAPI::deleteDevice(const String& publicKey) {
    return del(route(EdutechRoutes::Api::DEVICE, publicKey), true);
}

String EdutechAPI::getDeviceLatitude(const String& publicKey) {
    return devicePayloadValue("latitude", publicKey);
}

String EdutechAPI::getDeviceLongitude(const String& publicKey) {
    return devicePayloadValue("longitude", publicKey);
}

String EdutechAPI::getDeviceIp(const String& publicKey) {
    String localIp = devicePayloadValue("local_ip", publicKey);
    if (localIp.length() > 0 && localIp != "null") {
        return localIp;
    }

    return devicePayloadValue("ip", publicKey);
}

EdutechApiResponse EdutechAPI::lastResponse() const {
    return _lastResponse;
}

EdutechApiResponse EdutechAPI::listResources(const String& publicKey) {
    return get(route(EdutechRoutes::Api::DEVICE_RESOURCES, publicKey), true);
}

EdutechApiResponse EdutechAPI::createResource(const String& jsonBody, const String& publicKey) {
    return post(route(EdutechRoutes::Api::DEVICE_RESOURCES, publicKey), jsonBody, true);
}

EdutechApiResponse EdutechAPI::showResource(const String& resource, const String& publicKey) {
    return get(route(EdutechRoutes::Api::DEVICE_RESOURCE, publicKey, resource), true);
}

EdutechApiResponse EdutechAPI::updateResource(const String& resource, const String& jsonBody, const String& publicKey) {
    return patch(route(EdutechRoutes::Api::DEVICE_RESOURCE, publicKey, resource), jsonBody, true);
}

EdutechApiResponse EdutechAPI::deleteResource(const String& resource, const String& publicKey) {
    return del(route(EdutechRoutes::Api::DEVICE_RESOURCE, publicKey, resource), true);
}

EdutechApiResponse EdutechAPI::lastResourceFeed(const String& resource, const String& publicKey) {
    return get(route(EdutechRoutes::Api::DEVICE_RESOURCE_LAST_FEED, publicKey, resource), true);
}

EdutechApiResponse EdutechAPI::createResourceFeed(const String& resource, const String& jsonBody, const String& publicKey) {
    return post(route(EdutechRoutes::Api::DEVICE_RESOURCE_FEEDS, publicKey, resource), jsonBody, true);
}

EdutechApiResponse EdutechAPI::createResourceFeed(const String& resource, float value, const String& publicKey) {
    DynamicJsonDocument doc(128);
    doc["input"] = value;

    String body;
    serializeJson(doc, body);

    return createResourceFeed(resource, body, publicKey);
}

String EdutechAPI::activePublicKey(const String& publicKey) const {
    if (publicKey.length() > 0) {
        return publicKey;
    }

    return _publicKey;
}

String EdutechAPI::replaceToken(String value, const String& token, const String& replacement) const {
    value.replace(token, replacement);
    return value;
}

String EdutechAPI::devicePayloadValue(const String& field, const String& publicKey) {
    EdutechApiResponse response = showDevice(publicKey);
    if (!response.success) {
        debugLine("[EdutechAPI] Falha ao obter dispositivo: " + String(response.httpCode) + " " + response.message);
        return "";
    }

    StaticJsonDocument<128> filter;
    filter["payload"]["ip"] = true;
    filter["payload"]["local_ip"] = true;
    filter["payload"]["latitude"] = true;
    filter["payload"]["longitude"] = true;

    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, response.payload, DeserializationOption::Filter(filter));
    if (error != DeserializationError::Ok) {
        debugLine("[EdutechAPI] Erro JSON: " + String(error.c_str()));
        return "";
    }

    JsonVariant value = doc["payload"][field.c_str()];
    if (value.isNull()) {
        debugLine("[EdutechAPI] Campo ausente ou nulo: " + field);
        return "";
    }

    return value.as<String>();
}

void EdutechAPI::debugLine(const String& message) const {
    if (_debug) {
        _debug->println(message);
    }
}

EdutechApiResponse EdutechAPI::parseResponse(int httpCode, const String& payload) const {
    EdutechApiResponse response;
    response.httpCode = httpCode;
    response.payload = payload;

    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, payload);
    if (error == DeserializationError::Ok) {
        if (doc["success"].is<bool>()) {
            response.success = doc["success"].as<bool>();
        } else if (doc["received"].is<bool>()) {
            response.success = doc["received"].as<bool>();
        } else {
            response.success = httpCode >= 200 && httpCode < 300;
        }

        if (doc["message"].is<const char*>()) {
            response.message = doc["message"].as<String>();
        }
    } else {
        response.success = httpCode >= 200 && httpCode < 300;
    }

    return response;
}
