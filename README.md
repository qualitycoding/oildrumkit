# Oil Drum Kit

**Oil Drum Kit** is a portable, modal-synthesis drum plugin built with C++ and JUCE. Rather than modeling a traditional harmonic drum membrane, this engine specifically simulates the raw, harsh, and industrial resonance of struck sheet steel and oil barrels.

---

## Sound Model & Features

The audio engine treats every drum as a bank of physical partials, relying on inharmonic ratios to produce a distinct metallic clank.

*   **Modal Synthesis Engine:** Utilizes a mixture of sine oscillators for shell resonance and resonant bandpassed noise for metallic rattling and hiss.
*   **15 Distinct Instruments:** Provides a full General MIDI-mapped layout (Bass, Snares, Toms, Splash, Ride, Hi-Hats, Cowbell, Rimshot) reimagined as heavy metal impacts.
*   **Dynamic Pitch Tuning:** Every drum features a tunable fundamental frequency. The UI sliders can be tweaked live, resulting in a smooth, analog-style pitch glide while the note rings.
*   **Interchangeable Hammers:** Simulates physical striking materials (Metal, Wood, Rubber) by dynamically adjusting the noise-based attack transient and lowpass filtering on the initial click.
*   **Responsive UI:** Features a dark, industrial interface with hazard-stripe accents and dynamic visual feedback that flashes when drums are triggered.

---

## Technical Architecture

*   **Lightweight Engine:** The core `DrumEngine.h` is completely portable and header-only, requiring no external dependencies beyond standard C++ (`<cmath>`, `<array>`, `<cstdint>`).
*   **Real-Time Safe:** The engine is strictly allocation-free after initialization, making it completely safe to run inside a real-time audio thread.
*   **Framework:** The VST3 and Standalone application wrappers are powered by the JUCE framework.

---

## Getting Started

To compile the plugin from source, you will need a C++17 compatible compiler and CMake.

*   Clone the repository containing the source code.
*   Configure the project using CMake, ensuring JUCE is properly linked or included via `add_subdirectory`.
*   Build the VST3 or Standalone target using your preferred IDE or command-line build tool.