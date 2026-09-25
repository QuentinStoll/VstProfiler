#include "MarketplaceClient.h"

#include "SettingsPath.h"

namespace Marketplace {
namespace {
class UrlHttpTransport : public HttpTransport {
   public:
    HttpResponse send(const juce::String& method,
                      const juce::String& url,
                      const juce::String& jsonBody,
                      const juce::String& bearerToken) override {
        juce::URL requestUrl(url);
        if (jsonBody.isNotEmpty()) {
            requestUrl = requestUrl.withPOSTData(jsonBody);
        }

        juce::String headers = "Accept: application/json\r\n";
        if (jsonBody.isNotEmpty()) {
            headers << "Content-Type: application/json\r\n";
        }
        if (bearerToken.isNotEmpty()) {
            headers << "Authorization: Bearer " << bearerToken << "\r\n";
        }

        int statusCode = 0;
        const auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inPostData)
                                 .withConnectionTimeoutMs(20000)
                                 .withExtraHeaders(headers)
                                 .withHttpRequestCmd(method)
                                 .withStatusCode(&statusCode);

        HttpResponse response;
        response.statusCode = statusCode;
        if (auto stream = requestUrl.createInputStream(options)) {
            stream->readIntoMemoryBlock(response.body);
            response.statusCode = statusCode;
        }

        return response;
    }
};

juce::String bodyAsString(const juce::MemoryBlock& body) {
    return juce::String::fromUTF8(static_cast<const char*>(body.getData()), static_cast<int>(body.getSize()));
}

juce::String jsonString(const juce::var& object, const juce::Identifier& key) {
    if (auto* dynamicObject = object.getDynamicObject()) {
        if (dynamicObject->hasProperty(key)) {
            const auto value = dynamicObject->getProperty(key);
            if (!value.isVoid()) {
                return value.toString();
            }
        }
    }

    return {};
}

bool jsonBool(const juce::var& object, const juce::Identifier& key, bool fallback) {
    if (auto* dynamicObject = object.getDynamicObject()) {
        if (dynamicObject->hasProperty(key)) {
            return static_cast<bool>(dynamicObject->getProperty(key));
        }
    }

    return fallback;
}

Status statusFromCode(int statusCode, bool loginCall) {
    switch (statusCode) {
        case 200:
            return Status::Success;
        case 400:
            return Status::BadRequest;
        case 401:
            return Status::Unauthorized;
        case 403:
            return loginCall ? Status::MfaRequired : Status::Unexpected;
        case 503:
            return Status::Unavailable;
        case 0:
            return Status::TransportError;
        default:
            return Status::Unexpected;
    }
}

juce::String messageFor(Status status, bool mfaCall) {
    switch (status) {
        case Status::Unauthorized:
            return mfaCall ? "That code was refused or has expired. Sign in again."
                           : "Email or password was refused.";
        case Status::BadRequest:
            return mfaCall ? "Enter the 6-digit code."
                           : "The sign-in request is invalid.";
        case Status::Unavailable:
            return "Sign-in is unavailable right now.";
        case Status::TransportError:
            return "Could not reach profiler.audio.";
        case Status::Unexpected:
            return "profiler.audio returned an unexpected response.";
        case Status::Success:
        case Status::MfaRequired:
            break;
    }

    return {};
}

bool readSession(const juce::var& parsed, Session& session) {
    if (jsonBool(parsed, "mfaRequired", false)) {
        return false;
    }

    session.accessToken = jsonString(parsed, "accessToken");
    session.refreshToken = jsonString(parsed, "refreshToken");
    session.expiresIn = 0;
    if (auto* object = parsed.getDynamicObject()) {
        session.expiresIn = static_cast<int>(object->getProperty("expiresIn"));
    }

    if (auto* object = parsed.getDynamicObject()) {
        const auto user = object->getProperty("user");
        session.userId = jsonString(user, "id");
        session.email = jsonString(user, "email");
    }

    return session.accessToken.isNotEmpty();
}

LoginResult parseLoginBody(int statusCode, const juce::MemoryBlock& body, bool mfaCall) {
    LoginResult result;
    result.status = statusFromCode(statusCode, !mfaCall);
    result.message = messageFor(result.status, mfaCall);

    if (result.status == Status::TransportError || result.status == Status::Unavailable ||
        result.status == Status::BadRequest) {
        return result;
    }

    const auto parsed = juce::JSON::parse(bodyAsString(body));

    if (result.status == Status::MfaRequired) {
        result.challenge.factorId = jsonString(parsed, "factorId");
        result.challenge.challengeId = jsonString(parsed, "challengeId");
        if (result.challenge.factorId.isEmpty() || result.challenge.challengeId.isEmpty()) {
            result.status = Status::Unexpected;
            result.message = "profiler.audio did not return a verification challenge.";
            result.challenge = {};
        }
        return result;
    }

    if (result.status == Status::Success) {
        if (!readSession(parsed, result.session)) {
            result.status = Status::Unexpected;
            result.message = "The sign-in response did not include a session.";
            result.session = {};
        }
        return result;
    }

    if (mfaCall && result.status == Status::Unauthorized) {
        result.session = {};
        result.challenge = {};
    }

    return result;
}

LibraryItem parseItem(const juce::var& item) {
    LibraryItem libraryItem;
    libraryItem.entitlementId = jsonString(item, "entitlementId");
    libraryItem.acquiredAt = jsonString(item, "acquiredAt");
    libraryItem.source = jsonString(item, "source");
    libraryItem.id = jsonString(item, "id");
    libraryItem.title = jsonString(item, "title");
    libraryItem.authorName = jsonString(item, "authorName");
    libraryItem.fileUrl = jsonString(item, "fileUrl");
    libraryItem.irFileUrl = jsonString(item, "irFileUrl");
    libraryItem.hasIntegratedIr = jsonBool(item, "hasIntegratedIr", false);
    return libraryItem;
}
}  // namespace

