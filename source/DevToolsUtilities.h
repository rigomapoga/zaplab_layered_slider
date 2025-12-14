/*
  ==============================================================================

    DevToolsUtilities.h
    Created: 13 Dec 2025
    Author:  DevTools SliderDesigner
    
    Description:
    Utility functions for JSON handling, file operations, type conversions,
    and general helpers. Inspired by DataStructure.h patterns.

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../libs/nlohmann/json.hpp"
#include <variant>
#include <optional>
#include <fstream>
#include <bitset>

namespace zaplab
{

using json = nlohmann::json;

//==============================================================================
/**
 * @brief Utility namespace with helper functions
 */
namespace Utilities
{

//==============================================================================
// JSON File Operations

/**
 * @brief Load JSON from a file with error handling
 * @param file The file to load
 * @return The parsed JSON, or empty object on error
 */
inline std::optional<json> loadJsonFromFile(const juce::File& file)
{
    if (!file.existsAsFile())
    {
        DBG("File does not exist: " << file.getFullPathName());
        return std::nullopt;
    }
    
    try
    {
        juce::String content = file.loadFileAsString();
        if (content.isNotEmpty())
        {
            return json::parse(content.toStdString());
        }
    }
    catch (const json::parse_error& e)
    {
        DBG("JSON parse error: " << e.what());
    }
    catch (const std::exception& e)
    {
        DBG("Error loading JSON: " << e.what());
    }
    
    return std::nullopt;
}

/**
 * @brief Save JSON to a file with pretty printing
 * @param j The JSON to save
 * @param file The destination file
 * @param indent Indentation spaces (0 for compact)
 * @return True if successful
 */
inline bool saveJsonToFile(const json& j, const juce::File& file, int indent = 2)
{
    try
    {
        // Ensure parent directory exists
        file.getParentDirectory().createDirectory();
        
        juce::String content = juce::String(j.dump(indent));
        return file.replaceWithText(content);
    }
    catch (const std::exception& e)
    {
        DBG("Error saving JSON: " << e.what());
        return false;
    }
}

/**
 * @brief Check if a file contains JSON data
 */
inline bool isJsonFile(const juce::File& file)
{
    if (!file.existsAsFile())
        return false;
    
    if (file.hasFileExtension(".json"))
        return true;
    
    // Check content
    juce::String content = file.loadFileAsString().trimStart();
    return content.startsWithChar('{') || content.startsWithChar('[');
}

/**
 * @brief Check if a file contains XML data
 */
inline bool isXmlFile(const juce::File& file)
{
    if (!file.existsAsFile())
        return false;
    
    if (file.hasFileExtension(".xml"))
        return true;
    
    // Check content
    juce::String content = file.loadFileAsString().trimStart();
    return content.startsWithChar('<');
}

//==============================================================================
// Resource Loading

/**
 * @brief Load data from BinaryData resources
 * @param resourceName The resource name (with extension)
 * @return Optional pair of data pointer and size
 * @note Requires BinaryData to be defined in the project
 */
inline std::optional<std::pair<const char*, int>> getDataFromResources(const juce::String& name)
{
#if defined(BinaryData_H) || __has_include("BinaryData.h")
    // Replace '.' with '_' for BinaryData naming convention
    juce::String rawName = name.replaceCharacter('.', '_');
    
    int dataSize = 0;
    const char* data = BinaryData::getNamedResource(rawName.toRawUTF8(), dataSize);
    
    if (data == nullptr)
        return std::nullopt;
    
    return std::make_pair(data, dataSize);
#else
    juce::ignoreUnused(name);
    return std::nullopt;
#endif
}

/**
 * @brief Load a resource as a string
 */
inline juce::String loadResourceAsString(const juce::String& resourceName)
{
    auto resourceData = getDataFromResources(resourceName);
    if (resourceData)
    {
        return juce::String::fromUTF8(resourceData->first, resourceData->second);
    }
    return {};
}

/**
 * @brief Load a JSON resource
 */
inline std::optional<json> loadJsonFromResource(const juce::String& resourceName)
{
    juce::String content = loadResourceAsString(resourceName);
    if (content.isNotEmpty())
    {
        try
        {
            return json::parse(content.toStdString());
        }
        catch (...)
        {
            DBG("Failed to parse JSON resource: " << resourceName);
        }
    }
    return std::nullopt;
}

//==============================================================================
// Type Conversion Utilities

/**
 * @brief Variant type for generic parameter values
 */
using ParameterValue = std::variant<int, float, bool, juce::String>;

/**
 * @brief Convert variant to JSON
 */
inline json variantToJson(const ParameterValue& var)
{
    return std::visit([](auto&& arg) -> json {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, juce::String>)
            return arg.toStdString();
        else
            return arg;
    }, var);
}

/**
 * @brief Extract value from variant with type checking
 */
template <typename T>
inline T extractVariant(const ParameterValue& value)
{
    if (auto ptr = std::get_if<T>(&value))
        return *ptr;
    
    throw std::bad_variant_access();
}

/**
 * @brief Safe string to float conversion
 */
inline float safeStringToFloat(const juce::String& str, float defaultValue = 0.0f)
{
    if (str.isEmpty())
        return defaultValue;
    
    try
    {
        return str.getFloatValue();
    }
    catch (...)
    {
        return defaultValue;
    }
}

