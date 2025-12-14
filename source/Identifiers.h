/*
  ==============================================================================

    Identifiers.h
    Created: 13 Dec 2025
    Author:  DevTools SliderDesigner
    
    Description:
    Constant identifiers for property names, JSON keys, and preset metadata.
    Using constexpr prevents typos and enables IDE autocomplete.

  ==============================================================================
*/

#pragma once

#include <string>

namespace zaplab
{

//==============================================================================
/**
 * @brief Constant identifiers for consistent naming across the codebase
 */
namespace Identifiers
{
    //==========================================================================
    // Metadata Keys
    constexpr const char* metadata = "metadata";
    constexpr const char* version = "version";
    constexpr const char* format = "format";
    constexpr const char* appName = "appName";
    constexpr const char* appVersion = "appVersion";
    constexpr const char* lastSaveDate = "lastSaveDate";
    constexpr const char* author = "author";
    constexpr const char* description = "description";
    constexpr const char* tags = "tags";
    
    //==========================================================================
    // Preset Format Identifiers
    constexpr const char* formatSliderDesign = "slider-designer-json";
    constexpr const char* formatLayerStack = "layer-stack-json";
    constexpr const char* currentVersion = "2.0";
    
    //==========================================================================
    // Layer Properties
    constexpr const char* layer = "layer";
    constexpr const char* layers = "layers";
    constexpr const char* layerType = "layerType";
    constexpr const char* layerName = "name";
    constexpr const char* layerId = "id";
    constexpr const char* layerIndex = "index";
    constexpr const char* visible = "visible";
    constexpr const char* locked = "locked";
    
    //==========================================================================
    // Transform Properties
    constexpr const char* transform = "transform";
    constexpr const char* position = "position";
    constexpr const char* positionX = "positionX";
    constexpr const char* positionY = "positionY";
    constexpr const char* anchor = "anchor";
    constexpr const char* rotation = "rotation";
    constexpr const char* scale = "scale";
    constexpr const char* scaleX = "scaleX";
    constexpr const char* scaleY = "scaleY";
    constexpr const char* size = "size";
    constexpr const char* width = "width";
    constexpr const char* height = "height";
    
    //==========================================================================
    // Style Properties
    constexpr const char* style = "style";
    constexpr const char* fillMode = "fillMode";
    constexpr const char* hasFill = "hasFill";
    constexpr const char* fillColour = "fillColour";
    constexpr const char* fillGradient = "fillGradient";
    constexpr const char* hasStroke = "hasStroke";
    constexpr const char* strokeColour = "strokeColour";
    constexpr const char* strokeWidth = "strokeWidth";
    constexpr const char* strokeAlignment = "strokeAlignment";
    constexpr const char* cornerRadius = "cornerRadius";
    constexpr const char* opacity = "opacity";
    constexpr const char* blendMode = "blendMode";
    
    //==========================================================================
    // Gradient Properties
    constexpr const char* gradient = "gradient";
    constexpr const char* gradientType = "type";
    constexpr const char* gradientStops = "stops";
    constexpr const char* gradientStart = "start";
    constexpr const char* gradientEnd = "end";
    constexpr const char* gradientRotation = "rotation";
    
    //==========================================================================
    // Shadow Properties
    constexpr const char* shadow = "shadow";
    constexpr const char* shadowEnabled = "enabled";
    constexpr const char* shadowColour = "colour";
    constexpr const char* shadowOffset = "offset";
    constexpr const char* shadowRadius = "radius";
    constexpr const char* shadowSpread = "spread";
    
    //==========================================================================
    // Effects
    constexpr const char* effects = "effects";
    constexpr const char* dropShadow = "dropShadow";
    constexpr const char* innerShadow = "innerShadow";
    constexpr const char* outerGlow = "outerGlow";
    constexpr const char* innerGlow = "innerGlow";
    constexpr const char* bevelEmboss = "bevelEmboss";
    constexpr const char* gaussianBlur = "gaussianBlur";
    constexpr const char* colourOverlay = "colourOverlay";
    
    //==========================================================================
    // Shape Layer Properties
    constexpr const char* shapeType = "shapeType";
    constexpr const char* sizeRatio = "sizeRatio";
    constexpr const char* padding = "padding";
    constexpr const char* arcConfig = "arcConfig";
    constexpr const char* polygonConfig = "polygonConfig";
    constexpr const char* lineConfig = "lineConfig";
    
