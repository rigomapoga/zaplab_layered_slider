/*
  ==============================================================================

    TextLayer.h
    Created: 30 Nov 2025
    Author:  Paolo Zappalà @ zaplab.dev
    
    Description:
    A text layer that can render text with configurable font, size, colour,
    and alignment. Supports both automatic and fixed font sizing.

  ==============================================================================
*/

#pragma once

#include "Layer.h"

namespace zaplab
{

//==============================================================================
/**
 * @brief Text alignment configuration
 */
struct TextAlignment
{
    juce::Justification horizontal { juce::Justification::centred };
    bool verticalCentre { true };
    float baselineOffset { 0.0f };
};

//==============================================================================
/**
 * @brief Auto-sizing configuration for text
 */
struct TextAutoSize
{
    bool enabled { true };
    float minSize { 8.0f };
    float maxSize { 800.0f };
    float widthMultiplier { 0.6f };  // Character width estimation factor
    float heightMultiplier { 0.8f }; // Height relative to bounds
};

//==============================================================================
/**
 * @brief Text layer - renders text with rich styling options
 */
class TextLayer : public Layer
{
public:
    //==========================================================================
    // Construction
    
    explicit TextLayer(const juce::String& name = "Text");
    ~TextLayer() override = default;
    
    //==========================================================================
    // Layer Type
    
    [[nodiscard]] LayerType getType() const noexcept override { return LayerType::Text; }
    [[nodiscard]] juce::String getTypeName() const noexcept override { return "Text"; }
    
    //==========================================================================
    // Text Content
    
    void setText(const juce::String& text);
    [[nodiscard]] const juce::String& getText() const noexcept { return text_; }
    
    // Formatted text (with value substitution)
    void setTextFormat(const juce::String& format);
    [[nodiscard]] juce::String getFormattedText() const;
    
    //==========================================================================
    // Font Configuration
    
    void setFont(const juce::Font& font);
    [[nodiscard]] const juce::Font& getFont() const noexcept { return font_; }
    
    void setFontSize(float size);
    [[nodiscard]] float getFontSize() const noexcept { return fontSize_; }
    
    void setFontFamily(const juce::String& family);
    void setFontStyle(const juce::String& style);
    
    void setFontBold(bool bold);
    void setFontItalic(bool italic);
    void setFontUnderline(bool underline);
    
    //==========================================================================
    // Text Colour
    
    void setTextColour(juce::Colour colour);
    [[nodiscard]] juce::Colour getTextColour() const noexcept { return textColour_; }
    
    // State-based colours (for toggle/hover states)
    void setTextColourOn(juce::Colour colour) { textColourOn_ = colour; }
    void setTextColourOff(juce::Colour colour) { textColourOff_ = colour; }
    void setHoverTextColour(juce::Colour colour) { hoverTextColour_ = colour; }
    
    //==========================================================================
    // Alignment
    
    void setAlignment(const TextAlignment& alignment);
    [[nodiscard]] const TextAlignment& getAlignment() const noexcept { return alignment_; }
    void setJustification(juce::Justification justification);
    [[nodiscard]] juce::Justification getJustification() const noexcept { return alignment_.horizontal; }
    
    //==========================================================================
    // Auto-sizing
    
    void setAutoSize(const TextAutoSize& autoSize);
    void setAutoSizeEnabled(bool enabled);
    [[nodiscard]] bool isAutoSizeEnabled() const noexcept { return autoSize_.enabled; }
    
    void setMinFontSize(float size);
    void setMaxFontSize(float size);
    
    //==========================================================================
    // Multiline Text
    
    void setMultiline(bool multiline);
    [[nodiscard]] bool isMultiline() const noexcept { return isMultiline_; }
    
    void setLineSpacing(float spacing);
    [[nodiscard]] float getLineSpacing() const noexcept { return lineSpacing_; }
    
    //==========================================================================
    // Vertical Text (for special layouts)
    
    void setVerticalText(bool vertical);
    [[nodiscard]] bool isVerticalText() const noexcept { return isVerticalText_; }
    
    void setVerticalCharSpacing(float spacing);
    
    //==========================================================================
    // Extra Kerning
    
    void setExtraKerning(float kerning);
    [[nodiscard]] float getExtraKerning() const noexcept { return extraKerning_; }
    
    //==========================================================================
    // Slider Integration
    
    void updateForSliderValue(float normalizedValue) override;
    
    // Value display format
    void setValueDisplayFormat(const juce::String& format);
    void setValueDisplayDecimals(int decimals);
    void setValueRange(float min, float max);
    void setValueSkew(float skewFactor, bool symmetric);
    
    // Display slider value toggle
    void setDisplaySliderValue(bool display);
    [[nodiscard]] bool isDisplayingSliderValue() const noexcept { return displaySliderValue_; }
    
    //==========================================================================
    // Computed Properties
    
    [[nodiscard]] float getComputedFontSize() const;
    [[nodiscard]] juce::Rectangle<float> getTextBounds() const;
    
    //==========================================================================
    // Serialization
    
    [[nodiscard]] juce::ValueTree toValueTree() const override;
    void fromValueTree(const juce::ValueTree& tree) override;
    
    [[nodiscard]] json toJson() const override;
    void fromJson(const json& j) override;

protected:
    //==========================================================================
    // Rendering
    
    void renderLayer(juce::Graphics& g, const juce::Rectangle<float>& bounds) override;
    
private:
    //==========================================================================
    // Helper Methods
    
    [[nodiscard]] float calculateAutoFontSize(const juce::Rectangle<float>& bounds) const;
    [[nodiscard]] juce::Colour getCurrentTextColour() const;
    void renderSingleLineText(juce::Graphics& g, const juce::Rectangle<float>& bounds);
    void renderMultiLineText(juce::Graphics& g, const juce::Rectangle<float>& bounds);
    void renderVerticalText(juce::Graphics& g, const juce::Rectangle<float>& bounds);
    
    //==========================================================================
    // Members
    
    juce::String text_;
    juce::String textFormat_;  // For value substitution, e.g., "{value} dB"
    
    juce::Font font_;
    float fontSize_ { 16.0f };
    
    juce::Colour textColour_ { juce::Colours::white };
    juce::Colour textColourOn_ { juce::Colours::white };
    juce::Colour textColourOff_ { juce::Colours::grey };
    juce::Colour hoverTextColour_ { juce::Colours::transparentWhite };
    
    TextAlignment alignment_;
    TextAutoSize autoSize_;
    
    bool isMultiline_ { false };
    float lineSpacing_ { 1.0f };
    
    bool isVerticalText_ { false };
    float verticalCharSpacing_ { 0.0f };
    
    float extraKerning_ { 0.0f };
    
    // Value display
    bool displaySliderValue_ { false };  // When true, displays the slider value instead of text_
    juce::String valueFormat_ { "{value}" };
    int valueDecimals_ { 2 };
    float valueMin_ { 0.0f };
    float valueMax_ { 1.0f };
    float valueSkewFactor_ { 1.0f };
    bool valueSymmetricSkew_ { false };
    
    JUCE_LEAK_DETECTOR(TextLayer)
};

} // namespace zaplab
