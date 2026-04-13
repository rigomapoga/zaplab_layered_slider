/*
  ==============================================================================

    DevToolsLayers.h
    Created: 30 Nov 2025
    Author:  Paolo Zappalà @ zaplab.dev
    
    Description:
    Main include file for the DevTools Layer System.
    Include this single header to get access to all layer types.

  ==============================================================================
*/

#pragma once

// Core layer system
#include "Layer.h"
#include "ShapeLayer.h"
#include "TextLayer.h"
#include "ImageLayer.h"
#include "LayerStack.h"
#include "LayeredSlider.h"

namespace zaplab
{

//==============================================================================
/**
 * @brief Factory functions for creating layers
 */
namespace LayerFactory
{
    /**
     * @brief Create a layer from a LayerType enum
     */
    inline LayerPtr createLayer(LayerType type, const juce::String& name = "")
    {
        switch (type)
        {
            case LayerType::Shape:
                return std::make_shared<ShapeLayer>(name.isEmpty() ? "New Shape" : name);
            case LayerType::Text:
                return std::make_shared<TextLayer>(name.isEmpty() ? "New Text" : name);
            case LayerType::Image:
                return std::make_shared<ImageLayer>(name.isEmpty() ? "New Image" : name);
            case LayerType::Group:
                // TODO: Implement GroupLayer
                return nullptr;
            default:
                return nullptr;
        }
    }
    
    /**
     * @brief Create a shape layer with a specific shape type
     */
    inline std::shared_ptr<ShapeLayer> createShape(ShapeType type, const juce::String& name = "Shape")
    {
        auto layer = std::make_shared<ShapeLayer>(name);
        layer->setShapeType(type);
        return layer;
    }
    
    /**
     * @brief Create a rectangle shape layer
     */
    inline std::shared_ptr<ShapeLayer> createRectangle(const juce::String& name = "Rectangle")
    {
        return createShape(ShapeType::Rectangle, name);
    }
    
    /**
     * @brief Create an ellipse/circle shape layer
     */
    inline std::shared_ptr<ShapeLayer> createEllipse(const juce::String& name = "Ellipse")
    {
        return createShape(ShapeType::Ellipse, name);
    }
    
    /**
     * @brief Create an arc shape layer
     */
    inline std::shared_ptr<ShapeLayer> createArc(const juce::String& name = "Arc")
    {
        return createShape(ShapeType::Arc, name);
    }
    
    /**
     * @brief Create a text layer with text content
     */
    inline std::shared_ptr<TextLayer> createText(const juce::String& text, const juce::String& name = "Text")
    {
        auto layer = std::make_shared<TextLayer>(name);
        layer->setText(text);
        return layer;
    }
    
    /**
     * @brief Create an image layer from a file
     */
    inline std::shared_ptr<ImageLayer> createImageFromFile(const juce::File& file, const juce::String& name = "Image")
    {
        auto layer = std::make_shared<ImageLayer>(name);
        layer->setImageFromFile(file);
        return layer;
    }
    
    /**
     * @brief Create a layer from a ValueTree
     */
    inline LayerPtr createFromValueTree(const juce::ValueTree& tree)
    {
        if (!tree.isValid())
            return nullptr;
        
        const auto typeInt = static_cast<int>(tree.getProperty("type", 0));
        const auto type = static_cast<LayerType>(typeInt);
        
        auto layer = createLayer(type);
        if (layer)
            layer->fromValueTree(tree);
        
        return layer;
    }
}

//==============================================================================
/**
 * @brief Helper functions for common layer operations
 */
namespace LayerUtils
{
    /**
     * @brief Convert ShapeType to string
     */
    inline juce::String shapeTypeToString(ShapeType type)
    {
        switch (type)
        {
            case ShapeType::None:       return "None";
            case ShapeType::Rectangle:  return "Rectangle";
            case ShapeType::Ellipse:    return "Ellipse";
            case ShapeType::Arc:        return "Arc";
            case ShapeType::Polygon:    return "Polygon";
            case ShapeType::Star:       return "Star";
            case ShapeType::Triangle:   return "Triangle";
            case ShapeType::Line:       return "Line";
            default:                    return "Unknown";
        }
    }
    
    /**
     * @brief Convert string to ShapeType
     */
    inline ShapeType stringToShapeType(const juce::String& str)
    {
        if (str == "Rectangle")  return ShapeType::Rectangle;
        if (str == "Ellipse")    return ShapeType::Ellipse;
        if (str == "Arc")        return ShapeType::Arc;
        if (str == "Polygon")    return ShapeType::Polygon;
        if (str == "Star")       return ShapeType::Star;
        if (str == "Triangle")   return ShapeType::Triangle;
        if (str == "Line")       return ShapeType::Line;
        return ShapeType::None;
    }
    
    /**
     * @brief Convert LayerType to string
     */
    inline juce::String layerTypeToString(LayerType type)
    {
        switch (type)
        {
            case LayerType::Shape:  return "Shape";
            case LayerType::Text:   return "Text";
            case LayerType::Image:  return "Image";
            case LayerType::Group:  return "Group";
            default:                return "Unknown";
        }
    }
    
    /**
     * @brief Convert string to LayerType
     */
    inline LayerType stringToLayerType(const juce::String& str)
    {
        if (str == "Shape")  return LayerType::Shape;
        if (str == "Text")   return LayerType::Text;
        if (str == "Image")  return LayerType::Image;
        if (str == "Group")  return LayerType::Group;
        return LayerType::Shape;
    }
    
    /**
     * @brief Convert SliderLayerRole to string
     */
    inline juce::String sliderRoleToString(SliderLayerRole role)
    {
        switch (role)
        {
            case SliderLayerRole::None:         return "None";
            case SliderLayerRole::Thumb:        return "Thumb";
            case SliderLayerRole::Track:        return "Track";
            case SliderLayerRole::ValueTrack:   return "ValueTrack";
            case SliderLayerRole::ModForward:   return "ModForward";
            case SliderLayerRole::ModBackward:  return "ModBackward";
            case SliderLayerRole::Overlay:      return "Overlay";
            default:                            return "Unknown";
        }
    }
    
    /**
     * @brief Convert string to SliderLayerRole
     */
    inline SliderLayerRole stringToSliderRole(const juce::String& str)
    {
        if (str == "Thumb")        return SliderLayerRole::Thumb;
        if (str == "Track")        return SliderLayerRole::Track;
        if (str == "ValueTrack")   return SliderLayerRole::ValueTrack;
        if (str == "ModForward")   return SliderLayerRole::ModForward;
        if (str == "ModBackward")  return SliderLayerRole::ModBackward;
        if (str == "Overlay")      return SliderLayerRole::Overlay;
        return SliderLayerRole::None;
    }
    
    /**
     * @brief Get all available shape type names
     */
    inline juce::StringArray getShapeTypeNames()
    {
        return {
            "Rectangle", "Ellipse", "Arc", "Polygon", "Star", "Triangle", "Line"
        };
    }
    
    /**
     * @brief Get all available slider role names
     */
    inline juce::StringArray getSliderRoleNames()
    {
        return {
            "None", "Thumb", "Track", "ValueTrack", "ModForward", "ModBackward", "Overlay"
        };
    }
}

} // namespace zaplab
