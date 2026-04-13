#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../libs/nlohmann/json.hpp"
#include "LayerTypes.h"
#include "Identifiers.h"
#include "PresetMetadata.h"
#include "DevToolsUtilities.h"

// For convenience
using json = nlohmann::json;

//==============================================================================
// JSON conversion utilities for JUCE types
//==============================================================================

namespace juce
{
    // Colour
    inline void to_json(json& j, const Colour& c)
    {
        j = json{
            {"r", c.getRed()},
            {"g", c.getGreen()},
            {"b", c.getBlue()},
            {"a", c.getAlpha()}
        };
    }

    inline void from_json(const json& j, Colour& c)
    {
        c = Colour(
            static_cast<uint8>(j.at("r").get<int>()),
            static_cast<uint8>(j.at("g").get<int>()),
            static_cast<uint8>(j.at("b").get<int>()),
            static_cast<uint8>(j.at("a").get<int>())
        );
    }

    // Point<float>
    inline void to_json(json& j, const Point<float>& p)
    {
        j = json{{"x", p.x}, {"y", p.y}};
    }

    inline void from_json(const json& j, Point<float>& p)
    {
        p.x = j.at("x").get<float>();
        p.y = j.at("y").get<float>();
    }

    // Rectangle<float>
    inline void to_json(json& j, const Rectangle<float>& r)
    {
        j = json{
            {"x", r.getX()},
            {"y", r.getY()},
            {"width", r.getWidth()},
            {"height", r.getHeight()}
        };
    }

    inline void from_json(const json& j, Rectangle<float>& r)
    {
        r = Rectangle<float>(
            j.at("x").get<float>(),
            j.at("y").get<float>(),
            j.at("width").get<float>(),
            j.at("height").get<float>()
        );
    }

    // AffineTransform
    inline void to_json(json& j, const AffineTransform& t)
    {
        j = json{
            {"mat00", t.mat00}, {"mat01", t.mat01}, {"mat02", t.mat02},
            {"mat10", t.mat10}, {"mat11", t.mat11}, {"mat12", t.mat12}
        };
    }

    inline void from_json(const json& j, AffineTransform& t)
    {
        t = AffineTransform(
            j.at("mat00").get<float>(), j.at("mat01").get<float>(), j.at("mat02").get<float>(),
            j.at("mat10").get<float>(), j.at("mat11").get<float>(), j.at("mat12").get<float>()
        );
    }

    // Font
    inline void to_json(json& j, const Font& f)
    {
        j = json{
            {"typefaceName", f.getTypefaceName().toStdString()},
            {"height", f.getHeight()},
            {"bold", f.isBold()},
            {"italic", f.isItalic()}
        };
    }

    inline void from_json(const json& j, Font& f)
    {
        f = Font(
            j.at("typefaceName").get<std::string>(),
            j.at("height").get<float>(),
            j.value("bold", false) ? Font::bold : 
            j.value("italic", false) ? Font::italic : Font::plain
        );
    }
}

//==============================================================================
// JSON conversion for DevTools types
//==============================================================================

// ShapeType enum
NLOHMANN_JSON_SERIALIZE_ENUM(zaplab::ShapeType, {
    {zaplab::ShapeType::None, "None"},
    {zaplab::ShapeType::Rectangle, "Rectangle"},
    {zaplab::ShapeType::Ellipse, "Ellipse"},
    {zaplab::ShapeType::Triangle, "Triangle"},
    {zaplab::ShapeType::Arc, "Arc"},
    {zaplab::ShapeType::Line, "Line"},
    {zaplab::ShapeType::Polygon, "Polygon"}
})

// LayerType enum
NLOHMANN_JSON_SERIALIZE_ENUM(zaplab::LayerType, {
    {zaplab::LayerType::Shape, "Shape"},
    {zaplab::LayerType::Text, "Text"},
    {zaplab::LayerType::Image, "Image"},
    {zaplab::LayerType::Group, "Group"}
})

// StrokeAlignment enum
NLOHMANN_JSON_SERIALIZE_ENUM(zaplab::StrokeAlignment, {
    {zaplab::StrokeAlignment::Center, "Center"},
    {zaplab::StrokeAlignment::Inside, "Inside"},
    {zaplab::StrokeAlignment::Outside, "Outside"}
})

