#include "MarketplaceClient.h"
#include "TestRunner.h"

namespace profiler_tests {
namespace {

class FakeTransport : public Marketplace::HttpTransport {
   public:
    juce::String method;
    juce::String url;
    juce::String jsonBody;
    juce::String bearerToken;
    int statusCode = 200;
    juce::String responseBody;

    Marketplace::HttpResponse send(const juce::String& requestMethod,
                                   const juce::String& requestUrl,
                                   const juce::String& requestJson,
                                   const juce::String& requestToken) override {
        method = requestMethod;
        url = requestUrl;
        jsonBody = requestJson;
        bearerToken = requestToken;

        Marketplace::HttpResponse response;
        response.statusCode = statusCode;
        response.body.append(responseBody.toRawUTF8(), responseBody.getNumBytesAsUTF8());
        return response;
    }
};

class MarketplaceClientTests : public juce::UnitTest {
   public:
    MarketplaceClientTests()
        : juce::UnitTest("MarketplaceClient", "Profiler") {}

    void runTest() override {
        runCase("marketplace login", [this] { testLogin(); });
        runCase("marketplace mfa", [this] { testMfa(); });
        runCase("marketplace library", [this] { testLibrary(); });
        runCase("marketplace session file", [this] { testSessionFile(); });
    }

   private:
    template <typename Fn>
    void runCase(const juce::String& name, Fn&& fn) {
        if (requestedTestCase.isNotEmpty() && requestedTestCase != name) {
            return;
        }

        beginTest(name);
        fn();
    }

    void testLogin() {
        auto transport = std::make_shared<FakeTransport>();
        Marketplace::Client client(transport, "https://profiler.audio");

        transport->statusCode = 401;
        transport->responseBody = R"({"message":"nope"})";
        auto refused = client.login("a@profiler.audio", "secret");
        expect(refused.status == Marketplace::Status::Unauthorized);
        expect(transport->method == "POST");
        expect(transport->url == "https://profiler.audio/api/v1/plugin/login");
        expect(transport->bearerToken.isEmpty());
        expect(!refused.message.contains("secret"));

        transport->statusCode = 403;
        transport->responseBody = R"({"mfaRequired":true,"factorId":"fac_1","challengeId":"ch_1"})";
        auto challenge = client.login("a@profiler.audio", "secret");
        expect(challenge.status == Marketplace::Status::MfaRequired);
        expectEquals(challenge.challenge.factorId, juce::String("fac_1"));
        expectEquals(challenge.challenge.challengeId, juce::String("ch_1"));
        expect(challenge.session.accessToken.isEmpty());

        transport->statusCode = 200;
        transport->responseBody = R"({"mfaRequired":false,"accessToken":"access-secret","refreshToken":"refresh-secret","expiresIn":3600,"user":{"id":"user-1","email":"a@profiler.audio"}})";
        auto signedIn = client.login("a@profiler.audio", "secret");
        expect(signedIn.status == Marketplace::Status::Success);
        expectEquals(signedIn.session.accessToken, juce::String("access-secret"));
        expectEquals(signedIn.session.refreshToken, juce::String("refresh-secret"));
        expectEquals(signedIn.session.expiresIn, 3600);
        expectEquals(signedIn.session.userId, juce::String("user-1"));
        expect(!signedIn.message.contains("access-secret"));

        transport->statusCode = 400;
        auto invalid = client.login("a@profiler.audio", "secret");
        expect(invalid.status == Marketplace::Status::BadRequest);

        transport->statusCode = 503;
        auto unavailable = client.login("a@profiler.audio", "secret");
        expect(unavailable.status == Marketplace::Status::Unavailable);

        transport->statusCode = 0;
        auto offline = client.login("a@profiler.audio", "secret");
        expect(offline.status == Marketplace::Status::TransportError);
    }

    void testMfa() {
        auto transport = std::make_shared<FakeTransport>();
        Marketplace::Client client(transport);

        auto missing = client.submitCode("fac_1", "ch_1", "12");
        expect(missing.status == Marketplace::Status::BadRequest);
        expect(transport->url.isEmpty());

        transport->statusCode = 200;
        transport->responseBody = R"({"mfaRequired":false,"accessToken":"access-secret","refreshToken":"refresh-secret","expiresIn":3600,"user":{"id":"user-1","email":"a@profiler.audio"}})";
        auto signedIn = client.submitCode("fac_1", "ch_1", "123456");
        expect(signedIn.status == Marketplace::Status::Success);
        expect(transport->method == "POST");
        expect(transport->url.endsWith("/api/v1/plugin/mfa"));
        expect(transport->jsonBody.contains("\"code\": \"123456\"") || transport->jsonBody.contains("\"code\":\"123456\""));
        expect(transport->bearerToken.isEmpty());
        expectEquals(signedIn.session.email, juce::String("a@profiler.audio"));

        transport->statusCode = 401;
        auto expired = client.submitCode("fac_1", "ch_1", "000000");
        expect(expired.status == Marketplace::Status::Unauthorized);
        expect(expired.session.accessToken.isEmpty());

        transport->statusCode = 400;
        auto bad = client.submitCode("fac_1", "ch_1", "123456");
        expect(bad.status == Marketplace::Status::BadRequest);
    }

