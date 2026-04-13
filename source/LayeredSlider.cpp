/*
  ==============================================================================

    LayeredSlider.cpp
    Created: 30 Nov 2025
    Author:  Paolo Zappalà @ zaplab.dev

  ==============================================================================
*/

#include "LayeredSlider.h"
#include "TextLayer.h"
#include "ShapeLayer.h"

namespace zaplab
{

//==============================================================================
// Construction

LayeredSlider::LayeredSlider()
    : LayeredSlider("LayeredSlider")
{
}

LayeredSlider::LayeredSlider(const juce::String& name)
    : juce::Component(name)
{
    setWantsKeyboardFocus(true);
    // Start empty so the designer opens with a blank canvas (no default preset)
}

LayeredSlider::~LayeredSlider() = default;

//==============================================================================
// Value Control

void LayeredSlider::setValue(double newValue, juce::NotificationType notification)
{
    newValue = constrainedValue(snapValue(newValue));
    
    if (! juce::approximatelyEqual(value_, newValue))
    {
        value_ = newValue;
        updateLayersForValue();
        repaint();
        sendValueChangedMessage(notification);
    }
}

void LayeredSlider::setRange(double minimum, double maximum, double interval)
{
    // Safely handle invalid ranges instead of asserting
    if (minimum >= maximum)
    {
        // If values are equal, create a small valid range
        if (juce::approximatelyEqual(minimum, maximum))
        {
            maximum = minimum + 1.0;
        }
        else
        {
            // Swap if min > max
            std::swap(minimum, maximum);
        }
    }
    
    minimum_ = minimum;
    maximum_ = maximum;
    interval_ = std::abs(interval);  // Ensure positive interval
    
    // Propagate range to any TextLayers with displaySliderValue enabled
    for (const auto& layer : layerStack_.getLayers())
    {
        if (auto* textLayer = dynamic_cast<TextLayer*>(layer.get()))
        {
            if (textLayer->isDisplayingSliderValue())
            {
                textLayer->setValueRange(static_cast<float>(minimum_), static_cast<float>(maximum_));
            }
        }
    }
    
    setValue(constrainedValue(value_), juce::dontSendNotification);
}

void LayeredSlider::setDoubleClickReturnValue(bool enabled, double valueToReturnTo)
{
    hasDoubleClickReturnValue_ = enabled;
    doubleClickReturnValue_ = valueToReturnTo;
}

void LayeredSlider::setSkewFactor(double factor, bool symmetricSkew)
{
    skewFactor_ = factor;
    symmetricSkew_ = symmetricSkew;
    
    // Propagate skew to any TextLayers with displaySliderValue enabled
    for (const auto& layer : layerStack_.getLayers())
    {
        if (auto* textLayer = dynamic_cast<TextLayer*>(layer.get()))
        {
            if (textLayer->isDisplayingSliderValue())
            {
                textLayer->setValueSkew(static_cast<float>(skewFactor_), symmetricSkew_);
            }
        }
    }
}

void LayeredSlider::setSkewFactorFromMidPoint(double sliderValueToShowAtMidPoint)
{
    if (sliderValueToShowAtMidPoint > minimum_ && sliderValueToShowAtMidPoint < maximum_)
    {
        skewFactor_ = std::log(0.5) / std::log((sliderValueToShowAtMidPoint - minimum_) / (maximum_ - minimum_));
    }
}

void LayeredSlider::setBipolarMode(bool enabled)
{
    bipolarMode_ = enabled;
    // Update visuals (bindings use their own bipolar flag)
    updateLayersForValue();
}

double LayeredSlider::proportionOfLengthToValue(double proportion) const
{
    if (skewFactor_ != 1.0)
    {
        if (symmetricSkew_)
        {
            const double distanceFromMiddle = 2.0 * proportion - 1.0;
            const double sign = distanceFromMiddle < 0 ? -1.0 : 1.0;
            proportion = 0.5 + sign * 0.5 * std::pow(std::abs(distanceFromMiddle), skewFactor_);
        }
        else
        {
            proportion = std::pow(proportion, skewFactor_);
        }
    }
    
    return minimum_ + (maximum_ - minimum_) * proportion;
}

double LayeredSlider::valueToProportionOfLength(double val) const
{
    double proportion = (val - minimum_) / (maximum_ - minimum_);
    
    if (skewFactor_ != 1.0)
    {
        if (symmetricSkew_)
        {
            const double distanceFromMiddle = 2.0 * proportion - 1.0;
            const double sign = distanceFromMiddle < 0 ? -1.0 : 1.0;
            proportion = 0.5 + sign * 0.5 * std::pow(std::abs(distanceFromMiddle), 1.0 / skewFactor_);
        }
        else
        {
            proportion = std::pow(proportion, 1.0 / skewFactor_);
        }
    }
    
    return juce::jlimit(0.0, 1.0, proportion);
}

//==============================================================================
// Style

void LayeredSlider::setSliderStyle(SliderStyle newStyle)
{
    if (style_ != newStyle)
    {
        style_ = newStyle;
        repaint();
    }
}

void LayeredSlider::setRotaryParameters(const RotaryParameters& params)
{
    rotaryParams_ = params;
    updateLayersForValue();
    repaint();
}

void LayeredSlider::setLinearParameters(const LinearParameters& params)
{
    linearParams_ = params;
    updateLayersForValue();
    repaint();
}

//==============================================================================
// Layer Access

ShapeLayer* LayeredSlider::getBackgroundLayer() const
{
    return dynamic_cast<ShapeLayer*>(layerStack_.getLayerByRole(SliderLayerRole::Overlay));
}

ShapeLayer* LayeredSlider::getTrackLayer() const
{
    return dynamic_cast<ShapeLayer*>(layerStack_.getLayerByRole(SliderLayerRole::Track));
}

ShapeLayer* LayeredSlider::getValueTrackLayer() const
{
    return dynamic_cast<ShapeLayer*>(layerStack_.getLayerByRole(SliderLayerRole::ValueTrack));
}

ShapeLayer* LayeredSlider::getThumbLayer() const
{
    return dynamic_cast<ShapeLayer*>(layerStack_.getLayerByRole(SliderLayerRole::Thumb));
}

TextLayer* LayeredSlider::getValueTextLayer() const
{
    if (auto* layer = layerStack_.getLayerByName("ValueText"))
        return dynamic_cast<TextLayer*>(layer);
    return nullptr;
}

//==============================================================================
// Text Value Display

void LayeredSlider::setTextValueSuffix(const juce::String& suffix)
{
    textSuffix_ = suffix;
    if (auto* textLayer = getValueTextLayer())
    {
        textLayer->setText(getTextFromValue(value_));
        repaint();
    }
}

void LayeredSlider::setNumDecimalPlacesToDisplay(int places)
{
    numDecimalPlaces_ = places;
    if (auto* textLayer = getValueTextLayer())
    {
        textLayer->setText(getTextFromValue(value_));
        repaint();
    }
}

void LayeredSlider::setShowValueText(bool show)
{
    showValueText_ = show;
    if (auto* textLayer = getValueTextLayer())
    {
        textLayer->setVisible(show);
        repaint();
    }
}

juce::String LayeredSlider::getTextFromValue(double val) const
{
    if (textFromValueFunction)
        return textFromValueFunction(val);
    
    juce::String text;
    
    if (interval_ != 0.0)
    {
        const int decimals = juce::jmax(0, -static_cast<int>(std::floor(std::log10(std::abs(interval_)))));
        text = juce::String(val, decimals);
    }
    else
    {
        text = juce::String(val, numDecimalPlaces_);
    }
    
    return text + textSuffix_;
}

double LayeredSlider::getValueFromText(const juce::String& text) const
{
    if (valueFromTextFunction)
        return valueFromTextFunction(text);
    
    juce::String workText = text.trimStart();
    
    // Remove suffix if present
    if (textSuffix_.isNotEmpty() && workText.endsWith(textSuffix_))
        workText = workText.dropLastCharacters(textSuffix_.length());
    
    return constrainedValue(workText.getDoubleValue());
}

//==============================================================================
// Presets & Templates

void LayeredSlider::loadPreset(const juce::String& presetName)
{
    if (presetName == "rotaryKnob")
        applyRotaryKnobPreset();
    else if (presetName == "minimalRotary")
        applyMinimalRotaryPreset();
    else if (presetName == "arcKnob")
        applyArcKnobPreset();
    else if (presetName == "linearFader")
        applyLinearFaderPreset();
    else if (presetName == "modernKnob")
        applyModernKnobPreset();
}

void LayeredSlider::loadFromValueTree(const juce::ValueTree& tree)
{
    if (! tree.isValid() || tree.getType() != juce::Identifier("LayeredSlider"))
        return;
    
    // Load value properties
    minimum_ = tree.getProperty("minimum", 0.0);
    maximum_ = tree.getProperty("maximum", 1.0);
    interval_ = tree.getProperty("interval", 0.0);
    value_ = tree.getProperty("value", 0.0);
    skewFactor_ = tree.getProperty("skewFactor", 1.0);
    
    // Load style
    style_ = static_cast<SliderStyle>(static_cast<int>(tree.getProperty("style", 0)));
    
    // Load text properties  
    textSuffix_ = tree.getProperty("textSuffix", "").toString();
    numDecimalPlaces_ = tree.getProperty("decimalPlaces", 2);
    showValueText_ = tree.getProperty("showValueText", true);
    
    // Load rotary parameters
    if (auto rotaryTree = tree.getChildWithName("RotaryParameters"); rotaryTree.isValid())
    {
        rotaryParams_.startAngleRadians = rotaryTree.getProperty("startAngle", juce::MathConstants<float>::pi * 1.25f);
        rotaryParams_.endAngleRadians = rotaryTree.getProperty("endAngle", juce::MathConstants<float>::pi * 2.75f);
        rotaryParams_.stopAtEnd = rotaryTree.getProperty("stopAtEnd", true);
        rotaryParams_.sensitivity = rotaryTree.getProperty("sensitivity", 1.0f);
    }
    
    // Load layer stack
    if (auto layerStackTree = tree.getChildWithName("LayerStack"); layerStackTree.isValid())
    {
        layerStack_.fromValueTree(layerStackTree);
    }
    
    updateLayersForValue();
    repaint();
}

juce::ValueTree LayeredSlider::saveToValueTree() const
{
    juce::ValueTree tree("LayeredSlider");
    
    // Save value properties
    tree.setProperty("minimum", minimum_, nullptr);
    tree.setProperty("maximum", maximum_, nullptr);
    tree.setProperty("interval", interval_, nullptr);
    tree.setProperty("value", value_, nullptr);
    tree.setProperty("skewFactor", skewFactor_, nullptr);
    
    // Save style
    tree.setProperty("style", static_cast<int>(style_), nullptr);
    
    // Save text properties
    tree.setProperty("textSuffix", textSuffix_, nullptr);
    tree.setProperty("decimalPlaces", numDecimalPlaces_, nullptr);
    tree.setProperty("showValueText", showValueText_, nullptr);
    
    // Save rotary parameters
    juce::ValueTree rotaryTree("RotaryParameters");
    rotaryTree.setProperty("startAngle", rotaryParams_.startAngleRadians, nullptr);
    rotaryTree.setProperty("endAngle", rotaryParams_.endAngleRadians, nullptr);
    rotaryTree.setProperty("stopAtEnd", rotaryParams_.stopAtEnd, nullptr);
    rotaryTree.setProperty("sensitivity", rotaryParams_.sensitivity, nullptr);
    tree.appendChild(rotaryTree, nullptr);
    
    // Save layer stack
    tree.appendChild(layerStack_.toValueTree(), nullptr);
    
    return tree;
}

json LayeredSlider::saveToJson() const
{
    json j;
    
    // Save value properties
    j["minimum"] = minimum_;
    j["maximum"] = maximum_;
    j["interval"] = interval_;
    j["value"] = value_;
    j["skewFactor"] = skewFactor_;
    j["symmetricSkew"] = symmetricSkew_;
    j["bipolarMode"] = bipolarMode_;
    
    // Save double-click return value settings
    j["hasDoubleClickReturnValue"] = hasDoubleClickReturnValue_;
    j["doubleClickReturnValue"] = doubleClickReturnValue_;
    
    // Save style
    j["style"] = static_cast<int>(style_);
    
    // Save text properties
    j["textSuffix"] = textSuffix_.toStdString();
    j["decimalPlaces"] = numDecimalPlaces_;
    j["showValueText"] = showValueText_;
    
    // Save rotary parameters
    j["rotaryParameters"] = {
        {"startAngle", rotaryParams_.startAngleRadians},
        {"endAngle", rotaryParams_.endAngleRadians},
        {"stopAtEnd", rotaryParams_.stopAtEnd},
        {"sensitivity", rotaryParams_.sensitivity},
        {"velocityMode", rotaryParams_.velocityMode},
        {"velocitySensitivity", rotaryParams_.velocitySensitivity},
        {"dragMode", static_cast<int>(rotaryParams_.dragMode)}
    };
    
    // Save linear parameters
    j["linearParameters"] = {
        {"horizontal", linearParams_.horizontal},
        {"thumbSize", linearParams_.thumbSize},
        {"showTrack", linearParams_.showTrack},
        {"showValueTrack", linearParams_.showValueTrack}
    };
    
    // Save layer stack
    j["layerStack"] = layerStack_.toJson();
    
    return j;
}

bool LayeredSlider::loadFromJson(const json& j)
{
    try
    {
        // Load value properties into member variables first
        double loadedMin = j.value("minimum", 0.0);
        double loadedMax = j.value("maximum", 1.0);
        double loadedInterval = j.value("interval", 0.0);
        double loadedValue = j.value("value", 0.5);
        double loadedSkew = j.value("skewFactor", 1.0);
        bool loadedSymmetricSkew = j.value("symmetricSkew", false);
        bipolarMode_ = j.value("bipolarMode", false);
        
        // Load double-click return value settings
        bool hasDoubleClick = j.value("hasDoubleClickReturnValue", false);
        double doubleClickValue = j.value("doubleClickReturnValue", loadedMin);
        
        // Load style
        style_ = static_cast<SliderStyle>(j.value("style", 0));
        
        // Load text properties
        if (j.contains("textSuffix"))
            textSuffix_ = j["textSuffix"].get<std::string>();
        numDecimalPlaces_ = j.value("decimalPlaces", 2);
        showValueText_ = j.value("showValueText", false);
        
        // Load rotary parameters
        if (j.contains("rotaryParameters"))
        {
            const auto& rotary = j["rotaryParameters"];
            rotaryParams_.startAngleRadians = rotary.value("startAngle", juce::MathConstants<float>::pi * 1.2f);
            rotaryParams_.endAngleRadians = rotary.value("endAngle", juce::MathConstants<float>::pi * 2.8f);
            rotaryParams_.stopAtEnd = rotary.value("stopAtEnd", true);
            rotaryParams_.sensitivity = rotary.value("sensitivity", 1.0);
            rotaryParams_.velocityMode = rotary.value("velocityMode", false);
            rotaryParams_.velocitySensitivity = rotary.value("velocitySensitivity", 0.1f);
            rotaryParams_.dragMode = static_cast<RotaryDragMode>(
                rotary.value("dragMode", static_cast<int>(RotaryDragMode::HorizontalVerticalDrag)));
        }
        
        // Check for behavior section's sliderStyle and map to RotaryDragMode
        // This handles .sliderdesign files that use juce::Slider::SliderStyle values
        if (j.contains("behavior"))
        {
            const auto& behavior = j["behavior"];
            if (behavior.contains("sliderStyle"))
            {
                int juceStyle = behavior.value("sliderStyle", 7);  // Default to RotaryHorizontalVerticalDrag
                // Map juce::Slider::SliderStyle values to RotaryDragMode
                // juce::Slider::Rotary = 4, RotaryHorizontalDrag = 5, RotaryVerticalDrag = 6, RotaryHorizontalVerticalDrag = 7
                switch (juceStyle)
                {
                    case 4:  // juce::Slider::Rotary
                        rotaryParams_.dragMode = RotaryDragMode::Circular;
                        break;
                    case 5:  // juce::Slider::RotaryHorizontalDrag
                        rotaryParams_.dragMode = RotaryDragMode::HorizontalDrag;
                        break;
                    case 6:  // juce::Slider::RotaryVerticalDrag
                        rotaryParams_.dragMode = RotaryDragMode::VerticalDrag;
                        break;
                    case 7:  // juce::Slider::RotaryHorizontalVerticalDrag
                    default:
                        rotaryParams_.dragMode = RotaryDragMode::HorizontalVerticalDrag;
                        break;
                }
            }
        }
        
        // Load linear parameters
        if (j.contains("linearParameters"))
        {
            const auto& linear = j["linearParameters"];
            linearParams_.horizontal = linear.value("horizontal", false);
            linearParams_.thumbSize = linear.value("thumbSize", 20.0f);
            linearParams_.showTrack = linear.value("showTrack", true);
            linearParams_.showValueTrack = linear.value("showValueTrack", true);
        }
        
        // Load layer stack BEFORE applying range (so TextLayers exist)
        if (j.contains("layerStack"))
        {
            layerStack_.fromJson(j["layerStack"]);
        }
        
        // Now apply settings using setter methods for proper initialization
        // This ensures TextLayers get the range, value is constrained, etc.
        setSkewFactor(loadedSkew, loadedSymmetricSkew);
        setRange(loadedMin, loadedMax, loadedInterval);  // This also calls setValue
        setValue(loadedValue, juce::dontSendNotification);
        setDoubleClickReturnValue(hasDoubleClick, doubleClickValue);
        
        updateLayersForValue();
        repaint();
        return true;
    }
    catch (const std::exception& e)
    {
        DBG("LayeredSlider::loadFromJson failed: " << e.what());
        return false;
    }
}

bool LayeredSlider::loadPresetFile(const juce::File& file)
{
    if (!file.existsAsFile())
    {
        DBG("LayeredSlider: Preset file not found: " << file.getFullPathName());
        return false;
    }
    
    auto fileText = file.loadFileAsString();
    if (fileText.isEmpty())
    {
        DBG("LayeredSlider: Failed to read preset file: " << file.getFullPathName());
        return false;
    }
    
    try
    {
        auto j = json::parse(fileText.toStdString());
        loadFromJson(j);
        return true;
    }
    catch (const std::exception& e)
    {
        DBG("LayeredSlider: Failed to parse preset JSON: " << e.what());
        return false;
    }
}

bool LayeredSlider::loadPresetFile(const juce::String& filePath)
{
    return loadPresetFile(juce::File(filePath));
}

bool LayeredSlider::loadFromMemory(const juce::String& presetName)
{
#if JUCE_MODULE_AVAILABLE_juce_binary_data || defined(BinaryData)
    // Convert preset name to valid BinaryData identifier
    // Replace dots and other special chars with underscores
    auto resourceName = presetName.replaceCharacters(".-/ ", "____");
    
    // Ensure .sliderdesign extension
    if (!resourceName.endsWithIgnoreCase("_sliderdesign"))
    {
        if (resourceName.contains("_sliderdesign_"))
            resourceName = resourceName.upToLastOccurrenceOf("_sliderdesign", true, false);
        else
            resourceName += "_sliderdesign";
    }
    
    // Try to find resource in BinaryData
    int dataSize = 0;
    const char* data = nullptr;
    
#ifdef BinaryData
    // Use BinaryData reflection to find the resource
    auto numResources = BinaryData::namedResourceListSize;
    for (int i = 0; i < numResources; ++i)
    {
        auto name = juce::String(BinaryData::namedResourceList[i]);
        if (name.equalsIgnoreCase(resourceName) || 
            name.containsIgnoreCase(resourceName))
        {
            data = BinaryData::getNamedResource(name.toRawUTF8(), dataSize);
            break;
        }
    }
#endif
    
    if (data == nullptr || dataSize == 0)
    {
        DBG("LayeredSlider: Preset not found in BinaryData: " << resourceName);
        return false;
    }
    
    try
    {
        auto jsonText = juce::String::fromUTF8(data, dataSize);
        auto j = json::parse(jsonText.toStdString());
        loadFromJson(j);
        return true;
    }
    catch (const std::exception& e)
    {
        DBG("LayeredSlider: Failed to parse preset from memory: " << e.what());
        return false;
    }
#else
    juce::ignoreUnused(presetName);
    DBG("LayeredSlider: loadFromMemory() requires BinaryData. Add preset files as binary resources.");
    return false;
#endif
}

bool LayeredSlider::loadFromBase64(const juce::String& base64EncodedJson)
{
    juce::MemoryOutputStream memOut;
    
    if (!juce::Base64::convertFromBase64(memOut, base64EncodedJson))
    {
        DBG("LayeredSlider: Failed to decode Base64 string");
        return false;
    }
    
    try
    {
        auto jsonText = juce::String::fromUTF8(
            static_cast<const char*>(memOut.getData()), 
            static_cast<int>(memOut.getDataSize())
        );
        auto j = json::parse(jsonText.toStdString());
        loadFromJson(j);
        return true;
    }
    catch (const std::exception& e)
    {
        DBG("LayeredSlider: Failed to parse JSON from Base64: " << e.what());
        return false;
    }
}

//==============================================================================
// Preset Implementations

void LayeredSlider::applyRotaryKnobPreset()
{
    setSliderStyle(SliderStyle::Rotary);
    layerStack_.clearLayers();
    
    // Background circle
    auto* bg = layerStack_.addShapeLayer("Background", ShapeType::Ellipse, SliderLayerRole::Overlay);
    bg->setFillColour(juce::Colour(0xFF2A2A2A));
    bg->setStrokeColour(juce::Colour(0xFF404040));
    bg->setStrokeWidth(0.02f);  // Relative (2%)
    bg->getTransform().setPosition(0.5f, 0.5f);
    bg->getTransform().setSize(0.8f, 0.8f);
    
    // Track arc
    auto* track = layerStack_.addShapeLayer("Track", ShapeType::Arc, SliderLayerRole::Track);
    track->setFillColour(juce::Colours::transparentBlack);
    track->setStrokeColour(juce::Colour(0xFF505050));
    track->setStrokeWidth(0.08f);  // Relative (8%)
    track->getTransform().setPosition(0.5f, 0.5f);
    track->getTransform().setSize(0.7f, 0.7f);
    track->getArcConfig().startAngle = rotaryParams_.startAngleRadians;
    track->getArcConfig().endAngle = rotaryParams_.endAngleRadians;
    track->getArcConfig().thickness = 0.15f;
    
    // Value arc
    auto* valueTrack = layerStack_.addShapeLayer("ValueTrack", ShapeType::Arc, SliderLayerRole::ValueTrack);
    valueTrack->setFillColour(juce::Colours::transparentBlack);
    valueTrack->setStrokeColour(juce::Colour(0xFF00AAFF));
    valueTrack->setStrokeWidth(0.08f);  // Relative (8%)
    valueTrack->getTransform().setPosition(0.5f, 0.5f);
    valueTrack->getTransform().setSize(0.7f, 0.7f);
    valueTrack->getArcConfig().startAngle = rotaryParams_.startAngleRadians;
    valueTrack->getArcConfig().thickness = 0.15f;
    // Enable value binding for arc
    auto& binding = valueTrack->getValueBinding();
    binding.enabled = true;
    binding.property = ValueBindingProperty::ArcEnd;
    binding.valueAtMin = rotaryParams_.startAngleRadians;
    binding.valueAtMax = rotaryParams_.endAngleRadians;
    
    // Thumb indicator
    auto* thumb = layerStack_.addShapeLayer("Thumb", ShapeType::Ellipse, SliderLayerRole::Thumb);
    thumb->setFillColour(juce::Colours::white);
    thumb->getTransform().setSize(0.12f, 0.12f);
    
    // Value text
    auto* valueText = layerStack_.addTextLayer("ValueText", "0.00");
    valueText->setTextColour(juce::Colours::white);
    valueText->setFontSize(14.0f);
    valueText->setAlignment(TextAlignment{});
    valueText->getTransform().setPosition(0.5f, 0.5f);
    valueText->getTransform().setSize(1.0f, 0.2f);
    
    updateLayersForValue();
}

void LayeredSlider::applyMinimalRotaryPreset()
{
    setSliderStyle(SliderStyle::Rotary);
    layerStack_.clearLayers();
    
    // Just an arc track
    auto* track = layerStack_.addShapeLayer("Track", ShapeType::Arc, SliderLayerRole::Track);
    track->setFillColour(juce::Colours::transparentBlack);
    track->setStrokeColour(juce::Colour(0xFF303030));
    track->setStrokeWidth(0.04f);  // Relative (4%)
    track->getTransform().setPosition(0.5f, 0.5f);
    track->getTransform().setSize(0.8f, 0.8f);
    track->getArcConfig().startAngle = rotaryParams_.startAngleRadians;
    track->getArcConfig().endAngle = rotaryParams_.endAngleRadians;
    track->getArcConfig().thickness = 0.08f;
    
    // Value arc
    auto* valueTrack = layerStack_.addShapeLayer("ValueTrack", ShapeType::Arc, SliderLayerRole::ValueTrack);
    valueTrack->setFillColour(juce::Colours::transparentBlack);
    valueTrack->setStrokeColour(juce::Colour(0xFFFFFFFF));
    valueTrack->setStrokeWidth(0.04f);  // Relative (4%)
    valueTrack->getTransform().setPosition(0.5f, 0.5f);
    valueTrack->getTransform().setSize(0.8f, 0.8f);
    valueTrack->getArcConfig().startAngle = rotaryParams_.startAngleRadians;
    valueTrack->getArcConfig().thickness = 0.08f;
    // Enable value binding for arc
    auto& binding = valueTrack->getValueBinding();
    binding.enabled = true;
    binding.property = ValueBindingProperty::ArcEnd;
    binding.valueAtMin = rotaryParams_.startAngleRadians;
    binding.valueAtMax = rotaryParams_.endAngleRadians;
    
    updateLayersForValue();
}

void LayeredSlider::applyArcKnobPreset()
{
    setSliderStyle(SliderStyle::Rotary);
    layerStack_.clearLayers();
    
    // Background with gradient
    auto* bg = layerStack_.addShapeLayer("Background", ShapeType::Ellipse, SliderLayerRole::Overlay);
    bg->setFillColour(juce::Colour(0xFF1A1A1A));
    bg->getTransform().setPosition(0.5f, 0.5f);
    bg->getTransform().setSize(0.9f, 0.9f);
    
    auto& bgGradient = bg->getStyle().fillGradient;
    bgGradient.enabled = true;
    bgGradient.isRadial = true;
    bgGradient.colour1 = juce::Colour(0xFF3A3A3A);
    bgGradient.colour2 = juce::Colour(0xFF1A1A1A);
    
    // Thick arc track
    auto* track = layerStack_.addShapeLayer("Track", ShapeType::Arc, SliderLayerRole::Track);
    track->setFillColour(juce::Colour(0xFF252525));
    track->setStrokeColour(juce::Colours::transparentBlack);
    track->getTransform().setPosition(0.5f, 0.5f);
    track->getTransform().setSize(0.76f, 0.76f);
    track->getArcConfig().startAngle = rotaryParams_.startAngleRadians;
    track->getArcConfig().endAngle = rotaryParams_.endAngleRadians;
    track->getArcConfig().thickness = 0.2f;
    
    // Colored value arc
    auto* valueTrack = layerStack_.addShapeLayer("ValueTrack", ShapeType::Arc, SliderLayerRole::ValueTrack);
    valueTrack->setFillColour(juce::Colour(0xFF00CC88));
    valueTrack->setStrokeColour(juce::Colours::transparentBlack);
    valueTrack->getTransform().setPosition(0.5f, 0.5f);
    valueTrack->getTransform().setSize(0.76f, 0.76f);
    valueTrack->getArcConfig().startAngle = rotaryParams_.startAngleRadians;
    valueTrack->getArcConfig().thickness = 0.2f;
    // Enable value binding for arc
    auto& arcBinding = valueTrack->getValueBinding();
    arcBinding.enabled = true;
    arcBinding.property = ValueBindingProperty::ArcEnd;
    arcBinding.valueAtMin = rotaryParams_.startAngleRadians;
    arcBinding.valueAtMax = rotaryParams_.endAngleRadians;
    
    // Center cap
    auto* cap = layerStack_.addShapeLayer("CenterCap", ShapeType::Ellipse);
    cap->setFillColour(juce::Colour(0xFF2A2A2A));
    cap->getTransform().setPosition(0.5f, 0.5f);
    cap->getTransform().setSize(0.4f, 0.4f);
    
    // Value text  
    auto* valueText = layerStack_.addTextLayer("ValueText", "0.00");
    valueText->setTextColour(juce::Colours::white.withAlpha(0.9f));
    valueText->setFontSize(16.0f);
    valueText->setFontStyle("Bold");
    valueText->setAlignment(TextAlignment{});
    valueText->getTransform().setPosition(0.5f, 0.5f);
    valueText->getTransform().setSize(1.0f, 0.2f);
    
    updateLayersForValue();
}

void LayeredSlider::applyLinearFaderPreset()
{
    setSliderStyle(SliderStyle::LinearVertical);
    linearParams_.horizontal = false;
    layerStack_.clearLayers();
    
    // Track background - centered anchor, centered position
    auto* track = layerStack_.addShapeLayer("Track", ShapeType::Rectangle, SliderLayerRole::Track);
    track->setFillColour(juce::Colour(0xFF252525));
    track->setCornerRadius(0.04f);  // Relative (4%)
    track->getTransform().setPosition(0.5f, 0.5f);
    track->getTransform().setSize(0.3f, 0.9f);
    
    // Value track - anchor at bottom-center so it grows upward
    auto* valueTrack = layerStack_.addShapeLayer("ValueTrack", ShapeType::Rectangle, SliderLayerRole::ValueTrack);
    valueTrack->setFillColour(juce::Colour(0xFF00AAFF));
    valueTrack->setCornerRadius(0.04f);  // Relative (4%)
    valueTrack->getTransform().anchor = { 0.5f, 1.0f };  // Bottom-center anchor
    valueTrack->getTransform().setPosition(0.5f, 0.95f); // Position at bottom of track area
    valueTrack->getTransform().setSize(0.3f, 0.0f);      // Start with zero height
    // Enable value binding for height
    auto& binding = valueTrack->getValueBinding();
    binding.enabled = true;
    binding.property = ValueBindingProperty::Height;
    binding.valueAtMin = 0.0f;
    binding.valueAtMax = 0.9f;  // Same as track height
    
    // Thumb/fader cap
    auto* thumb = layerStack_.addShapeLayer("Thumb", ShapeType::Rectangle, SliderLayerRole::Thumb);
    thumb->setFillColour(juce::Colour(0xFFDDDDDD));
    thumb->setCornerRadius(0.03f);  // Relative (3%)
    thumb->getTransform().setSize(0.6f, 0.1f);
    thumb->getTransform().setPosition(0.5f, 0.95f);  // Start at bottom
    // Enable value binding for thumb position (moves up as value increases)
    auto& thumbBinding = thumb->getValueBinding();
    thumbBinding.enabled = true;
    thumbBinding.property = ValueBindingProperty::PositionY;
    thumbBinding.valueAtMin = 0.95f;  // Bottom position
    thumbBinding.valueAtMax = 0.05f;  // Top position
    
    // Add shadow to thumb
    thumb->getStyle().shadow.enabled = true;
    thumb->getStyle().shadow.colour = juce::Colours::black.withAlpha(0.4f);
    thumb->getStyle().shadow.radius = 0.06f;  // Relative (6%)
    thumb->getStyle().shadow.offset = { 0.0f, 0.02f };  // Relative
    
    updateLayersForValue();
}

void LayeredSlider::applyModernKnobPreset()
{
    setSliderStyle(SliderStyle::Rotary);
    layerStack_.clearLayers();
    
    // Outer ring with shadow
    auto* outer = layerStack_.addShapeLayer("OuterRing", ShapeType::Ellipse);
    outer->setFillColour(juce::Colour(0xFF1E1E1E));
    outer->setStrokeColour(juce::Colour(0xFF333333));
    outer->setStrokeWidth(0.01f);  // Relative (1%)
    outer->getTransform().setPosition(0.5f, 0.5f);
    outer->getTransform().setSize(0.96f, 0.96f);
    outer->getStyle().shadow.enabled = true;
    outer->getStyle().shadow.colour = juce::Colours::black.withAlpha(0.5f);
    outer->getStyle().shadow.radius = 0.12f;  // Relative (12%)
    outer->getStyle().shadow.offset = { 0.0f, 0.04f };  // Relative
    
    // Inner knob with gradient
    auto* knob = layerStack_.addShapeLayer("Knob", ShapeType::Ellipse);
    knob->setFillColour(juce::Colour(0xFF2D2D2D));
    knob->getTransform().setPosition(0.5f, 0.5f);
    knob->getTransform().setSize(0.8f, 0.8f);
    
    auto& knobGrad = knob->getStyle().fillGradient;
    knobGrad.enabled = true;
    knobGrad.isRadial = true;
    knobGrad.colour1 = juce::Colour(0xFF404040);
    knobGrad.colour2 = juce::Colour(0xFF252525);
    
    // Value arc
    auto* valueTrack = layerStack_.addShapeLayer("ValueTrack", ShapeType::Arc, SliderLayerRole::ValueTrack);
    valueTrack->setFillColour(juce::Colours::transparentBlack);
    valueTrack->setStrokeColour(juce::Colour(0xFF00CCFF));
    valueTrack->setStrokeWidth(0.03f);  // Relative (3%)
    valueTrack->getTransform().setPosition(0.5f, 0.5f);
    valueTrack->getTransform().setSize(0.88f, 0.88f);
    valueTrack->getArcConfig().startAngle = rotaryParams_.startAngleRadians;
    valueTrack->getArcConfig().thickness = 0.05f;
    // Enable value binding for arc
    auto& arcBinding = valueTrack->getValueBinding();
    arcBinding.enabled = true;
    arcBinding.property = ValueBindingProperty::ArcEnd;
    arcBinding.valueAtMin = rotaryParams_.startAngleRadians;
    arcBinding.valueAtMax = rotaryParams_.endAngleRadians;
    
    // Pointer line
    auto* pointer = layerStack_.addShapeLayer("Pointer", ShapeType::Rectangle, SliderLayerRole::Thumb);
    pointer->setFillColour(juce::Colours::white);
    pointer->getTransform().setSize(0.03f, 0.25f);
    
    // Value text
    auto* valueText = layerStack_.addTextLayer("ValueText", "0.00");
    valueText->setTextColour(juce::Colours::white.withAlpha(0.7f));
    valueText->setFontSize(12.0f);
    valueText->setAlignment(TextAlignment{});
    valueText->getTransform().setPosition(0.5f, 0.5f);
    valueText->getTransform().setSize(1.0f, 0.16f);
    
    updateLayersForValue();
}

void LayeredSlider::applyNeonGlowKnobPreset()
{
    setSliderStyle(SliderStyle::Rotary);
    layerStack_.clearLayers();
    
    // Dark background
    auto* bg = layerStack_.addShapeLayer("Background", ShapeType::Ellipse);
    bg->setFillColour(juce::Colour(0xFF0A0A0A));
    bg->getTransform().setPosition(0.5f, 0.5f);
    bg->getTransform().setSize(0.95f, 0.95f);
    
    // Outer glow ring
    auto* glowRing = layerStack_.addShapeLayer("GlowRing", ShapeType::Arc);
    glowRing->setFillColour(juce::Colours::transparentBlack);
    glowRing->setStrokeColour(juce::Colour(0xFF00FF88));
    glowRing->setStrokeWidth(0.02f);  // Relative
    glowRing->getTransform().setPosition(0.5f, 0.5f);
    glowRing->getTransform().setSize(0.85f, 0.85f);
    glowRing->getArcConfig().startAngle = rotaryParams_.startAngleRadians;
    glowRing->getArcConfig().endAngle = rotaryParams_.endAngleRadians;
    glowRing->getArcConfig().thickness = 0.08f;
    
    // Enable outer glow effect
    glowRing->getEffects().outerGlow.enabled = true;
    glowRing->getEffects().outerGlow.colour = juce::Colour(0xFF00FF88);
    glowRing->getEffects().outerGlow.blurRadius = 15;
    glowRing->getEffects().outerGlow.spread = 0.5f;
    
    // Value arc with neon effect
    auto* valueTrack = layerStack_.addShapeLayer("ValueTrack", ShapeType::Arc, SliderLayerRole::ValueTrack);
    valueTrack->setFillColour(juce::Colours::transparentBlack);
    valueTrack->setStrokeColour(juce::Colour(0xFFFF00AA));
    valueTrack->setStrokeWidth(0.03f);  // Relative
    valueTrack->getTransform().setPosition(0.5f, 0.5f);
    valueTrack->getTransform().setSize(0.85f, 0.85f);
    valueTrack->getArcConfig().startAngle = rotaryParams_.startAngleRadians;
    valueTrack->getArcConfig().thickness = 0.08f;
    // Enable value binding for arc
    auto& arcBinding = valueTrack->getValueBinding();
    arcBinding.enabled = true;
    arcBinding.property = ValueBindingProperty::ArcEnd;
    arcBinding.valueAtMin = rotaryParams_.startAngleRadians;
    arcBinding.valueAtMax = rotaryParams_.endAngleRadians;
    
    valueTrack->getEffects().outerGlow.enabled = true;
    valueTrack->getEffects().outerGlow.colour = juce::Colour(0xFFFF00AA);
    valueTrack->getEffects().outerGlow.blurRadius = 12;
    
    // Center dot
    auto* center = layerStack_.addShapeLayer("Center", ShapeType::Ellipse);
    center->setFillColour(juce::Colour(0xFF151515));
    center->getTransform().setPosition(0.5f, 0.5f);
    center->getTransform().setSize(0.5f, 0.5f);
    
    updateLayersForValue();
}

void LayeredSlider::applyVintageKnobPreset()
{
    setSliderStyle(SliderStyle::Rotary);
    layerStack_.clearLayers();
    
    // Outer bezel with bevel
    auto* bezel = layerStack_.addShapeLayer("Bezel", ShapeType::Ellipse);
    bezel->setFillColour(juce::Colour(0xFF8B7355));  // Warm brown
    bezel->setStrokeColour(juce::Colour(0xFF5C4A3A));
    bezel->setStrokeWidth(0.015f);  // Relative
    bezel->getTransform().setPosition(0.5f, 0.5f);
    bezel->getTransform().setSize(0.95f, 0.95f);
    
    bezel->getEffects().bevelEmboss.enabled = true;
    bezel->getEffects().bevelEmboss.depth = 4;
    bezel->getEffects().bevelEmboss.lightAngle = 135.0f * juce::MathConstants<float>::pi / 180.0f;
    bezel->getEffects().bevelEmboss.softness = 3;
    
    // Inner knob with cream color
    auto* knob = layerStack_.addShapeLayer("Knob", ShapeType::Ellipse);
    knob->setFillColour(juce::Colour(0xFFF5E6D3));  // Cream
    knob->getTransform().setPosition(0.5f, 0.5f);
    knob->getTransform().setSize(0.75f, 0.75f);
    
    auto& knobGrad = knob->getStyle().fillGradient;
    knobGrad.enabled = true;
    knobGrad.isRadial = true;
    knobGrad.colour1 = juce::Colour(0xFFFFF8EE);
    knobGrad.colour2 = juce::Colour(0xFFE8D4C0);
    
    // Pointer notch
    auto* pointer = layerStack_.addShapeLayer("Pointer", ShapeType::Rectangle, SliderLayerRole::Thumb);
    pointer->setFillColour(juce::Colour(0xFF2A2A2A));
    pointer->getTransform().setSize(0.04f, 0.2f);
    pointer->setCornerRadius(0.01f);
    
    // Scale markings would be drawn as separate layers...
    
    updateLayersForValue();
}

void LayeredSlider::applyGlassyKnobPreset()
{
    setSliderStyle(SliderStyle::Rotary);
    layerStack_.clearLayers();
    
    // Shadow layer
    auto* shadow = layerStack_.addShapeLayer("Shadow", ShapeType::Ellipse);
    shadow->setFillColour(juce::Colours::black.withAlpha(0.3f));
    shadow->getTransform().setPosition(0.52f, 0.54f);
    shadow->getTransform().setSize(0.9f, 0.9f);
    
    // Base layer
    auto* base = layerStack_.addShapeLayer("Base", ShapeType::Ellipse);
    base->setFillColour(juce::Colour(0xFF1A2A3A));
    base->getTransform().setPosition(0.5f, 0.5f);
    base->getTransform().setSize(0.9f, 0.9f);
    
    // Glass reflection gradient
    auto* glass = layerStack_.addShapeLayer("Glass", ShapeType::Ellipse);
    glass->getTransform().setPosition(0.5f, 0.5f);
    glass->getTransform().setSize(0.85f, 0.85f);
    
    auto& glassGrad = glass->getStyle().fillGradient;
    glassGrad.enabled = true;
    glassGrad.isRadial = false;
    glassGrad.colour1 = juce::Colour(0x80FFFFFF);
    glassGrad.colour2 = juce::Colour(0x10FFFFFF);
    glassGrad.start = { 0.5f, 0.0f };
    glassGrad.end = { 0.5f, 0.7f };
    
    // Value arc
    auto* valueTrack = layerStack_.addShapeLayer("ValueTrack", ShapeType::Arc, SliderLayerRole::ValueTrack);
    valueTrack->setFillColour(juce::Colours::transparentBlack);
    valueTrack->setStrokeColour(juce::Colour(0xFF4FC3F7));
    valueTrack->setStrokeWidth(0.025f);  // Relative
    valueTrack->getTransform().setPosition(0.5f, 0.5f);
    valueTrack->getTransform().setSize(0.75f, 0.75f);
    valueTrack->getArcConfig().startAngle = rotaryParams_.startAngleRadians;
    valueTrack->getArcConfig().thickness = 0.06f;
    // Enable value binding for arc
    auto& glassyBinding = valueTrack->getValueBinding();
    glassyBinding.enabled = true;
    glassyBinding.property = ValueBindingProperty::ArcEnd;
    glassyBinding.valueAtMin = rotaryParams_.startAngleRadians;
    glassyBinding.valueAtMax = rotaryParams_.endAngleRadians;
    
    // Center highlight
    auto* highlight = layerStack_.addShapeLayer("Highlight", ShapeType::Ellipse);
    highlight->setFillColour(juce::Colour(0x40FFFFFF));
    highlight->getTransform().setPosition(0.5f, 0.42f);
    highlight->getTransform().setSize(0.4f, 0.25f);
    
    updateLayersForValue();
}

void LayeredSlider::applyMetallicKnobPreset()
{
    setSliderStyle(SliderStyle::Rotary);
    layerStack_.clearLayers();
    
    // Outer rim
    auto* rim = layerStack_.addShapeLayer("Rim", ShapeType::Ellipse);
    rim->setFillColour(juce::Colour(0xFF606060));
    rim->setStrokeColour(juce::Colour(0xFF404040));
    rim->setStrokeWidth(0.01f);  // Relative
    rim->getTransform().setPosition(0.5f, 0.5f);
    rim->getTransform().setSize(0.95f, 0.95f);
    
    rim->getEffects().bevelEmboss.enabled = true;
    rim->getEffects().bevelEmboss.depth = 3;
    rim->getEffects().bevelEmboss.lightAngle = 120.0f * juce::MathConstants<float>::pi / 180.0f;
    rim->getEffects().bevelEmboss.highlightColour = juce::Colour(0xFFAAAAAA);
    rim->getEffects().bevelEmboss.shadowColour = juce::Colour(0xFF303030);
    
    // Brushed metal knob with conic gradient
    auto* knob = layerStack_.addShapeLayer("Knob", ShapeType::Ellipse);
    knob->getTransform().setPosition(0.5f, 0.5f);
    knob->getTransform().setSize(0.85f, 0.85f);
    
    // Conic gradient for brushed metal look
    auto& metalGrad = knob->getStyle().fillGradient;
    metalGrad.enabled = true;
    metalGrad.type = GradientType::Conic;
    metalGrad.colour1 = juce::Colour(0xFFB0B0B0);
    metalGrad.colour2 = juce::Colour(0xFF707070);
    knob->getStyle().fillMode = FillMode::ConicGradient;
    
    // Indicator line
    auto* indicator = layerStack_.addShapeLayer("Indicator", ShapeType::Rectangle, SliderLayerRole::Thumb);
    indicator->setFillColour(juce::Colour(0xFFFF4444));
    indicator->getTransform().setSize(0.025f, 0.3f);
    
    // Center cap
    auto* cap = layerStack_.addShapeLayer("Cap", ShapeType::Ellipse);
    cap->setFillColour(juce::Colour(0xFF505050));
    cap->getTransform().setPosition(0.5f, 0.5f);
    cap->getTransform().setSize(0.2f, 0.2f);
    
    cap->getEffects().bevelEmboss.enabled = true;
    cap->getEffects().bevelEmboss.depth = 2;
    
    updateLayersForValue();
}

void LayeredSlider::applyFlatMaterialPreset()
{
    setSliderStyle(SliderStyle::Rotary);
    layerStack_.clearLayers();
    
    // Background circle
    auto* bg = layerStack_.addShapeLayer("Background", ShapeType::Ellipse);
    bg->setFillColour(juce::Colour(0xFF3F51B5));  // Material indigo
    bg->getTransform().setPosition(0.5f, 0.5f);
    bg->getTransform().setSize(0.9f, 0.9f);
    
    // Simple drop shadow
    bg->getStyle().shadow.enabled = true;
    bg->getStyle().shadow.colour = juce::Colours::black.withAlpha(0.25f);
    bg->getStyle().shadow.radius = 0.03f;  // Relative
    bg->getStyle().shadow.offset = { 0.0f, 0.02f };
    
    // Track arc (subtle)
    auto* track = layerStack_.addShapeLayer("Track", ShapeType::Arc, SliderLayerRole::Track);
    track->setFillColour(juce::Colours::transparentBlack);
    track->setStrokeColour(juce::Colour(0x40FFFFFF));
    track->setStrokeWidth(0.03f);  // Relative
    track->getTransform().setPosition(0.5f, 0.5f);
    track->getTransform().setSize(0.7f, 0.7f);
    track->getArcConfig().startAngle = rotaryParams_.startAngleRadians;
    track->getArcConfig().endAngle = rotaryParams_.endAngleRadians;
    track->getArcConfig().thickness = 0.1f;
    
    // Value arc
    auto* valueTrack = layerStack_.addShapeLayer("ValueTrack", ShapeType::Arc, SliderLayerRole::ValueTrack);
    valueTrack->setFillColour(juce::Colours::transparentBlack);
    valueTrack->setStrokeColour(juce::Colours::white);
    valueTrack->setStrokeWidth(0.03f);  // Relative
    valueTrack->getTransform().setPosition(0.5f, 0.5f);
    valueTrack->getTransform().setSize(0.7f, 0.7f);
    valueTrack->getArcConfig().startAngle = rotaryParams_.startAngleRadians;
    valueTrack->getArcConfig().thickness = 0.1f;
    // Enable value binding for arc
    auto& flatBinding = valueTrack->getValueBinding();
    flatBinding.enabled = true;
    flatBinding.property = ValueBindingProperty::ArcEnd;
    flatBinding.valueAtMin = rotaryParams_.startAngleRadians;
    flatBinding.valueAtMax = rotaryParams_.endAngleRadians;
    
    // Center
    auto* center = layerStack_.addShapeLayer("Center", ShapeType::Ellipse);
    center->setFillColour(juce::Colour(0xFF303F9F));  // Darker indigo
    center->getTransform().setPosition(0.5f, 0.5f);
    center->getTransform().setSize(0.35f, 0.35f);
    
    updateLayersForValue();
}

void LayeredSlider::applyGradientSliderPreset()
{
    setSliderStyle(SliderStyle::LinearHorizontal);
    layerStack_.clearLayers();
    
    // Track background
    auto* track = layerStack_.addShapeLayer("Track", ShapeType::Rectangle, SliderLayerRole::Track);
    track->setFillColour(juce::Colour(0xFF2A2A2A));
    track->setCornerRadius(0.1f);  // Relative
    track->getTransform().setPosition(0.5f, 0.5f);
    track->getTransform().setSize(0.9f, 0.15f);
    
    // Value fill with gradient
    auto* valueFill = layerStack_.addShapeLayer("ValueFill", ShapeType::Rectangle, SliderLayerRole::ValueTrack);
    valueFill->setCornerRadius(0.1f);  // Relative
    valueFill->getTransform().setPosition(0.5f, 0.5f);
    valueFill->getTransform().setSize(0.9f, 0.15f);
    
    auto& fillGrad = valueFill->getStyle().fillGradient;
    fillGrad.enabled = true;
    fillGrad.colour1 = juce::Colour(0xFF00D4AA);
    fillGrad.colour2 = juce::Colour(0xFF0099FF);
    fillGrad.start = { 0.0f, 0.5f };
    fillGrad.end = { 1.0f, 0.5f };
    valueFill->getStyle().fillMode = FillMode::LinearGradient;
    
    // Thumb
    auto* thumb = layerStack_.addShapeLayer("Thumb", ShapeType::Ellipse, SliderLayerRole::Thumb);
    thumb->setFillColour(juce::Colours::white);
    thumb->getTransform().setSize(0.12f, 0.25f);
    
    thumb->getStyle().shadow.enabled = true;
    thumb->getStyle().shadow.radius = 0.02f;
    thumb->getStyle().shadow.offset = { 0.0f, 0.01f };
    
    updateLayersForValue();
}

void LayeredSlider::applySkeumorphicKnobPreset()
{
    setSliderStyle(SliderStyle::Rotary);
    layerStack_.clearLayers();
    
    // Deep shadow
    auto* shadow = layerStack_.addShapeLayer("DeepShadow", ShapeType::Ellipse);
    shadow->setFillColour(juce::Colours::black.withAlpha(0.4f));
    shadow->getTransform().setPosition(0.52f, 0.55f);
    shadow->getTransform().setSize(0.88f, 0.88f);
    
    // Outer ring
    auto* outer = layerStack_.addShapeLayer("OuterRing", ShapeType::Ellipse);
    outer->setFillColour(juce::Colour(0xFF1A1A1A));
    outer->setStrokeColour(juce::Colour(0xFF0A0A0A));
    outer->setStrokeWidth(0.02f);  // Relative
    outer->getTransform().setPosition(0.5f, 0.5f);
    outer->getTransform().setSize(0.92f, 0.92f);
    
    outer->getEffects().bevelEmboss.enabled = true;
    outer->getEffects().bevelEmboss.depth = 5;
    outer->getEffects().bevelEmboss.lightAngle = 135.0f * juce::MathConstants<float>::pi / 180.0f;
    
    // Knob body
    auto* body = layerStack_.addShapeLayer("Body", ShapeType::Ellipse);
    body->getTransform().setPosition(0.5f, 0.5f);
    body->getTransform().setSize(0.8f, 0.8f);
    
    auto& bodyGrad = body->getStyle().fillGradient;
    bodyGrad.enabled = true;
    bodyGrad.isRadial = true;
    bodyGrad.colour1 = juce::Colour(0xFF505050);
    bodyGrad.colour2 = juce::Colour(0xFF252525);
    
    // Value arc
    auto* valueTrack = layerStack_.addShapeLayer("ValueTrack", ShapeType::Arc, SliderLayerRole::ValueTrack);
    valueTrack->setFillColour(juce::Colours::transparentBlack);
    valueTrack->setStrokeColour(juce::Colour(0xFF44AAFF));
    valueTrack->setStrokeWidth(0.025f);  // Relative
    valueTrack->getTransform().setPosition(0.5f, 0.5f);
    valueTrack->getTransform().setSize(0.86f, 0.86f);
    valueTrack->getArcConfig().startAngle = rotaryParams_.startAngleRadians;
    valueTrack->getArcConfig().thickness = 0.05f;
    // Enable value binding for arc
    auto& skeuBinding = valueTrack->getValueBinding();
    skeuBinding.enabled = true;
    skeuBinding.property = ValueBindingProperty::ArcEnd;
    skeuBinding.valueAtMin = rotaryParams_.startAngleRadians;
    skeuBinding.valueAtMax = rotaryParams_.endAngleRadians;
    
    // Metal pointer
    auto* pointer = layerStack_.addShapeLayer("Pointer", ShapeType::Rectangle, SliderLayerRole::Thumb);
    pointer->setFillColour(juce::Colour(0xFFCCCCCC));
    pointer->getTransform().setSize(0.04f, 0.25f);
    
    pointer->getEffects().bevelEmboss.enabled = true;
    pointer->getEffects().bevelEmboss.depth = 2;
    
    // Center screw
    auto* screw = layerStack_.addShapeLayer("Screw", ShapeType::Ellipse);
    screw->setFillColour(juce::Colour(0xFF3A3A3A));
    screw->getTransform().setPosition(0.5f, 0.5f);
    screw->getTransform().setSize(0.15f, 0.15f);
    
    screw->getEffects().bevelEmboss.enabled = true;
    screw->getEffects().bevelEmboss.depth = 2;
    screw->getEffects().bevelEmboss.lightAngle = 315.0f * juce::MathConstants<float>::pi / 180.0f;
    
    updateLayersForValue();
}

void LayeredSlider::applyMinimalLineKnobPreset()
{
    setSliderStyle(SliderStyle::Rotary);
    layerStack_.clearLayers();
    
    // Single arc line
    auto* arc = layerStack_.addShapeLayer("Arc", ShapeType::Arc, SliderLayerRole::ValueTrack);
    arc->setFillColour(juce::Colours::transparentBlack);
    arc->setStrokeColour(juce::Colours::white);
    arc->setStrokeWidth(0.015f);  // Relative
    arc->getTransform().setPosition(0.5f, 0.5f);
    arc->getTransform().setSize(0.85f, 0.85f);
    arc->getArcConfig().startAngle = rotaryParams_.startAngleRadians;
    arc->getArcConfig().thickness = 0.02f;
    // Enable value binding for arc
    auto& minimalBinding = arc->getValueBinding();
    minimalBinding.enabled = true;
    minimalBinding.property = ValueBindingProperty::ArcEnd;
    minimalBinding.valueAtMin = rotaryParams_.startAngleRadians;
    minimalBinding.valueAtMax = rotaryParams_.endAngleRadians;
    
    // Center dot
    auto* dot = layerStack_.addShapeLayer("Dot", ShapeType::Ellipse);
    dot->setFillColour(juce::Colours::white);
    dot->getTransform().setPosition(0.5f, 0.5f);
    dot->getTransform().setSize(0.08f, 0.08f);
    
    // Rotating line indicator
    auto* line = layerStack_.addShapeLayer("Line", ShapeType::Rectangle, SliderLayerRole::Thumb);
    line->setFillColour(juce::Colours::white);
    line->getTransform().setSize(0.015f, 0.35f);
    
    updateLayersForValue();
}

//==============================================================================
// Export

juce::Image LayeredSlider::renderToImage(int width, int height) const
{
    juce::Image image(juce::Image::ARGB, width, height, true);
    juce::Graphics g(image);
    
    // Save current bounds and render at target size
    LayerStack tempStack;
    // Clone would be needed for perfect export - for now just render current
    layerStack_.paintLayers(g, { 0, 0, static_cast<float>(width), static_cast<float>(height) });
    
    return image;
}

juce::Image LayeredSlider::renderFilmstrip(int frameWidth, int frameHeight,
                                            int numFrames, bool vertical) const
{
    return layerStack_.renderFilmstrip(frameWidth, frameHeight, numFrames, vertical);
}

//==============================================================================
// Listeners

void LayeredSlider::addListener(Listener* listener)
{
    listeners_.add(listener);
}

void LayeredSlider::removeListener(Listener* listener)
{
    listeners_.remove(listener);
}

//==============================================================================
// Component Overrides

void LayeredSlider::paint(juce::Graphics& g)
{
    layerStack_.paintLayers(g, getLocalBounds().toFloat());
}

void LayeredSlider::resized()
{
    layerStack_.setBounds(getLocalBounds());
    updateLayersForValue();
}

void LayeredSlider::mouseDown(const juce::MouseEvent& e)
{
    if (! isEnabled())
        return;
    
    isDragging_ = true;
    valueOnMouseDown_ = value_;
    lastMousePos_ = e.position;
    
    updateInteractionState(isHovered_, true);
    
    if (onDragStart)
        onDragStart();
    
    listeners_.call([this](Listener& l) { l.sliderDragStarted(this); });
}

void LayeredSlider::mouseDrag(const juce::MouseEvent& e)
{
    if (! isDragging_)
        return;
    
    if (style_ == SliderStyle::Rotary || style_ == SliderStyle::RotaryWithLabel)
        handleRotaryDrag(e);
    else
        handleLinearDrag(e);
    
    lastMousePos_ = e.position;
}

void LayeredSlider::mouseUp(const juce::MouseEvent& e)
{
    juce::ignoreUnused(e);
    
    if (isDragging_)
    {
        isDragging_ = false;
        updateInteractionState(isHovered_, false);
        
        if (onDragEnd)
            onDragEnd();
        
        listeners_.call([this](Listener& l) { l.sliderDragEnded(this); });
    }
}

void LayeredSlider::mouseDoubleClick(const juce::MouseEvent& e)
{
    juce::ignoreUnused(e);
    
    if (hasDoubleClickReturnValue_)
    {
        setValue(doubleClickReturnValue_, juce::sendNotificationSync);
    }
}

void LayeredSlider::mouseEnter(const juce::MouseEvent& e)
{
    juce::ignoreUnused(e);
    isHovered_ = true;
    updateInteractionState(true, isDragging_);
}

void LayeredSlider::mouseExit(const juce::MouseEvent& e)
{
    juce::ignoreUnused(e);
    isHovered_ = false;
    updateInteractionState(false, isDragging_);
}

void LayeredSlider::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    juce::ignoreUnused(e);
    
