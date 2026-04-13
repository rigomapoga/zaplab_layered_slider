# Changelog

All notable changes to this project will be documented in this file.


## [1.1.0] - 2025-12-14

### Changed
- 

## [1.0.0] - 2025-12-14

### Added
- Initial release of zaplab_layered_slider module
- Layer base class with transform and value binding support
- ShapeLayer for rectangles, ellipses, arcs, polygons, and more
- TextLayer for animated text rendering
- ImageLayer for image-based skins
- LayerStack for managing multiple layers
- LayeredSlider main component

### Preset Loading (from SliderDesigner app exports)
- `loadPresetFile(path)` - Load .sliderdesign files directly
- `loadFromJson(json)` - Load from JSON object (Export as JSON String)
- `loadFromBase64(base64)` - Load from Base64 string (Export as Embedded Data) - **recommended for production**
- `loadFromMemory(name)` - Load from BinaryData resources
- C++ code export support (legacy, deprecated - use JSON or Base64 instead)


