/*
  ==============================================================================

    PresetMetadata.h
    Created: 13 Dec 2025
    Author:  Paolo Zappalà @ zaplab.dev
    
    Description:
    Metadata for slider design presets including timestamps, versioning,
    and authorship information.

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../libs/nlohmann/json.hpp"
#include <chrono>
#include <iomanip>
#include <sstream>
#include "Identifiers.h"

namespace zaplab
{

using json = nlohmann::json;

//==============================================================================
/**
 * @brief Metadata for slider design presets
 * 
 * Stores information about when a preset was created/modified,
 * what version of the app created it, and author information.
 */
struct PresetMetadata
{
    juce::String lastSaveDate;
    juce::String creationDate;
    juce::String appVersion { "1.0.0" };
    juce::String appName { "SliderDesigner" };
    juce::String formatVersion { Identifiers::currentVersion };
    juce::String author;
    juce::String description;
    juce::StringArray tags;
    
    //==========================================================================
    // Time Utilities
    
    /**
     * @brief Get current time as ISO 8601 string
     */
    static juce::String getCurrentTimeString(const juce::String& format = "%Y-%m-%dT%H:%M:%S")
    {
        auto now = std::chrono::system_clock::now();
        auto nowTimeT = std::chrono::system_clock::to_time_t(now);
        std::tm nowTm = *std::localtime(&nowTimeT);
        
        char buffer[100];
        std::strftime(buffer, sizeof(buffer), format.toRawUTF8(), &nowTm);
        return juce::String(buffer);
    }
    
    /**
     * @brief Convert ISO 8601 timestamp to Unix timestamp
     */
    static juce::int64 toUnixTimestamp(const juce::String& timestamp, 
                                        const juce::String& format = "%Y-%m-%dT%H:%M:%S")
    {
        std::tm tm = {};
        std::istringstream ss(timestamp.toStdString());
        ss >> std::get_time(&tm, format.toRawUTF8());
        
        if (ss.fail())
            return 0;
        
        return static_cast<juce::int64>(std::mktime(&tm));
    }
    
    /**
     * @brief Convert Unix timestamp to readable string
     */
    static juce::String fromUnixTimestamp(juce::int64 unixTimestamp,
                                          const juce::String& format = "%Y-%m-%dT%H:%M:%S")
    {
        std::time_t time = static_cast<std::time_t>(unixTimestamp);
        std::tm tm = *std::localtime(&time);
        
        char buffer[100];
        std::strftime(buffer, sizeof(buffer), format.toRawUTF8(), &tm);
        return juce::String(buffer);
    }
    
    /**
     * @brief Get a human-readable relative time (e.g., "2 hours ago")
     */
    static juce::String getRelativeTimeString(const juce::String& timestamp)
    {
        auto then = toUnixTimestamp(timestamp);
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        auto diff = now - then;
        
        if (diff < 60)
            return "just now";
        if (diff < 3600)
            return juce::String(diff / 60) + " minutes ago";
        if (diff < 86400)
            return juce::String(diff / 3600) + " hours ago";
        if (diff < 604800)
            return juce::String(diff / 86400) + " days ago";
        if (diff < 2592000)
            return juce::String(diff / 604800) + " weeks ago";
        
        return timestamp;
    }
    
    //==========================================================================
    // Serialization
    
    /**
     * @brief Update the last save date to now
     */
    void updateSaveDate()
    {
        lastSaveDate = getCurrentTimeString();
        
        if (creationDate.isEmpty())
            creationDate = lastSaveDate;
    }
    
    /**
     * @brief Serialize to JSON
     */
    [[nodiscard]] json toJson() const
    {
        json j;
        j[Identifiers::format] = Identifiers::formatSliderDesign;
        j[Identifiers::version] = formatVersion.toStdString();
        j[Identifiers::appName] = appName.toStdString();
        j[Identifiers::appVersion] = appVersion.toStdString();
        j[Identifiers::lastSaveDate] = lastSaveDate.toStdString();
        
        if (creationDate.isNotEmpty())
            j["creationDate"] = creationDate.toStdString();
        
        if (author.isNotEmpty())
            j[Identifiers::author] = author.toStdString();
        
        if (description.isNotEmpty())
            j[Identifiers::description] = description.toStdString();
        
        if (!tags.isEmpty())
        {
            json tagsArray = json::array();
            for (const auto& tag : tags)
                tagsArray.push_back(tag.toStdString());
            j[Identifiers::tags] = tagsArray;
        }
        
        return j;
    }
    
    /**
     * @brief Deserialize from JSON
     */
    void fromJson(const json& j)
    {
        if (j.contains(Identifiers::version))
            formatVersion = j[Identifiers::version].get<std::string>();
        
        if (j.contains(Identifiers::appName))
            appName = j[Identifiers::appName].get<std::string>();
        
        if (j.contains(Identifiers::appVersion))
            appVersion = j[Identifiers::appVersion].get<std::string>();
        
        if (j.contains(Identifiers::lastSaveDate))
            lastSaveDate = j[Identifiers::lastSaveDate].get<std::string>();
        
        if (j.contains("creationDate"))
            creationDate = j["creationDate"].get<std::string>();
        
        if (j.contains(Identifiers::author))
            author = j[Identifiers::author].get<std::string>();
        
        if (j.contains(Identifiers::description))
            description = j[Identifiers::description].get<std::string>();
        
        if (j.contains(Identifiers::tags) && j[Identifiers::tags].is_array())
        {
            tags.clear();
            for (const auto& tag : j[Identifiers::tags])
                tags.add(juce::String(tag.get<std::string>()));
        }
    }
    
    //==========================================================================
    // Validation
    
    /**
     * @brief Check if this preset is compatible with current app version
     */
    [[nodiscard]] bool isCompatible() const
    {
        // Simple version check - could be made more sophisticated
        return formatVersion == Identifiers::currentVersion || 
               formatVersion.upToFirstOccurrenceOf(".", false, false) == 
               juce::String(Identifiers::currentVersion).upToFirstOccurrenceOf(".", false, false);
    }
    
    /**
     * @brief Check if metadata is valid
     */
    [[nodiscard]] bool isValid() const
    {
        return formatVersion.isNotEmpty() && lastSaveDate.isNotEmpty();
    }
};

} // namespace zaplab