Client::Client(std::shared_ptr<HttpTransport> transport, juce::String apiBaseUrl)
    : _transport(transport != nullptr ? std::move(transport) : std::make_shared<UrlHttpTransport>()),
      _apiBaseUrl(apiBaseUrl.trim().trimCharactersAtEnd("/")) {}

LoginResult Client::login(const juce::String& email, const juce::String& password) const {
    auto payload = std::make_unique<juce::DynamicObject>();
    payload->setProperty("email", email.trim());
    payload->setProperty("password", password);
    const auto json = juce::JSON::toString(juce::var(payload.release()), false);

    const auto response = _transport->send("POST", _apiBaseUrl + "/api/v1/plugin/login", json, {});
    return parseLoginBody(response.statusCode, response.body, false);
}

LoginResult Client::submitCode(const juce::String& factorId,
                               const juce::String& challengeId,
                               const juce::String& code) const {
    LoginResult rejected;
    if (!isSixDigitCode(code)) {
        rejected.status = Status::BadRequest;
        rejected.message = "Enter the 6-digit code.";
        return rejected;
    }

    auto payload = std::make_unique<juce::DynamicObject>();
    payload->setProperty("factorId", factorId);
    payload->setProperty("challengeId", challengeId);
    payload->setProperty("code", code);
    const auto json = juce::JSON::toString(juce::var(payload.release()), false);

    const auto response = _transport->send("POST", _apiBaseUrl + "/api/v1/plugin/mfa", json, {});
    auto result = parseLoginBody(response.statusCode, response.body, true);
    if (result.status == Status::Unauthorized) {
        result.message = "That code was refused or has expired. Sign in again.";
    } else if (result.status == Status::BadRequest) {
        result.message = "Enter the 6-digit code.";
    } else if (result.status == Status::Unavailable) {
        result.message = "Sign-in is unavailable right now.";
    }
    return result;
}