// BlendMode enum
NLOHMANN_JSON_SERIALIZE_ENUM(zaplab::BlendMode, {
    {zaplab::BlendMode::Normal, "Normal"},
    {zaplab::BlendMode::Multiply, "Multiply"},
    {zaplab::BlendMode::Screen, "Screen"},
    {zaplab::BlendMode::Overlay, "Overlay"},
    {zaplab::BlendMode::Add, "Add"}
})

// ValueBindingProperty enum
NLOHMANN_JSON_SERIALIZE_ENUM(zaplab::ValueBindingProperty, {
    {zaplab::ValueBindingProperty::None, "None"},
    {zaplab::ValueBindingProperty::Rotation, "Rotation"},
    {zaplab::ValueBindingProperty::RotationAroundPoint, "RotationAroundPoint"},
    {zaplab::ValueBindingProperty::PositionX, "PositionX"},
    {zaplab::ValueBindingProperty::PositionY, "PositionY"},
    {zaplab::ValueBindingProperty::Width, "Width"},
    {zaplab::ValueBindingProperty::Height, "Height"},
    {zaplab::ValueBindingProperty::ScaleX, "ScaleX"},
    {zaplab::ValueBindingProperty::ScaleY, "ScaleY"},
    {zaplab::ValueBindingProperty::ScaleUniform, "ScaleUniform"},
    {zaplab::ValueBindingProperty::Opacity, "Opacity"},
    {zaplab::ValueBindingProperty::ArcEnd, "ArcEnd"},
    {zaplab::ValueBindingProperty::ArcStart, "ArcStart"},
    {zaplab::ValueBindingProperty::ArcThickness, "ArcThickness"},
    {zaplab::ValueBindingProperty::OuterGlowBlur, "OuterGlowBlur"},
    {zaplab::ValueBindingProperty::OverlayOpacity, "OverlayOpacity"}
})

// GradientType enum
NLOHMANN_JSON_SERIALIZE_ENUM(zaplab::GradientType, {
    {zaplab::GradientType::Linear, "Linear"},
    {zaplab::GradientType::Radial, "Radial"},
    {zaplab::GradientType::Conic, "Conic"}
})

// FillMode enum
NLOHMANN_JSON_SERIALIZE_ENUM(zaplab::FillMode, {
    {zaplab::FillMode::Solid, "Solid"},
    {zaplab::FillMode::LinearGradient, "LinearGradient"},
    {zaplab::FillMode::RadialGradient, "RadialGradient"},
    {zaplab::FillMode::ConicGradient, "ConicGradient"},
    {zaplab::FillMode::None, "None"}
})

//==============================================================================
// DevTools types serialization
//==============================================================================

namespace zaplab
{
    // Forward declarations
    class Layer;
    struct LayerMask;

    // Gradient struct
    inline void to_json(json& j, const Gradient& g)
    {
        // Manually serialize stops
        json stopsJson = json::array();
        for (const auto& stop : g.stops)
        {
            stopsJson.push_back({
                {"position", stop.first},
                {"colour", stop.second}
            });
        }

        j = json{
            {"enabled", g.enabled},
            {"type", g.type},
            {"stops", stopsJson},
            {"start", g.start},
            {"end", g.end},
            {"rotation", g.rotation},
            {"colour1", g.colour1},
            {"colour2", g.colour2}
        };
    }

    inline void from_json(const json& j, Gradient& g)
    {
        g.enabled = j.value("enabled", false);
        g.type = j.value("type", GradientType::Linear);
        
        if (j.contains("stops"))
        {
            g.stops.clear();
            for (const auto& stopJson : j["stops"])
            {
                g.stops.emplace_back(
                    stopJson.at("position").get<float>(),
                    stopJson.at("colour").get<juce::Colour>()
                );
            }
        }

        if (j.contains("start")) j.at("start").get_to(g.start);
        if (j.contains("end")) j.at("end").get_to(g.end);
        g.rotation = j.value("rotation", 0.0f);
        if (j.contains("colour1")) j.at("colour1").get_to(g.colour1);
        if (j.contains("colour2")) j.at("colour2").get_to(g.colour2);
    }

    // Shadow struct
    inline void to_json(json& j, const Shadow& s)
    {
        j = json{
            {"enabled", s.enabled},
            {"colour", s.colour},
            {"offset", s.offset},
            {"radius", s.radius},
            {"spread", s.spread}
        };
    }

