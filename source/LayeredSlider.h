/*
  ==============================================================================

    LayeredSlider.h
    Created: 30 Nov 2025
    Author:  DevTools Layer System
    
    Description:
    A custom slider component that uses the layer stack system to render
    beautiful, fully customizable sliders/knobs for audio plugins.

  ==============================================================================
*/

#pragma once

#include "DevToolsLayers.h"

namespace zaplab
{

//==============================================================================
/**
 * @brief Slider style/type
 */
enum class SliderStyle
{
    Rotary,             // Rotary knob
    RotaryWithLabel,    // Rotary knob with value label below
    LinearVertical,     // Vertical slider
    LinearHorizontal,   // Horizontal slider
    LinearBar,          // Bar-style slider
    TwoValueVertical,   // Two-thumb vertical
    TwoValueHorizontal  // Two-thumb horizontal
};

//==============================================================================
/**
 * @brief Rotary drag mode - how mouse movement controls the knob
 */
enum class RotaryDragMode
{
    Circular,              // Move mouse in circle around knob (like juce::Slider::Rotary)
    HorizontalDrag,        // Drag left/right (like juce::Slider::RotaryHorizontalDrag)
    VerticalDrag,          // Drag up/down (like juce::Slider::RotaryVerticalDrag)
    HorizontalVerticalDrag // Drag in any direction (like juce::Slider::RotaryHorizontalVerticalDrag)
};

//==============================================================================
/**
 * @brief Configuration for rotary sliders
 */
struct RotaryParameters
{
    float startAngleRadians { juce::MathConstants<float>::pi * 1.25f };
    float endAngleRadians { juce::MathConstants<float>::pi * 2.75f };
    bool stopAtEnd { true };
    float sensitivity { 1.0f };
    bool velocityMode { false };
    float velocitySensitivity { 0.1f };
    RotaryDragMode dragMode { RotaryDragMode::HorizontalVerticalDrag };  // Default to HV drag
};

//==============================================================================
/**
 * @brief Configuration for linear sliders  
 */
struct LinearParameters
{
    bool horizontal { false };
    float thumbSize { 20.0f };
    bool showTrack { true };
    bool showValueTrack { true };
};

//==============================================================================
/**
 * @brief LayeredSlider - A fully customizable slider using the layer system
 * 
 * Features:
 * - Rotary and linear styles
 * - Layer-based rendering (background, track, value track, thumb, overlay, text)
 * - Full animation support
 * - Hover and press states
 * - Value display with formatting
 * - Serialization to/from ValueTree
 */
class LayeredSlider : public juce::Component,
                      public juce::SettableTooltipClient
{
public:
    //==========================================================================
    // Construction
    
    LayeredSlider();
    explicit LayeredSlider(const juce::String& name);
    ~LayeredSlider() override;
    
    //==========================================================================
    // Value Control
    
    void setValue(double newValue, juce::NotificationType notification = juce::sendNotificationAsync);
    [[nodiscard]] double getValue() const noexcept { return value_; }
    
    void setRange(double minimum, double maximum, double interval = 0.0);
    [[nodiscard]] double getMinimum() const noexcept { return minimum_; }
    [[nodiscard]] double getMaximum() const noexcept { return maximum_; }
    [[nodiscard]] double getInterval() const noexcept { return interval_; }
    
    void setDoubleClickReturnValue(bool enabled, double valueToReturnTo);
    
    void setSkewFactor(double factor, bool symmetricSkew = false);
    void setSkewFactorFromMidPoint(double sliderValueToShowAtMidPoint);
    [[nodiscard]] double getSkewFactor() const noexcept { return skewFactor_; }
    
    void setBipolarMode(bool enabled);
    [[nodiscard]] bool isBipolarMode() const noexcept { return bipolarMode_; }
    
    [[nodiscard]] double proportionOfLengthToValue(double proportion) const;
    [[nodiscard]] double valueToProportionOfLength(double value) const;
    
    //==========================================================================
    // Style
    
    void setSliderStyle(SliderStyle style);
    [[nodiscard]] SliderStyle getSliderStyle() const noexcept { return style_; }
    
    void setRotaryParameters(const RotaryParameters& params);
    [[nodiscard]] const RotaryParameters& getRotaryParameters() const noexcept { return rotaryParams_; }
    
    void setLinearParameters(const LinearParameters& params);
    [[nodiscard]] const LinearParameters& getLinearParameters() const noexcept { return linearParams_; }
    
    //==========================================================================
    // Layer Access
    
    [[nodiscard]] LayerStack& getLayerStack() noexcept { return layerStack_; }
    [[nodiscard]] const LayerStack& getLayerStack() const noexcept { return layerStack_; }
    
    // Convenience layer access
    [[nodiscard]] ShapeLayer* getBackgroundLayer() const;
    [[nodiscard]] ShapeLayer* getTrackLayer() const;
    [[nodiscard]] ShapeLayer* getValueTrackLayer() const;
    [[nodiscard]] ShapeLayer* getThumbLayer() const;
    [[nodiscard]] TextLayer* getValueTextLayer() const;
    
    //==========================================================================
    // Text Value Display
    
    void setTextValueSuffix(const juce::String& suffix);
    [[nodiscard]] const juce::String& getTextValueSuffix() const noexcept { return textSuffix_; }
    
    void setNumDecimalPlacesToDisplay(int places);
    [[nodiscard]] int getNumDecimalPlacesToDisplay() const noexcept { return numDecimalPlaces_; }
    
    void setShowValueText(bool show);
    [[nodiscard]] bool isShowingValueText() const noexcept { return showValueText_; }
    
    // Custom text conversion functions
    std::function<juce::String(double)> textFromValueFunction;
    std::function<double(const juce::String&)> valueFromTextFunction;
    
    [[nodiscard]] juce::String getTextFromValue(double value) const;
    [[nodiscard]] double getValueFromText(const juce::String& text) const;
    
    //==========================================================================
    // Presets & Templates
    
    void loadPreset(const juce::String& presetName);
    void loadFromValueTree(const juce::ValueTree& tree);
    [[nodiscard]] juce::ValueTree saveToValueTree() const;
    
    bool loadFromJson(const json& j);
    [[nodiscard]] json saveToJson() const;
    
    // Convenience loading methods
    bool loadPresetFile(const juce::File& file);
    bool loadPresetFile(const juce::String& filePath);
    bool loadFromMemory(const juce::String& presetName);
    bool loadFromBase64(const juce::String& base64EncodedJson);
    
    // Built-in presets
    void applyRotaryKnobPreset();
    void applyMinimalRotaryPreset();
    void applyArcKnobPreset();
    void applyLinearFaderPreset();
    void applyModernKnobPreset();
    
    // Additional beautiful presets
    void applyNeonGlowKnobPreset();
    void applyVintageKnobPreset();
    void applyGlassyKnobPreset();
    void applyMetallicKnobPreset();
    void applyFlatMaterialPreset();
    void applyGradientSliderPreset();
    void applySkeumorphicKnobPreset();
    void applyMinimalLineKnobPreset();
    
    //==========================================================================
    // Export
    
    [[nodiscard]] juce::Image renderToImage(int width, int height) const;
    [[nodiscard]] juce::Image renderFilmstrip(int frameWidth, int frameHeight, 
                                               int numFrames, bool vertical = true) const;
    
    //==========================================================================
    // Callbacks
    
    std::function<void()> onValueChange;
    std::function<void()> onDragStart;
    std::function<void()> onDragEnd;
    
    // For integration with juce::Slider::Listener pattern
    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void sliderValueChanged(LayeredSlider* slider) = 0;
        virtual void sliderDragStarted(LayeredSlider* slider) {}
        virtual void sliderDragEnded(LayeredSlider* slider) {}
    };
    