    void testLibrary() {
        auto transport = std::make_shared<FakeTransport>();
        Marketplace::Client client(transport);

        auto missingToken = client.fetchLibrary({});
        expect(missingToken.status == Marketplace::Status::Unauthorized);
        expect(transport->url.isEmpty());

        transport->statusCode = 200;
        transport->responseBody = R"({"user":{"id":"user-1","email":"a@profiler.audio"},"library":[{"entitlementId":"ent-1","acquiredAt":"2026-09-01T00:00:00Z","source":"purchase","id":"pack-1","title":"Clean","authorName":"Ada","fileUrl":"/files/clean.profilerprofile","irFileUrl":"https://cdn.example/ir.wav","hasIntegratedIr":false},{"entitlementId":"ent-2","acquiredAt":"2026-09-02T00:00:00Z","source":"free","id":"pack-2","title":"Lead","authorName":"Bea","fileUrl":"https://profiler.audio/files/lead.profilerprofile","irFileUrl":null,"hasIntegratedIr":true}]})";
        auto library = client.fetchLibrary("access-secret");
        expect(library.status == Marketplace::Status::Success);
        expect(transport->method == "GET");
        expect(transport->url == "https://profiler.audio/api/v1/plugin/library");
        expectEquals(transport->bearerToken, juce::String("access-secret"));
        expectEquals(static_cast<int>(library.items.size()), 2);
        expectEquals(library.items[0].source, juce::String("purchase"));
        expectEquals(library.items[0].irFileUrl, juce::String("https://cdn.example/ir.wav"));
        expect(!library.items[0].hasIntegratedIr);
        expect(library.items[1].irFileUrl.isEmpty());
        expect(library.items[1].hasIntegratedIr);
        expect(library.items[1].source == "free");

        transport->statusCode = 401;
        auto expired = client.fetchLibrary("access-secret");
        expect(expired.status == Marketplace::Status::Unauthorized);

        transport->statusCode = 403;
        auto forbidden = client.fetchLibrary("access-secret");
        expect(forbidden.status == Marketplace::Status::Unexpected);

        transport->bearerToken.clear();
        transport->statusCode = 200;
        transport->responseBody = "profile-bytes";
        auto externalFile = client.download("https://cdn.example/ir.wav", "access-secret");
        expect(externalFile.status == Marketplace::Status::Success);
        expect(transport->bearerToken.isEmpty());
        expectEquals(externalFile.body.toString(), juce::String("profile-bytes"));

        auto hostedFile = client.download("/files/clean.profilerprofile", "access-secret");
        expect(hostedFile.status == Marketplace::Status::Success);
        expect(transport->url == "https://profiler.audio/files/clean.profilerprofile");
        expectEquals(transport->bearerToken, juce::String("access-secret"));
    }

    void testSessionFile() {
        const auto file = juce::File::getSpecialLocation(juce::File::tempDirectory)
                              .getChildFile("profiler-marketplace-session-test.json");
        file.deleteFile();

        Marketplace::Session session;
        session.accessToken = "access-secret";
        session.refreshToken = "refresh-secret";
        session.expiresIn = 120;
        session.userId = "user-1";
        session.email = "a@profiler.audio";

        expect(Marketplace::saveSession(file, session));
        Marketplace::Session loaded;
        expect(Marketplace::loadSession(file, loaded));
        expectEquals(loaded.accessToken, session.accessToken);
        expectEquals(loaded.refreshToken, session.refreshToken);
        expectEquals(loaded.expiresIn, 120);
        expectEquals(loaded.email, session.email);

        Marketplace::clearSession(file);
        expect(!file.existsAsFile());
        expect(!Marketplace::loadSession(file, loaded));
        expect(loaded.accessToken.isEmpty());
    }
};

MarketplaceClientTests marketplaceClientTests;

}  // namespace
}  // namespace profiler_tests