    //==========================================================================
    // Arc Configuration
    constexpr const char* startAngle = "startAngle";
    constexpr const char* endAngle = "endAngle";
    constexpr const char* thickness = "thickness";
    constexpr const char* connectToValue = "connectToValue";
    
    //==========================================================================
    // Text Layer Properties
    constexpr const char* text = "text";
    constexpr const char* font = "font";
    constexpr const char* fontSize = "fontSize";
    constexpr const char* fontName = "fontName";
    constexpr const char* fontStyle = "fontStyle";
    constexpr const char* textColour = "textColour";
    constexpr const char* alignment = "alignment";
    constexpr const char* lineSpacing = "lineSpacing";
    constexpr const char* isMultiline = "isMultiline";
    constexpr const char* isVerticalText = "isVerticalText";
    constexpr const char* displaySliderValue = "displaySliderValue";
    constexpr const char* valueFormat = "valueFormat";
    constexpr const char* valueDecimals = "valueDecimals";
    
    //==========================================================================
    // Image Layer Properties
    constexpr const char* assetId = "assetId";
    constexpr const char* imagePath = "imagePath";
    constexpr const char* imageMode = "imageMode";
    constexpr const char* placement = "placement";
    constexpr const char* imageAlpha = "imageAlpha";
    constexpr const char* filmstrip = "filmstrip";
    constexpr const char* overlay = "overlay";
    constexpr const char* frameCount = "frameCount";
    constexpr const char* currentFrame = "currentFrame";
    
    //==========================================================================
    // Value Binding Properties
    constexpr const char* valueBindings = "valueBindings";
    constexpr const char* property = "property";
    constexpr const char* enabled = "enabled";
    constexpr const char* curve = "curve";
    constexpr const char* valueAtMin = "valueAtMin";
    constexpr const char* valueAtMax = "valueAtMax";
    constexpr const char* steps = "steps";
    constexpr const char* isRelative = "isRelative";
    constexpr const char* bipolar = "bipolar";
    
    //==========================================================================
    // Slider Properties
    constexpr const char* slider = "slider";
    constexpr const char* sliderStyle = "style";
    constexpr const char* minimum = "minimum";
    constexpr const char* maximum = "maximum";
    constexpr const char* interval = "interval";
    constexpr const char* value = "value";
    constexpr const char* skewFactor = "skewFactor";
    constexpr const char* bipolarMode = "bipolarMode";
    constexpr const char* textSuffix = "textSuffix";
    constexpr const char* decimalPlaces = "decimalPlaces";
    
    //==========================================================================
    // Rotary Parameters
    constexpr const char* rotaryParameters = "rotaryParameters";
    constexpr const char* stopAtEnd = "stopAtEnd";
    constexpr const char* sensitivity = "sensitivity";
    
    //==========================================================================
    // Linear Parameters
    constexpr const char* linearParameters = "linearParameters";
    constexpr const char* horizontal = "horizontal";
    constexpr const char* thumbSize = "thumbSize";
    constexpr const char* showTrack = "showTrack";
    constexpr const char* showValueTrack = "showValueTrack";
    
    //==========================================================================
    // Layer Stack
    constexpr const char* layerStack = "layerStack";
    constexpr const char* backgroundColour = "backgroundColour";
    constexpr const char* transparentBackground = "transparentBackground";
    
    //==========================================================================
    // Assets
    constexpr const char* assets = "assets";
    constexpr const char* asset = "asset";
    constexpr const char* base64Data = "base64Data";
    constexpr const char* fileSize = "fileSize";
    constexpr const char* referenceCount = "referenceCount";
    
    //==========================================================================
    // Slider Roles
    constexpr const char* sliderRole = "sliderRole";
    constexpr const char* sizingMode = "sizingMode";
    
    //==========================================================================
    // Mask Properties
    constexpr const char* mask = "mask";
    constexpr const char* inverted = "inverted";
    constexpr const char* maskLayerName = "maskLayerName";
    
    //==========================================================================
    // Common Values
    constexpr const char* none = "none";
    constexpr const char* unset = "unset";
    constexpr const char* defaultValue = "default";
    
} // namespace Identifiers

} // namespace zaplab
