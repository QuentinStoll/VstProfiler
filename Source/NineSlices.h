//=============================================================================
//  file: NineSlices.h
//  purpose: Header file for the NineSlices class that has for goal to allow
//      for scallable non streching UI
//=============================================================================

#include "juce_graphics/juce_graphics.h"
#include <JuceHeader.h>
#include <alsa/asoundlib.h>
#include <cstdint>
#include <sys/types.h>

class NineSlices {
    enum class SliceID {
        topLeft = 0,
        top = 1,
        topRight = 2,
        left = 3,
        center = 4,
        right = 5,
        bottomLeft = 6,
        bottom = 7,
        bottomRight = 8
    };

    class Slice {
        enum class Mode {
            strech,
            tile,
            none
        };

        public:
            Slice(juce::Colour col = juce::Colours::transparentBlack);
            Slice(juce::Image& img, int32_t x = 0, int32_t y = 0, u_int32_t w = -1, u_int32_t h = -1, Mode _mode = Mode::strech);

            void setImage(juce::Colour col);
            void setImage(juce::Image& img, int32_t x = 0, int32_t y = 0, u_int32_t w = -1, u_int32_t h = -1);
            void setMode(Mode _mode);
            juce::Image& getImage(void);
            Mode& getMode(void);

        private:
            juce::Image image;
            Mode mode;
    };

    public:
        NineSlices(std::string config_path = "");

        void setSlice(SliceID id, Slice& slice);
        Slice& getSlice(SliceID id);

        void reloadConfig(std::string config_path);

    private:
        std::array<Slice, 9> slices {};
};