    if (! isEnabled())
        return;
    
    const double delta = (wheel.deltaY != 0.0f ? wheel.deltaY : wheel.deltaX);
    const double range = maximum_ - minimum_;
    const double step = interval_ > 0.0 ? interval_ : range * 0.01;
    
    setValue(value_ + delta * step * 10.0);
}

bool LayeredSlider::keyPressed(const juce::KeyPress& key)
{
    if (! isEnabled())
        return false;
    
    const double step = interval_ > 0.0 ? interval_ : (maximum_ - minimum_) * 0.01;
    
    if (key == juce::KeyPress::upKey || key == juce::KeyPress::rightKey)
    {
        setValue(value_ + step);
        return true;
    }
    else if (key == juce::KeyPress::downKey || key == juce::KeyPress::leftKey)
    {
        setValue(value_ - step);
        return true;
    }
    else if (key == juce::KeyPress::homeKey)
    {
        setValue(minimum_);
        return true;
    }
    else if (key == juce::KeyPress::endKey)
    {
        setValue(maximum_);
        return true;
    }
    
    return false;
}

void LayeredSlider::focusGained(FocusChangeType cause)
{
    juce::ignoreUnused(cause);
    repaint();
}

void LayeredSlider::focusLost(FocusChangeType cause)
{
    juce::ignoreUnused(cause);
    repaint();
}

