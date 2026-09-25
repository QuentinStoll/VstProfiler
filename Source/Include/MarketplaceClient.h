#pragma once

#include <JuceHeader.h>

#include <memory>
#include <vector>

namespace Marketplace {
inline constexpr const char* baseUrl = "https://profiler.audio";
inline constexpr const char* signupUrl = "https://profiler.audio/login";

enum class Status {
    Success,
    MfaRequired,
    Unauthorized,
    BadRequest,
    Unavailable,
    TransportError,
    Unexpected
};

struct Session {
    juce::String accessToken;
    juce::String refreshToken;
    int expiresIn = 0;
    juce::String userId;
    juce::String email;
};

struct MfaChallenge {
    juce::String factorId;
    juce::String challengeId;
};

struct LoginResult {
    Status status = Status::Unexpected;
    Session session;
    MfaChallenge challenge;
    juce::String message;
};

struct LibraryItem {
    juce::String entitlementId;
    juce::String acquiredAt;
    juce::String source;
    juce::String id;
    juce::String title;
    juce::String authorName;
    juce::String fileUrl;
    juce::String irFileUrl;
    bool hasIntegratedIr = false;
};

struct LibraryResult {
    Status status = Status::Unexpected;
    juce::String email;
    std::vector<LibraryItem> items;
    juce::String message;
};

struct DownloadResult {
    Status status = Status::Unexpected;
    juce::MemoryBlock body;
    juce::String message;
};

struct HttpResponse {
    int statusCode = 0;
    juce::MemoryBlock body;
};

class HttpTransport {
   public:
    virtual ~HttpTransport() = default;

    virtual HttpResponse send(const juce::String& method,
                              const juce::String& url,
                              const juce::String& jsonBody,
                              const juce::String& bearerToken) = 0;
};

class Client {
   public:
    explicit Client(std::shared_ptr<HttpTransport> transport = nullptr,
                    juce::String apiBaseUrl = baseUrl);

    LoginResult login(const juce::String& email, const juce::String& password) const;
    LoginResult submitCode(const juce::String& factorId,
                           const juce::String& challengeId,
                           const juce::String& code) const;
    LibraryResult fetchLibrary(const juce::String& accessToken) const;
    DownloadResult download(const juce::String& fileUrl, const juce::String& accessToken) const;

    static bool isSixDigitCode(const juce::String& code);

   private:
    std::shared_ptr<HttpTransport> _transport;
    juce::String _apiBaseUrl;

    juce::String resolveUrl(const juce::String& fileUrl) const;
    static bool sendBearerToken(const juce::String& absoluteUrl);
};

bool saveSession(const juce::File& file, const Session& session);
bool loadSession(const juce::File& file, Session& session);
void clearSession(const juce::File& file);
juce::File defaultSessionFile();
}  // namespace Marketplace