    void addListener(Listener* listener);
    void removeListener(Listener* listener);

protected:
    //==========================================================================
    // Component Overrides
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;
    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
    
    bool keyPressed(const juce::KeyPress& key) override;
    
    void focusGained(FocusChangeType cause) override;
    void focusLost(FocusChangeType cause) override;
    
private:
    //==========================================================================
    // Internal Methods
    
    void setupDefaultLayers();
    void updateLayersForValue();
    void updateInteractionState(bool hovered, bool pressed);
    
    [[nodiscard]] double getValueFromMousePosition(juce::Point<float> pos) const;
    [[nodiscard]] double constrainedValue(double value) const;
    [[nodiscard]] double snapValue(double value) const;
    
    void sendValueChangedMessage(juce::NotificationType notification);
    
    void handleRotaryDrag(const juce::MouseEvent& e);
    void handleLinearDrag(const juce::MouseEvent& e);
    
    //==========================================================================
    // Members
    
    // Value state
    double value_ { 0.0 };
    double minimum_ { 0.0 };
    double maximum_ { 1.0 };
    double interval_ { 0.0 };
    double skewFactor_ { 1.0 };
    bool symmetricSkew_ { false };
    
    double doubleClickReturnValue_ { 0.0 };
    bool hasDoubleClickReturnValue_ { false };
    
    // Bipolar mode (for panning/balance knobs)
    bool bipolarMode_ { false };
    
    // Style
    SliderStyle style_ { SliderStyle::Rotary };
    RotaryParameters rotaryParams_;
    LinearParameters linearParams_;
    
    // Text display
    juce::String textSuffix_;
    int numDecimalPlaces_ { 2 };
    bool showValueText_ { true };
    
    // Layer system
    LayerStack layerStack_;
    
    // Interaction state
    bool isHovered_ { false };
    bool isDragging_ { false };
    juce::Point<float> lastMousePos_;
    double valueOnMouseDown_ { 0.0 };
    
    // Listeners
    juce::ListenerList<Listener> listeners_;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LayeredSlider)
};

} // namespace zaplab
