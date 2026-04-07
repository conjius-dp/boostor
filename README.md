# GainKnob

A simple audio plugin with a single gain knob ranging from -inf (silence) to +24 dB. The gain parameter is fully automatable, so DAWs can map it to any MIDI controller.

**Formats:** VST3, AU, Standalone

## Dependencies

| Dependency | Version | Notes |
|---|---|---|
| [JUCE](https://github.com/juce-framework/JUCE) | **8.0.12** | Must be cloned into the project root as `JUCE/` |
| CMake | **≥ 3.22** | Tested with 3.29.2 |
| C++ compiler | **C++17** | Tested with Apple Clang 17.0.0 |

## Setup

1. Clone the repository:

```bash
git clone https://github.com/conjius/gain-knob.git
cd gain-knob
```

2. Clone JUCE 8.0.12 into the project root:

```bash
git clone --branch 8.0.12 --depth 1 https://github.com/juce-framework/JUCE.git
```

3. Configure and build:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The built plugins are in `build/GainKnob_artefacts/Release/`:

- `VST3/GainKnob.vst3`
- `AU/GainKnob.component`
- `Standalone/GainKnob.app`

On macOS, `COPY_PLUGIN_AFTER_BUILD` is enabled so the VST3 and AU are automatically installed to `~/Library/Audio/Plug-Ins/`.

## Running Tests

```bash
cmake --build build --target GainKnobTests
./build/GainKnobTests_artefacts/Release/GainKnobTests
```
