/*
  ==============================================================================

    TextLayer.cpp
    Created: 30 Nov 2025
    Author:  DevTools Refactored Layer System

  ==============================================================================
*/

#include "TextLayer.h"

namespace zaplab
{

//==============================================================================
// Additional IDs for TextLayer serialization
namespace TextIDs
{
    static const juce::Identifier Text { "text" };
    static const juce::Identifier TextFormat { "textFormat" };
    static const juce::Identifier FontFamily { "fontFamily" };
    static const juce::Identifier FontStyle { "fontStyle" };
    static const juce::Identifier FontSize { "fontSize" };
    static const juce::Identifier FontBold { "bold" };
    static const juce::Identifier FontItalic { "italic" };
    static const juce::Identifier TextColour { "textColour" };
    static const juce::Identifier TextColourOn { "textColourOn" };
    static const juce::Identifier TextColourOff { "textColourOff" };
    static const juce::Identifier HoverTextColour { "hoverTextColour" };
    static const juce::Identifier Justification { "justification" };
    static const juce::Identifier AutoSize { "autoSize" };
    static const juce::Identifier MinFontSize { "minFontSize" };
    static const juce::Identifier MaxFontSize { "maxFontSize" };
    static const juce::Identifier Multiline { "multiline" };
    static const juce::Identifier LineSpacing { "lineSpacing" };
    static const juce::Identifier VerticalText { "verticalText" };
    static const juce::Identifier ExtraKerning { "extraKerning" };
    static const juce::Identifier ValueFormat { "valueFormat" };
    static const juce::Identifier ValueDecimals { "valueDecimals" };
    static const juce::Identifier DisplaySliderValue { "displaySliderValue" };
}

//==============================================================================
TextLayer::TextLayer(const juce::String& name)
    : Layer(name)
{
    // Default font
    font_ = juce::Font(fontSize_);
}

//==============================================================================
// Text Content

void TextLayer::setText(const juce::String& text)
{
    if (text_ != text)
    {
        text_ = text;
        repaint();
    }
}

void TextLayer::setTextFormat(const juce::String& format)
{
    textFormat_ = format;
    repaint();
}

juce::String TextLayer::getFormattedText() const
{
    // Helper lambda to apply skew to a normalized value (0-1) and convert to display value
    auto applySkewAndRange = [this](float normalizedValue) -> float
    {
        float proportion = normalizedValue;
        
        if (valueSkewFactor_ != 1.0f)
        {
            if (valueSymmetricSkew_)
            {
                const float distanceFromMiddle = 2.0f * proportion - 1.0f;
                const float sign = distanceFromMiddle < 0 ? -1.0f : 1.0f;
                proportion = 0.5f + sign * 0.5f * std::pow(std::abs(distanceFromMiddle), valueSkewFactor_);
            }
            else
            {
                proportion = std::pow(proportion, valueSkewFactor_);
            }
        }
        
        return valueMin_ + (valueMax_ - valueMin_) * proportion;
    };
    
    // If displaying slider value, format it appropriately
    if (displaySliderValue_)
    {
        const float displayValue = applySkewAndRange(sliderValue_);
        
        if (valueFormat_.isNotEmpty() && valueFormat_ != "{value}")
        {
            juce::String result = valueFormat_;
            result = result.replace("{value}", juce::String(displayValue, valueDecimals_));
            result = result.replace("{percent}", juce::String(sliderValue_ * 100.0f, 0) + "%");
            return result;
        }
        
        return juce::String(displayValue, valueDecimals_);
    }
    
    // Normal text formatting with optional value substitution
    if (textFormat_.isEmpty())
        return text_;
    
    // Replace {value} with the current slider value
    juce::String result = textFormat_;
    
    const float displayValue = applySkewAndRange(sliderValue_);
    result = result.replace("{value}", juce::String(displayValue, valueDecimals_));
    result = result.replace("{percent}", juce::String(sliderValue_ * 100.0f, 0) + "%");
    
    return result;
}

//==============================================================================
// Font Configuration

void TextLayer::setFont(const juce::Font& font)
{
    font_ = font;
    fontSize_ = font.getHeight();
    repaint();
}

void TextLayer::setFontSize(float size)
{
    size = std::max(1.0f, size);
    
    if (fontSize_ != size)
    {
        fontSize_ = size;
        font_ = font_.withHeight(size);
        repaint();
    }
}

void TextLayer::setFontFamily(const juce::String& family)
{
    font_ = juce::Font(family, fontSize_, font_.getStyleFlags());
    repaint();
}

void TextLayer::setFontStyle(const juce::String& style)
{
    font_ = font_.withTypefaceStyle(style);
    repaint();
}

void TextLayer::setFontBold(bool bold)
{
    font_ = font_.withStyle(bold 
        ? (font_.getStyleFlags() | juce::Font::bold)
        : (font_.getStyleFlags() & ~juce::Font::bold));
    repaint();
}

void TextLayer::setFontItalic(bool italic)
{
    font_ = font_.withStyle(italic 
        ? (font_.getStyleFlags() | juce::Font::italic)
        : (font_.getStyleFlags() & ~juce::Font::italic));
    repaint();
}

void TextLayer::setFontUnderline(bool underline)
{
    font_ = font_.withStyle(underline 
        ? (font_.getStyleFlags() | juce::Font::underlined)
        : (font_.getStyleFlags() & ~juce::Font::underlined));
    repaint();
}

//==============================================================================
// Text Colour

void TextLayer::setTextColour(juce::Colour colour)
{
    if (textColour_ != colour)
    {
        textColour_ = colour;
        repaint();
    }
}

//==============================================================================
// Alignment

void TextLayer::setAlignment(const TextAlignment& alignment)
{
    alignment_ = alignment;
    repaint();
}

void TextLayer::setJustification(juce::Justification justification)
{
    alignment_.horizontal = justification;
    repaint();
}

//==============================================================================
// Auto-sizing

void TextLayer::setAutoSize(const TextAutoSize& autoSize)
{
    autoSize_ = autoSize;
    repaint();
}

void TextLayer::setAutoSizeEnabled(bool enabled)
{
    if (autoSize_.enabled != enabled)
    {
        autoSize_.enabled = enabled;
        repaint();
    }
}

void TextLayer::setMinFontSize(float size)
{
    autoSize_.minSize = std::max(1.0f, size);
    repaint();
}

void TextLayer::setMaxFontSize(float size)
{
    autoSize_.maxSize = std::max(autoSize_.minSize, size);
    repaint();
}

//==============================================================================
// Multiline Text

void TextLayer::setMultiline(bool multiline)
{
    if (isMultiline_ != multiline)
    {
        isMultiline_ = multiline;
        repaint();
    }
}

void TextLayer::setLineSpacing(float spacing)
{
    lineSpacing_ = std::max(0.5f, spacing);
    repaint();
}

//==============================================================================
// Vertical Text

void TextLayer::setVerticalText(bool vertical)
{
    if (isVerticalText_ != vertical)
    {
        isVerticalText_ = vertical;
        repaint();
    }
}

void TextLayer::setVerticalCharSpacing(float spacing)
{
    verticalCharSpacing_ = spacing;
    repaint();
}

//==============================================================================
// Extra Kerning

void TextLayer::setExtraKerning(float kerning)
{
    if (extraKerning_ != kerning)
    {
        extraKerning_ = kerning;
        font_ = font_.withExtraKerningFactor(kerning);
        repaint();
    }
}

//==============================================================================
// Slider Integration

void TextLayer::updateForSliderValue(float normalizedValue)
{
    Layer::updateForSliderValue(normalizedValue);
    
    // If displaying slider value or we have a text format with value, trigger repaint
    if (displaySliderValue_ || textFormat_.isNotEmpty())
    {
        repaint();
    }
}

void TextLayer::setValueDisplayFormat(const juce::String& format)
{
    valueFormat_ = format;
    textFormat_ = format;
    repaint();
}

void TextLayer::setValueDisplayDecimals(int decimals)
{
    valueDecimals_ = juce::jlimit(0, 10, decimals);
    repaint();
}

void TextLayer::setValueRange(float min, float max)
{
    valueMin_ = min;
    valueMax_ = max;
    repaint();
}

void TextLayer::setValueSkew(float skewFactor, bool symmetric)
{
    valueSkewFactor_ = skewFactor;
    valueSymmetricSkew_ = symmetric;
    repaint();
}

void TextLayer::setDisplaySliderValue(bool display)
{
    if (displaySliderValue_ != display)
    {
        displaySliderValue_ = display;
        repaint();
    }
}

//==============================================================================
// Computed Properties

float TextLayer::getComputedFontSize() const
{
    if (!autoSize_.enabled)
        return fontSize_;
    
    return calculateAutoFontSize(getLocalBounds().toFloat());
}

juce::Rectangle<float> TextLayer::getTextBounds() const
{
    const juce::String displayText = (displaySliderValue_ || textFormat_.isNotEmpty()) ? getFormattedText() : text_;
    const float computedSize = getComputedFontSize();
    const auto currentFont = font_.withHeight(computedSize);
    
    // Calculate text bounds using GlyphArrangement for accurate bounds
    juce::GlyphArrangement glyphs;
    glyphs.addLineOfText(currentFont, displayText, 0.0f, 0.0f);
    return glyphs.getBoundingBox(0, glyphs.getNumGlyphs(), false);
}

//==============================================================================
// Serialization

juce::ValueTree TextLayer::toValueTree() const
{
    auto tree = Layer::toValueTree();
    
    tree.setProperty(TextIDs::Text, text_, nullptr);
    tree.setProperty(TextIDs::TextFormat, textFormat_, nullptr);
    tree.setProperty(TextIDs::FontFamily, font_.getTypefaceName(), nullptr);
    tree.setProperty(TextIDs::FontStyle, font_.getTypefaceStyle(), nullptr);
    tree.setProperty(TextIDs::FontSize, fontSize_, nullptr);
    tree.setProperty(TextIDs::FontBold, font_.isBold(), nullptr);
    tree.setProperty(TextIDs::FontItalic, font_.isItalic(), nullptr);
    tree.setProperty(TextIDs::TextColour, textColour_.toString(), nullptr);
    tree.setProperty(TextIDs::TextColourOn, textColourOn_.toString(), nullptr);
    tree.setProperty(TextIDs::TextColourOff, textColourOff_.toString(), nullptr);
    tree.setProperty(TextIDs::HoverTextColour, hoverTextColour_.toString(), nullptr);
    tree.setProperty(TextIDs::Justification, alignment_.horizontal.getFlags(), nullptr);
    tree.setProperty(TextIDs::AutoSize, autoSize_.enabled, nullptr);
    tree.setProperty(TextIDs::MinFontSize, autoSize_.minSize, nullptr);
    tree.setProperty(TextIDs::MaxFontSize, autoSize_.maxSize, nullptr);
    tree.setProperty(TextIDs::Multiline, isMultiline_, nullptr);
    tree.setProperty(TextIDs::LineSpacing, lineSpacing_, nullptr);
    tree.setProperty(TextIDs::VerticalText, isVerticalText_, nullptr);
    tree.setProperty(TextIDs::ExtraKerning, extraKerning_, nullptr);
    tree.setProperty(TextIDs::ValueFormat, valueFormat_, nullptr);
    tree.setProperty(TextIDs::ValueDecimals, valueDecimals_, nullptr);
    tree.setProperty(TextIDs::DisplaySliderValue, displaySliderValue_, nullptr);
    
    return tree;
}

void TextLayer::fromValueTree(const juce::ValueTree& tree)
{
    Layer::fromValueTree(tree);
    
    text_ = tree.getProperty(TextIDs::Text, "").toString();
    textFormat_ = tree.getProperty(TextIDs::TextFormat, "").toString();
    
    const auto family = tree.getProperty(TextIDs::FontFamily, "").toString();
    const auto style = tree.getProperty(TextIDs::FontStyle, "").toString();
    fontSize_ = tree.getProperty(TextIDs::FontSize, 16.0f);
    
    if (family.isNotEmpty())
    {
        font_ = juce::Font(family, fontSize_, 0);
        if (style.isNotEmpty())
            font_ = font_.withTypefaceStyle(style);
    }
    else
    {
        font_ = juce::Font(fontSize_);
    }
    
    if (static_cast<bool>(tree.getProperty(TextIDs::FontBold, false)))
        font_ = font_.boldened();
    if (static_cast<bool>(tree.getProperty(TextIDs::FontItalic, false)))
        font_ = font_.italicised();
    
    textColour_ = juce::Colour::fromString(tree.getProperty(TextIDs::TextColour, "ffffffff").toString());
    textColourOn_ = juce::Colour::fromString(tree.getProperty(TextIDs::TextColourOn, "ffffffff").toString());
    textColourOff_ = juce::Colour::fromString(tree.getProperty(TextIDs::TextColourOff, "ff808080").toString());
    hoverTextColour_ = juce::Colour::fromString(tree.getProperty(TextIDs::HoverTextColour, "00000000").toString());
    
    alignment_.horizontal = juce::Justification(static_cast<int>(tree.getProperty(TextIDs::Justification, 36)));
    
    autoSize_.enabled = tree.getProperty(TextIDs::AutoSize, true);
    autoSize_.minSize = tree.getProperty(TextIDs::MinFontSize, 8.0f);
    autoSize_.maxSize = tree.getProperty(TextIDs::MaxFontSize, 800.0f);
    
    isMultiline_ = tree.getProperty(TextIDs::Multiline, false);
    lineSpacing_ = tree.getProperty(TextIDs::LineSpacing, 1.0f);
    isVerticalText_ = tree.getProperty(TextIDs::VerticalText, false);
    extraKerning_ = tree.getProperty(TextIDs::ExtraKerning, 0.0f);
    valueFormat_ = tree.getProperty(TextIDs::ValueFormat, "{value}").toString();
    valueDecimals_ = tree.getProperty(TextIDs::ValueDecimals, 2);
    displaySliderValue_ = tree.getProperty(TextIDs::DisplaySliderValue, false);
}

//==============================================================================
// Rendering

void TextLayer::renderLayer(juce::Graphics& g, const juce::Rectangle<float>& bounds)
{
    const juce::String displayText = (displaySliderValue_ || textFormat_.isNotEmpty()) ? getFormattedText() : text_;
    
    if (displayText.isEmpty())
        return;
    
    // Set colour
    g.setColour(getCurrentTextColour());
    
    // Set font with computed size
    const float computedSize = calculateAutoFontSize(bounds);
    g.setFont(font_.withHeight(computedSize));
    
    if (isVerticalText_)
    {
        renderVerticalText(g, bounds);
    }
    else if (isMultiline_)
    {
        renderMultiLineText(g, bounds);
    }
    else
    {
        renderSingleLineText(g, bounds);
    }
}

//==============================================================================
// Helper Methods

float TextLayer::calculateAutoFontSize(const juce::Rectangle<float>& bounds) const
{
    if (!autoSize_.enabled)
        return fontSize_;
    
    const juce::String displayText = (displaySliderValue_ || textFormat_.isNotEmpty()) ? getFormattedText() : text_;
    
    if (displayText.isEmpty())
        return fontSize_;
    
    // Calculate font size based on available space
    float heightBasedSize = bounds.getHeight() * autoSize_.heightMultiplier;
    float widthBasedSize = bounds.getWidth() / (displayText.length() * autoSize_.widthMultiplier);
    
    float computedSize = std::min(heightBasedSize, widthBasedSize);
    
    return juce::jlimit(autoSize_.minSize, autoSize_.maxSize, computedSize);
}

juce::Colour TextLayer::getCurrentTextColour() const
{
    // Check interaction states
    const auto& interaction = getInteractionState();
    
    if (interaction.isHovered && hoverTextColour_.getAlpha() > 0)
        return hoverTextColour_;
    
    if (interaction.isToggled)
        return textColourOn_;
    
    return textColour_;
}

void TextLayer::renderSingleLineText(juce::Graphics& g, const juce::Rectangle<float>& bounds)
{
    const juce::String displayText = (displaySliderValue_ || textFormat_.isNotEmpty()) ? getFormattedText() : text_;
    g.drawText(displayText, bounds, alignment_.horizontal, true);
}

void TextLayer::renderMultiLineText(juce::Graphics& g, const juce::Rectangle<float>& bounds)
{
    const juce::String displayText = (displaySliderValue_ || textFormat_.isNotEmpty()) ? getFormattedText() : text_;
    
    // Split by newlines
    juce::StringArray lines;
    lines.addTokens(displayText, "\n", "");
    
    const float lineHeight = g.getCurrentFont().getHeight() * lineSpacing_;
    float y = bounds.getY();
    
    // Vertical centering if needed
    if (alignment_.verticalCentre)
    {
        const float totalHeight = lines.size() * lineHeight;
        y = bounds.getY() + (bounds.getHeight() - totalHeight) * 0.5f;
    }
    
    for (const auto& line : lines)
    {
        g.drawText(line, bounds.withY(y).withHeight(lineHeight), alignment_.horizontal, true);
        y += lineHeight;
    }
}

void TextLayer::renderVerticalText(juce::Graphics& g, const juce::Rectangle<float>& bounds)
{
    const juce::String displayText = (displaySliderValue_ || textFormat_.isNotEmpty()) ? getFormattedText() : text_;
    const auto& currentFont = g.getCurrentFont();
    const float charHeight = currentFont.getHeight() + verticalCharSpacing_;
    
    const float totalHeight = displayText.length() * charHeight;
    float y = bounds.getY();
    
    // Vertical centering
    if (alignment_.verticalCentre)
    {
        y = bounds.getY() + (bounds.getHeight() - totalHeight) * 0.5f;
    }
    
    for (int i = 0; i < displayText.length(); ++i)
    {
        const juce::String charStr = displayText.substring(i, i + 1);
        const float charWidth = currentFont.getStringWidthFloat(charStr);
        
        // Centre horizontally
        const float x = bounds.getCentreX() - charWidth * 0.5f;
        
        g.drawText(charStr, juce::Rectangle<float>(x, y, charWidth, currentFont.getHeight()), 
                   juce::Justification::centred, false);
        
        y += charHeight;
    }
}

//==============================================================================
// JSON Serialization

json TextLayer::toJson() const
{
    json j = Layer::toJson();
    
    // Text-specific properties
    j["text"] = text_.toStdString();
    j["font"] = font_;
    j["fontSize"] = fontSize_;
    j["textColour"] = textColour_;
    j["alignment"] = {
        {"horizontal", alignment_.horizontal.getFlags()},
        {"verticalCentre", alignment_.verticalCentre},
        {"baselineOffset", alignment_.baselineOffset}
    };
    
    // Interactive text colours
    j["textColourHover"] = hoverTextColour_;
    j["textColourOn"] = textColourOn_;
    j["textColourOff"] = textColourOff_;
    
    // Multi-line and vertical text
    j["isMultiline"] = isMultiline_;
    j["lineSpacing"] = lineSpacing_;
    j["isVerticalText"] = isVerticalText_;
    j["verticalCharSpacing"] = verticalCharSpacing_;
    j["extraKerning"] = extraKerning_;
    
    // Slider value display
    j["displaySliderValue"] = displaySliderValue_;
    j["textFormat"] = textFormat_.toStdString();
    j["valueFormat"] = valueFormat_.toStdString();
    j["valueDecimals"] = valueDecimals_;
    j["valueMin"] = valueMin_;
    j["valueMax"] = valueMax_;
    j["valueSkewFactor"] = valueSkewFactor_;
    j["valueSymmetricSkew"] = valueSymmetricSkew_;
    
    // Auto-size
    j["autoSize"] = {
        {"enabled", autoSize_.enabled},
        {"minSize", autoSize_.minSize},
        {"maxSize", autoSize_.maxSize},
        {"widthMultiplier", autoSize_.widthMultiplier},
        {"heightMultiplier", autoSize_.heightMultiplier}
    };
    
    return j;
}

void TextLayer::fromJson(const json& j)
{
    Layer::fromJson(j);
    
    // Text-specific properties
    if (j.contains("text"))
        text_ = j["text"].get<std::string>();
    
    if (j.contains("font"))
        j["font"].get_to(font_);
    
    fontSize_ = j.value("fontSize", 16.0f);
    
    if (j.contains("textColour"))
        j["textColour"].get_to(textColour_);
    
    // Alignment
    if (j.contains("alignment"))
    {
        const auto& align = j["alignment"];
        alignment_.horizontal = juce::Justification(align.value("horizontal", 36));
        alignment_.verticalCentre = align.value("verticalCentre", true);
        alignment_.baselineOffset = align.value("baselineOffset", 0.0f);
    }
    
    // Interactive text colours
    if (j.contains("textColourHover"))
        j["textColourHover"].get_to(hoverTextColour_);
    if (j.contains("textColourOn"))
        j["textColourOn"].get_to(textColourOn_);
    if (j.contains("textColourOff"))
        j["textColourOff"].get_to(textColourOff_);
    
    // Multi-line and vertical text
    isMultiline_ = j.value("isMultiline", false);
    lineSpacing_ = j.value("lineSpacing", 1.0f);
    isVerticalText_ = j.value("isVerticalText", false);
    verticalCharSpacing_ = j.value("verticalCharSpacing", 0.0f);
    extraKerning_ = j.value("extraKerning", 0.0f);
    
    // Slider value display
    displaySliderValue_ = j.value("displaySliderValue", false);
    if (j.contains("textFormat"))
        textFormat_ = j["textFormat"].get<std::string>();
    if (j.contains("valueFormat"))
        valueFormat_ = j["valueFormat"].get<std::string>();
    valueDecimals_ = j.value("valueDecimals", 2);
    valueMin_ = j.value("valueMin", 0.0f);
    valueMax_ = j.value("valueMax", 1.0f);
    valueSkewFactor_ = j.value("valueSkewFactor", 1.0f);
    valueSymmetricSkew_ = j.value("valueSymmetricSkew", false);
    
    // Auto-size
    if (j.contains("autoSize"))
    {
        const auto& as = j["autoSize"];
        autoSize_.enabled = as.value("enabled", true);
        autoSize_.minSize = as.value("minSize", 8.0f);
        autoSize_.maxSize = as.value("maxSize", 800.0f);
        autoSize_.widthMultiplier = as.value("widthMultiplier", 0.6f);
        autoSize_.heightMultiplier = as.value("heightMultiplier", 0.8f);
    }
}

} // namespace zaplab
