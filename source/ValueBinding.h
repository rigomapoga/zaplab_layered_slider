/*
  ==============================================================================

    ValueBinding.h
    Created: 11 Dec 2025

    Value binding configuration for connecting layer properties to slider values.

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <cmath>
#include <utility>

namespace zaplab
{

enum class ValueBindingProperty
{
    None,
    Rotation,
    RotationAroundPoint,
    PositionX,
    PositionY,
    Width,
    Height,
    ScaleX,
    ScaleY,
    ScaleUniform,
    Opacity,
    ArcEnd,
    ArcStart,
    ArcThickness,
    OuterGlowBlur,
    OverlayOpacity
};

enum class ValueBindingCurve
{
    Linear,
    EaseIn,
    EaseOut,
    EaseInOut,
    Stepped
};

struct ValueBinding
{
    bool enabled { false };
    ValueBindingProperty property { ValueBindingProperty::None };
    ValueBindingCurve curve { ValueBindingCurve::Linear };

    float valueAtMin { 0.0f };
    float valueAtMax { 1.0f };

    float originalValue { 0.0f };
    bool hasStoredOriginal { false };

    int steps { 10 };

    bool isRelative { true };
    float rotationOffset { 0.0f };

    juce::Point<float> pivotPoint { 0.5f, 0.5f };
    float orbitRadius { 0.3f };

    bool bipolar { false };

    [[nodiscard]] float computePropertyValue(float normalizedValue) const noexcept
    {
        float t = juce::jlimit(0.0f, 1.0f, normalizedValue);

        if (bipolar)
            t = std::abs(t - 0.5f) * 2.0f;

        switch (curve)
        {
            case ValueBindingCurve::Linear:
                break;
            case ValueBindingCurve::EaseIn:
                t = t * t;
                break;
            case ValueBindingCurve::EaseOut:
                t = 1.0f - (1.0f - t) * (1.0f - t);
                break;
            case ValueBindingCurve::EaseInOut:
                t = t < 0.5f ? 2.0f * t * t
                             : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
                break;
            case ValueBindingCurve::Stepped:
                if (steps > 1)
                    t = std::round(t * (steps - 1)) / (steps - 1);
                break;
        }

        return valueAtMin + (valueAtMax - valueAtMin) * t;
    }

    [[nodiscard]] static juce::String propertyToString(ValueBindingProperty prop) noexcept
    {
        switch (prop)
        {
            case ValueBindingProperty::None:                return "None";
            case ValueBindingProperty::Rotation:            return "Rotation";
            case ValueBindingProperty::RotationAroundPoint: return "Orbit";
            case ValueBindingProperty::PositionX:           return "Position X";
            case ValueBindingProperty::PositionY:           return "Position Y";
            case ValueBindingProperty::Width:               return "Width";
            case ValueBindingProperty::Height:              return "Height";
            case ValueBindingProperty::ScaleX:              return "Scale X";
            case ValueBindingProperty::ScaleY:              return "Scale Y";
            case ValueBindingProperty::ScaleUniform:        return "Scale";
            case ValueBindingProperty::Opacity:             return "Opacity";
            case ValueBindingProperty::ArcEnd:              return "Arc End";
            case ValueBindingProperty::ArcStart:            return "Arc Start";
            case ValueBindingProperty::ArcThickness:        return "Arc Thickness";
            case ValueBindingProperty::OuterGlowBlur:       return "Outer Glow Blur";
            case ValueBindingProperty::OverlayOpacity:      return "Overlay Opacity";
        }

        return "None";
    }

    [[nodiscard]] static ValueBindingProperty stringToProperty(const juce::String& str) noexcept
    {
        if (str == "Rotation")         return ValueBindingProperty::Rotation;
        if (str == "Orbit")            return ValueBindingProperty::RotationAroundPoint;
        if (str == "Position X")       return ValueBindingProperty::PositionX;
        if (str == "Position Y")       return ValueBindingProperty::PositionY;
        if (str == "Width")            return ValueBindingProperty::Width;
        if (str == "Height")           return ValueBindingProperty::Height;
        if (str == "Scale X")          return ValueBindingProperty::ScaleX;
        if (str == "Scale Y")          return ValueBindingProperty::ScaleY;
        if (str == "Scale")            return ValueBindingProperty::ScaleUniform;
        if (str == "Opacity")          return ValueBindingProperty::Opacity;
        if (str == "Arc End")          return ValueBindingProperty::ArcEnd;
        if (str == "Arc Start")        return ValueBindingProperty::ArcStart;
        if (str == "Arc Thickness")    return ValueBindingProperty::ArcThickness;
        if (str == "Outer Glow Blur")  return ValueBindingProperty::OuterGlowBlur;
        if (str == "Overlay Opacity")  return ValueBindingProperty::OverlayOpacity;
        return ValueBindingProperty::None;
    }

    [[nodiscard]] static juce::String curveToString(ValueBindingCurve c) noexcept
    {
        switch (c)
        {
            case ValueBindingCurve::Linear:    return "Linear";
            case ValueBindingCurve::EaseIn:    return "Ease In";
            case ValueBindingCurve::EaseOut:   return "Ease Out";
            case ValueBindingCurve::EaseInOut: return "Ease In-Out";
            case ValueBindingCurve::Stepped:   return "Stepped";
        }

        return "Linear";
    }

    [[nodiscard]] static ValueBindingCurve stringToCurve(const juce::String& str) noexcept
    {
        if (str == "Ease In")     return ValueBindingCurve::EaseIn;
        if (str == "Ease Out")    return ValueBindingCurve::EaseOut;
        if (str == "Ease In-Out") return ValueBindingCurve::EaseInOut;
        if (str == "Stepped")     return ValueBindingCurve::Stepped;
        return ValueBindingCurve::Linear;
    }

    [[nodiscard]] static std::pair<float, float> getDefaultRange(ValueBindingProperty prop) noexcept
    {
        switch (prop)
        {
            case ValueBindingProperty::Rotation:
            case ValueBindingProperty::RotationAroundPoint:
                return { juce::MathConstants<float>::pi * 1.25f,
                         juce::MathConstants<float>::pi * 2.75f };

            case ValueBindingProperty::PositionX:
            case ValueBindingProperty::PositionY:
            case ValueBindingProperty::Width:
            case ValueBindingProperty::Height:
                return { 0.0f, 1.0f };

            case ValueBindingProperty::ScaleX:
            case ValueBindingProperty::ScaleY:
            case ValueBindingProperty::ScaleUniform:
                return { 0.5f, 1.5f };

            case ValueBindingProperty::Opacity:
            case ValueBindingProperty::OverlayOpacity:
                return { 0.0f, 1.0f };

            case ValueBindingProperty::ArcEnd:
            case ValueBindingProperty::ArcStart:
                return { juce::MathConstants<float>::pi * 1.25f,
                         juce::MathConstants<float>::pi * 2.75f };

            case ValueBindingProperty::ArcThickness:
                return { 0.1f, 1.0f };

            case ValueBindingProperty::OuterGlowBlur:
                return { 0.0f, 50.0f };

            case ValueBindingProperty::None:
                return { 0.0f, 1.0f };
        }

        return { 0.0f, 1.0f };
    }
};

} // namespace zaplab