/**
 * @brief Safe string to int conversion
 */
inline int safeStringToInt(const juce::String& str, int defaultValue = 0)
{
    if (str.isEmpty())
        return defaultValue;
    
    try
    {
        return str.getIntValue();
    }
    catch (...)
    {
        return defaultValue;
    }
}

//==============================================================================
// String Utilities

/**
 * @brief Extract name and index from strings like "layer_1"
 */
inline std::pair<juce::String, int> extractNameAndIndex(const juce::String& input)
{
    int lastUnderscore = input.lastIndexOfChar('_');
    
    if (lastUnderscore < 0 || lastUnderscore == input.length() - 1)
        return { input, -1 };
    
    juce::String indexPart = input.substring(lastUnderscore + 1);
    
    // Check if all characters after underscore are digits
    if (!indexPart.containsOnly("0123456789"))
        return { input, -1 };
    
    juce::String name = input.substring(0, lastUnderscore);
    int index = indexPart.getIntValue();
    
    return { name, index };
}

/**
 * @brief Create a unique identifier from name and index
 */
inline juce::String createIdentifier(const juce::String& name, int index)
{
    return name + "_" + juce::String(index);
}

/**
 * @brief Generate a unique ID using UUID-like format
 */
inline juce::String generateUniqueId()
{
    juce::Uuid uuid;
    return uuid.toString();
}

/**
 * @brief Generate a shorter unique ID based on timestamp + random
 */
inline juce::String generateShortId()
{
    auto now = juce::Time::currentTimeMillis();
    auto random = juce::Random::getSystemRandom().nextInt();
    return juce::String::toHexString(now) + juce::String::toHexString(random).substring(0, 4);
}

//==============================================================================
// Math Utilities

/**
 * @brief Calculate range between two values
 */
template <typename T>
inline T calculateRange(T min, T max)
{
    return std::abs(max - min);
}

/**
 * @brief Linear interpolation
 */
template <typename T>
inline T lerp(T a, T b, float t)
{
    return a + (b - a) * t;
}

/**
 * @brief Inverse linear interpolation (get t from value)
 */
template <typename T>
inline float inverseLerp(T a, T b, T value)
{
    if (std::abs(b - a) < std::numeric_limits<T>::epsilon())
        return 0.0f;
    
    return static_cast<float>(value - a) / static_cast<float>(b - a);
}

/**
 * @brief Remap value from one range to another
 */
template <typename T>
inline T remap(T value, T fromMin, T fromMax, T toMin, T toMax)
{
    float t = inverseLerp(fromMin, fromMax, value);
    return lerp(toMin, toMax, t);
}

/**
 * @brief Apply skew factor to normalized value
 */
inline float applySkew(float normalizedValue, float skewFactor)
{
    if (std::abs(skewFactor - 1.0f) < 0.001f)
        return normalizedValue;
    
    return std::pow(normalizedValue, skewFactor);
}

/**
 * @brief Remove skew factor from normalized value
 */
inline float removeSkew(float skewedValue, float skewFactor)
{
    if (std::abs(skewFactor - 1.0f) < 0.001f)
        return skewedValue;
    
    return std::pow(skewedValue, 1.0f / skewFactor);
}

//==============================================================================
// Color Utilities

/**
 * @brief Blend two colors
 */
inline juce::Colour blendColours(juce::Colour a, juce::Colour b, float ratio)
{
    return a.interpolatedWith(b, ratio);
}

/**
 * @brief Get contrasting text color for a background
 */
inline juce::Colour getContrastingColour(juce::Colour background)
{
    float luminance = background.getPerceivedBrightness();
    return luminance > 0.5f ? juce::Colours::black : juce::Colours::white;
}

/**
 * @brief Parse color from hex string
 */
inline juce::Colour parseHexColour(const juce::String& hex)
{
    juce::String cleanHex = hex.trimCharactersAtStart("#");
    
    if (cleanHex.length() == 6)
        cleanHex = "ff" + cleanHex;  // Add alpha
    
    return juce::Colour(static_cast<juce::uint32>(cleanHex.getHexValue32()));
}

/**
 * @brief Convert color to hex string
 */
inline juce::String colourToHex(juce::Colour colour, bool includeAlpha = true)
{
    if (includeAlpha)
        return "#" + colour.toDisplayString(true);
    else
        return "#" + colour.toDisplayString(false);
}

//==============================================================================
// Debug Utilities

/**
 * @brief Get memory address as string (for debugging)
 */
template <typename T>
inline juce::String getAddressAsString(const T& obj)
{
    std::ostringstream oss;
    oss << "0x" << std::hex << reinterpret_cast<std::uintptr_t>(&obj);
    return juce::String(oss.str());
}

/**
 * @brief Get object size in KB
 */
template <typename T>
inline float getObjectSizeKB(const T& obj)
{
    return static_cast<float>(sizeof(obj)) / 1024.0f;
}

/**
 * @brief Log object info for debugging
 */
template <typename T>
inline void logObjectInfo(const T& obj, const juce::String& name)
{
    DBG(name << " @ " << getAddressAsString(obj) << " - " << getObjectSizeKB(obj) << " KB");
}

} // namespace Utilities

} // namespace zaplab