//==============================================================================
// Private Methods

void LayeredSlider::setupDefaultLayers()
{
    // Start with rotary knob preset
    applyRotaryKnobPreset();
}

void LayeredSlider::updateLayersForValue()
{
    const double proportion = valueToProportionOfLength(value_);
    
    // All layer updates are now handled by ValueBinding in each layer
    layerStack_.updateForSliderValue(static_cast<float>(proportion));
    
    // Update value text layer with formatted value string
    if (auto* textLayer = getValueTextLayer())
    {
        textLayer->setText(getTextFromValue(value_));
    }
}

void LayeredSlider::updateInteractionState(bool hovered, bool pressed)
{
    for (int i = 0; i < layerStack_.getNumLayers(); ++i)
    {
        if (auto* layer = layerStack_.getLayer(i))
        {
            layer->setHovered(hovered);
            layer->setPressed(pressed);
        }
    }
    
    repaint();
}

double LayeredSlider::getValueFromMousePosition(juce::Point<float> pos) const
{
    const auto bounds = getLocalBounds().toFloat();
    double proportion = 0.0;
    
    if (style_ == SliderStyle::Rotary || style_ == SliderStyle::RotaryWithLabel)
    {
        const float centerX = bounds.getCentreX();
        const float centerY = bounds.getCentreY();
        
        float angle = std::atan2(pos.x - centerX, centerY - pos.y);
        if (angle < 0)
            angle += juce::MathConstants<float>::twoPi;
        
        const float range = rotaryParams_.endAngleRadians - rotaryParams_.startAngleRadians;
        proportion = (angle - rotaryParams_.startAngleRadians + juce::MathConstants<float>::halfPi) / range;
    }
    else
    {
        if (linearParams_.horizontal)
        {
            proportion = (pos.x - 10) / (bounds.getWidth() - 20);
        }
        else
        {
            proportion = 1.0 - (pos.y - 10) / (bounds.getHeight() - 20);
        }
    }
    
    return proportionOfLengthToValue(juce::jlimit(0.0, 1.0, proportion));
}

