# 🎛️ ZapLab Layered Slider

> Create professional sliders and knobs for your JUCE audio plugins using a layer-based design system.

![ZapLab Layered Slider](https://img.shields.io/badge/JUCE-Module-blue) ![License](https://img.shields.io/badge/License-MIT-green) ![C++17](https://img.shields.io/badge/C%2B%2B-17-orange)

<p align="center">
  <img src="docs/demo.gif" alt="LayeredSlider Demo" width="400"/>
</p>

## ✨ Features

- **Layer-based design** - Build complex slider graphics from simple layers
- **Value bindings** - Properties animate automatically based on slider value
- **JSON loading** - Compatible with [SliderDesigner](https://zaplab.dev/sliderdesigner) exports
- **Multiple layer types** - Shapes, text, images
- **Modern C++17** - Clean, efficient code
- **Zero dependencies** - Just JUCE modules

## ⚡ Quick Start

### 1. Add the module to your project

Copy the `zaplab_layered_slider` folder to your JUCE modules directory, then add it in Projucer:

```
Modules → Add a module → Add a module from a specific folder...
```

Or in CMake:
```cmake
juce_add_module(path/to/zaplab_layered_slider)
target_link_libraries(YourPlugin PRIVATE zaplab_layered_slider)
```

### 2. Use LayeredSlider in your plugin

```cpp
#include <zaplab_layered_slider/zaplab_layered_slider.h>

class MyPluginEditor : public juce::AudioProcessorEditor
{
public:
    MyPluginEditor(MyAudioProcessor& p)
        : AudioProcessorEditor(&p)
    {
        // Load a design created with SliderDesigner
        gainSlider.loadFromFile(juce::File("path/to/gain.sliderdesign"));
        gainSlider.setRange(0.0, 1.0);
        addAndMakeVisible(gainSlider);
        
        setSize(400, 300);
    }
    
    void resized() override
    {
        gainSlider.setBounds(150, 100, 100, 100);
    }
    
private:
    zaplab::LayeredSlider gainSlider;
};
```

### 3. Create designs with SliderDesigner

Download [SliderDesigner](https://zaplab.dev/sliderdesigner) to visually create your slider designs:

1. Add layers (shapes, text, images)
2. Configure value bindings (rotation, position, opacity, etc.)
3. Export as `.sliderdesign` file
4. Load in your plugin!

## 📦 Layer Types

### ShapeLayer
Geometric shapes with fill and stroke:
- Rectangle, Ellipse, Arc, Polygon, Star, Triangle, Line

### TextLayer
Text with font customization:
- Custom fonts, size, bold, italic
- Color and alignment

### ImageLayer
Bitmap images:
- Load from file or Base64
- Aspect ratio control

## 🔗 Value Bindings

Bind layer properties to the slider value for automatic animation:

| Property | Description |
|----------|-------------|
| `Rotation` | Rotate around anchor point |
| `RotationAroundPoint` | Orbit around a pivot |
| `PositionX / PositionY` | Move horizontally/vertically |
| `Width / Height` | Scale dimensions |
| `Opacity` | Fade in/out |
| `ArcStart / ArcEnd` | Animate arc angles |

**Easing curves**: Linear, EaseIn, EaseOut, EaseInOut, Stepped

## 📁 Module Structure

```
zaplab_layered_slider/
├── zaplab_layered_slider.h       # Main module header
├── zaplab_layered_slider.cpp     # Module implementation
└── source/
    ├── LayerTypes.h              # Core types and enums
    ├── ValueBinding.h            # Value binding definitions
    ├── Layer.h / Layer.cpp       # Base layer class
    ├── ShapeLayer.h / .cpp       # Shape layer
    ├── TextLayer.h / .cpp        # Text layer
    ├── ImageLayer.h / .cpp       # Image layer
    ├── LayerStack.h / .cpp       # Layer container
    ├── LayeredSlider.h / .cpp    # Main slider component
    └── JsonLoader.h / .cpp       # JSON parsing
```

## 🛠️ Requirements

- JUCE 6.0+ (or JUCE 7.x)
- C++17 or later
- Dependencies: `juce_core`, `juce_graphics`, `juce_gui_basics`

## 📝 License

MIT License - See [LICENSE](LICENSE) for details.

## 🙏 Credits

Created by [ZapLab.dev](https://zaplab.dev)

---

**Questions?** Open an issue or reach out on the [JUCE Forum](https://forum.juce.com/).