    inline void from_json(const json& j, Shadow& s)
    {
        s.enabled = j.value("enabled", false);
        if (j.contains("colour")) j.at("colour").get_to(s.colour);
        if (j.contains("offset")) j.at("offset").get_to(s.offset);
        s.radius = j.value("radius", 0.04f);
        s.spread = j.value("spread", 0.0f);
    }

    // LayerStyle
    inline void to_json(json& j, const LayerStyle& style)
    {
        j = json{
            {"fillMode", style.fillMode},
            {"hasFill", style.hasFill},
            {"fillColour", style.fillColour},
            {"fillGradient", style.fillGradient},
            {"hasStroke", style.hasStroke},
            {"strokeColour", style.strokeColour},
            {"strokeWidth", style.strokeWidth},
            {"strokeAlignment", style.strokeAlignment},
            {"cornerRadius", style.cornerRadius},
            {"cornerRadiusTL", style.cornerRadiusTL},
            {"cornerRadiusTR", style.cornerRadiusTR},
            {"cornerRadiusBL", style.cornerRadiusBL},
            {"cornerRadiusBR", style.cornerRadiusBR},
            {"useIndividualCorners", style.useIndividualCorners},
            {"shadow", style.shadow},
            {"opacity", style.opacity},
            {"blendMode", style.blendMode}
        };
    }

    inline void from_json(const json& j, LayerStyle& style)
    {
        style.fillMode = j.value("fillMode", FillMode::Solid);
        style.hasFill = j.value("hasFill", true);
        if (j.contains("fillColour")) j.at("fillColour").get_to(style.fillColour);
        if (j.contains("fillGradient")) j.at("fillGradient").get_to(style.fillGradient);
        style.hasStroke = j.value("hasStroke", false);
        if (j.contains("strokeColour")) j.at("strokeColour").get_to(style.strokeColour);
        style.strokeWidth = j.value("strokeWidth", 0.01f);
        style.strokeAlignment = j.value("strokeAlignment", StrokeAlignment::Center);
        style.cornerRadius = j.value("cornerRadius", 0.0f);
        style.cornerRadiusTL = j.value("cornerRadiusTL", 0.0f);
        style.cornerRadiusTR = j.value("cornerRadiusTR", 0.0f);
        style.cornerRadiusBL = j.value("cornerRadiusBL", 0.0f);
        style.cornerRadiusBR = j.value("cornerRadiusBR", 0.0f);
        style.useIndividualCorners = j.value("useIndividualCorners", false);
        if (j.contains("shadow")) j.at("shadow").get_to(style.shadow);
        style.opacity = j.value("opacity", 1.0f);
        style.blendMode = j.value("blendMode", BlendMode::Normal);
    }

    // ValueBinding
    inline void to_json(json& j, const ValueBinding& binding)
    {
        j = json{
            {"property", binding.property},
            {"enabled", binding.enabled},
            {"curve", binding.curve},
            {"valueAtMin", binding.valueAtMin},
            {"valueAtMax", binding.valueAtMax},
            {"steps", binding.steps},
            {"isRelative", binding.isRelative},
            {"rotationOffset", binding.rotationOffset},
            {"pivotPoint", binding.pivotPoint},
            {"orbitRadius", binding.orbitRadius},
            {"bipolar", binding.bipolar}
        };
        
        // Only include original value if it was stored
        if (binding.hasStoredOriginal)
        {
            j["originalValue"] = binding.originalValue;
            j["hasStoredOriginal"] = true;
        }
    }

    inline void from_json(const json& j, ValueBinding& binding)
    {
        j.at("property").get_to(binding.property);
        binding.enabled = j.value("enabled", false);
        j.at("curve").get_to(binding.curve);
        binding.valueAtMin = j.value("valueAtMin", 0.0f);
        binding.valueAtMax = j.value("valueAtMax", 1.0f);
        binding.steps = j.value("steps", 10);
        binding.isRelative = j.value("isRelative", true);
        binding.rotationOffset = j.value("rotationOffset", 0.0f);
        if (j.contains("pivotPoint"))
            j.at("pivotPoint").get_to(binding.pivotPoint);
        binding.orbitRadius = j.value("orbitRadius", 0.3f);
        binding.bipolar = j.value("bipolar", false);
        
        // Restore original value if present
        binding.hasStoredOriginal = j.value("hasStoredOriginal", false);
        if (binding.hasStoredOriginal)
        {
            binding.originalValue = j.value("originalValue", 0.0f);
        }
    }
} // namespace zaplab