LibraryResult Client::fetchLibrary(const juce::String& accessToken) const {
    LibraryResult result;
    if (accessToken.isEmpty()) {
        result.status = Status::Unauthorized;
        result.message = "Sign in to load your marketplace library.";
        return result;
    }

    const auto response = _transport->send("GET", _apiBaseUrl + "/api/v1/plugin/library", {}, accessToken);
    result.status = statusFromCode(response.statusCode, false);

    if (result.status == Status::Unauthorized) {
        result.message = "Your session has expired. Sign in again.";
        return result;
    }

    if (result.status != Status::Success) {
        result.message = result.status == Status::TransportError
                             ? "Could not reach profiler.audio."
                             : "Could not load your marketplace library.";
        return result;
    }

    const auto parsed = juce::JSON::parse(bodyAsString(response.body));
    auto* object = parsed.getDynamicObject();
    if (object == nullptr) {
        result.status = Status::Unexpected;
        result.message = "The library response was invalid.";
        return result;
    }

    result.email = jsonString(object->getProperty("user"), "email");
    const auto library = object->getProperty("library");
    if (auto* items = library.getArray()) {
        for (const auto& item : *items) {
            result.items.push_back(parseItem(item));
        }
        return result;
    }

    result.status = Status::Unexpected;
    result.message = "The library response was invalid.";
    return result;
}

DownloadResult Client::download(const juce::String& fileUrl, const juce::String& accessToken) const {
    DownloadResult result;
    const auto absoluteUrl = resolveUrl(fileUrl);
    if (absoluteUrl.isEmpty()) {
        result.status = Status::BadRequest;
        result.message = "This pack has no file to download.";
        return result;
    }

    const auto token = sendBearerToken(absoluteUrl) ? accessToken : juce::String{};
    const auto response = _transport->send("GET", absoluteUrl, {}, token);
    result.status = statusFromCode(response.statusCode, false);
    if (result.status == Status::Success) {
        result.body = response.body;
        return result;
    }

    if (result.status == Status::Unauthorized) {
        result.message = "Your session has expired. Sign in again.";
    } else if (result.status == Status::TransportError) {
        result.message = "Could not reach profiler.audio.";
    } else {
        result.message = "Could not download this pack.";
    }

    return result;
}

bool Client::isSixDigitCode(const juce::String& code) {
    return code.length() == 6 && code.containsOnly("0123456789");
}

juce::String Client::resolveUrl(const juce::String& fileUrl) const {
    const auto trimmed = fileUrl.trim();
    if (trimmed.isEmpty() || trimmed == "null") {
        return {};
    }

    if (trimmed.startsWithIgnoreCase("https://") || trimmed.startsWithIgnoreCase("http://")) {
        return trimmed;
    }

    if (trimmed.startsWithChar('/')) {
        return _apiBaseUrl + trimmed;
    }

    return _apiBaseUrl + "/" + trimmed;
}

bool Client::sendBearerToken(const juce::String& absoluteUrl) {
    const auto host = juce::URL(absoluteUrl).getDomain();
    return host.equalsIgnoreCase("profiler.audio") || host.endsWithIgnoreCase(".profiler.audio");
}

bool saveSession(const juce::File& file, const Session& session) {
    auto user = std::make_unique<juce::DynamicObject>();
    user->setProperty("id", session.userId);
    user->setProperty("email", session.email);

    auto root = std::make_unique<juce::DynamicObject>();
    root->setProperty("accessToken", session.accessToken);
    root->setProperty("refreshToken", session.refreshToken);
    root->setProperty("expiresIn", session.expiresIn);
    root->setProperty("user", juce::var(user.release()));

    const auto parent = file.getParentDirectory();
    if (!parent.isDirectory() && !parent.createDirectory().wasOk()) {
        return false;
    }

    return file.replaceWithText(juce::JSON::toString(juce::var(root.release()), false));
}

bool loadSession(const juce::File& file, Session& session) {
    session = {};
    if (!file.existsAsFile()) {
        return false;
    }

    const auto parsed = juce::JSON::parse(file.loadFileAsString());
    if (!readSession(parsed, session)) {
        session = {};
        return false;
    }

    return true;
}

void clearSession(const juce::File& file) {
    if (file.existsAsFile()) {
        file.deleteFile();
    }
}

juce::File defaultSessionFile() {
    return getFileInSettingsFolder("marketplace_session.json");
}
}  // namespace Marketplace
