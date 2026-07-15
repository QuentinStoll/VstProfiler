#include "Components/NotificationBanner.h"
#include "TestRunner.h"

namespace profiler_tests {
namespace {

class NotificationBannerUnitTests : public juce::UnitTest {
   public:
    NotificationBannerUnitTests()
        : juce::UnitTest("NotificationBanner", "Profiler") {}

    void runTest() override {
        runCase("notification banner lifecycle", [this] {
            testLifecycle();
        });
        runCase("notification banner painting", [this] {
            testPainting();
        });
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

    void testLifecycle() {
        NotificationBanner banner;
        bool dismissed = false;
        banner.onDismissed = [&dismissed] { dismissed = true; };

        expect(!banner.isVisible(), "Banner should start hidden.");

        banner.setMessage("Ready");
        banner.setType(NotificationBanner::Type::Success);
        banner.showMessage("Profile loaded", NotificationBanner::Type::Success, 0);

        expect(banner.isVisible(), "showMessage should display the banner.");
        expectWithinAbsoluteError(banner.getIdealHeight(), 42, 0);
        expect(banner.getIdealWidth() >= 220, "Banner should enforce its minimum width.");

        banner.dismiss();
        expect(!banner.isVisible(), "dismiss should hide the banner.");
        expect(dismissed, "dismiss should invoke the callback.");

        dismissed = false;
        banner.dismiss();
        expect(!dismissed, "Dismissing an already hidden banner should do nothing.");
    }

    void testPainting() {
        NotificationBanner banner;
        banner.setBounds(0, 0, 360, banner.getIdealHeight());
        banner.setAction("Retry", [] {});
        banner.showMessage("Unable to load profile", NotificationBanner::Type::Info, 0);

        juce::Image image(juce::Image::ARGB, 360, banner.getIdealHeight(), true);
        juce::Graphics graphics(image);

        const NotificationBanner::Type types[] = {
            NotificationBanner::Type::Info,
            NotificationBanner::Type::Success,
            NotificationBanner::Type::Warning,
            NotificationBanner::Type::Error,
        };

        for (const auto type : types) {
            banner.setType(type);
            banner.paint(graphics);
        }

        banner.clearAction();
        banner.resized();
        banner.paint(graphics);

        expect(image.isValid(), "Painting should use a valid image target.");
        expect(banner.isVisible(), "Painting should not change banner visibility.");
    }
};

NotificationBannerUnitTests notificationBannerUnitTests;

}  // namespace
}  // namespace profiler_tests