double LayeredSlider::constrainedValue(double val) const
{
    return juce::jlimit(minimum_, maximum_, val);
}

double LayeredSlider::snapValue(double val) const
{
    if (interval_ > 0.0)
    {
        val = minimum_ + std::round((val - minimum_) / interval_) * interval_;
    }
    return val;
}

void LayeredSlider::sendValueChangedMessage(juce::NotificationType notification)
{
    if (notification == juce::dontSendNotification)
        return;
    
    auto notify = [this]()
    {
        if (onValueChange)
            onValueChange();
        
        listeners_.call([this](Listener& l) { l.sliderValueChanged(this); });
    };
    
    if (notification == juce::sendNotificationSync)
        notify();
    else
        juce::MessageManager::callAsync(notify);
}

void LayeredSlider::handleRotaryDrag(const juce::MouseEvent& e)
{
    double valueDelta = 0.0;
    const float pixelsForFullRange = static_cast<float>(getHeight()) * 2.0f;
    
    switch (rotaryParams_.dragMode)
    {
        case RotaryDragMode::Circular:
        {
            // Original circular/angular drag mode
            const auto bounds = getLocalBounds().toFloat();
            const float centerX = bounds.getCentreX();
            const float centerY = bounds.getCentreY();
            
            const float lastAngle = std::atan2(lastMousePos_.x - centerX, centerY - lastMousePos_.y);
            const float currentAngle = std::atan2(e.position.x - centerX, centerY - e.position.y);
            
            float angleDelta = currentAngle - lastAngle;
            
            // Handle wrap-around
            if (angleDelta > juce::MathConstants<float>::pi)
                angleDelta -= juce::MathConstants<float>::twoPi;
            else if (angleDelta < -juce::MathConstants<float>::pi)
                angleDelta += juce::MathConstants<float>::twoPi;
            
            const float range = rotaryParams_.endAngleRadians - rotaryParams_.startAngleRadians;
            valueDelta = (angleDelta / range) * (maximum_ - minimum_) * rotaryParams_.sensitivity;
            break;
        }
        
        case RotaryDragMode::HorizontalDrag:
        {
            // Drag left/right only
            const float deltaX = e.position.x - lastMousePos_.x;
            const double normalizedDelta = deltaX / pixelsForFullRange;
            valueDelta = normalizedDelta * (maximum_ - minimum_) * rotaryParams_.sensitivity;
            break;
        }
        
        case RotaryDragMode::VerticalDrag:
        {
            // Drag up/down only (up = increase)
            const float deltaY = lastMousePos_.y - e.position.y;
            const double normalizedDelta = deltaY / pixelsForFullRange;
            valueDelta = normalizedDelta * (maximum_ - minimum_) * rotaryParams_.sensitivity;
            break;
        }
        
        case RotaryDragMode::HorizontalVerticalDrag:
        default:
        {
            // Drag in any direction (up or right = increase)
            const float deltaX = e.position.x - lastMousePos_.x;
            const float deltaY = lastMousePos_.y - e.position.y;
            const float mouseDelta = deltaX + deltaY;
            const double normalizedDelta = mouseDelta / pixelsForFullRange;
            valueDelta = normalizedDelta * (maximum_ - minimum_) * rotaryParams_.sensitivity;
            break;
        }
    }
    
    setValue(value_ + valueDelta);
}

void LayeredSlider::handleLinearDrag(const juce::MouseEvent& e)
{
    const auto bounds = getLocalBounds().toFloat();
    double proportion;
    
    if (linearParams_.horizontal)
    {
        proportion = (e.position.x - 10) / (bounds.getWidth() - 20);
    }
    else
    {
        proportion = 1.0 - (e.position.y - 10) / (bounds.getHeight() - 20);
    }
    
    setValue(proportionOfLengthToValue(juce::jlimit(0.0, 1.0, proportion)));
}

} // namespace zaplab
