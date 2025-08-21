# 🐱 DigiCat - Virtual Pet for ESP32

A charming virtual cat companion that lives on your ESP32 with touchscreen display! Watch your pixel art cat grow, play, and evolve while you care for it.


## ✨ Features

- 🎨 **Beautiful Pixel Art Cat** - Detailed animations with expressions, blinking, and movement
- 📱 **Touch Interface** - Intuitive touch controls for feeding, playing, and petting
- 🎭 **Dynamic Expressions** - Cat's mood changes based on happiness, hunger, and health
- 🏆 **Evolution System** - Watch your kitten grow into an adult cat
- 😴 **Sleep Mode** - Cat sleeps when health is low
- ❤️ **Interactive Petting** - Touch the cat directly for bonus happiness
- 🎮 **Chill Gameplay** - Relaxing experience with slow stat changes
- 🎪 **Smooth Animations** - Jumping, eating, and idle movements

## 🛠️ Hardware Required

- **ESP32 CYD-R (2432S024R)** - Cheap Yellow Display with **resistive** touchscreen
  - 240x320 ST7789 Display
  - XPT2046 Resistive Touch Controller
  - ESP32 microcontroller
- **microSD card** (optional) - for future features

### ⚠️ Important: CYD Variants
This project is specifically designed for the **CYD-R** (resistive touch) model. If you have the capacitive version (CYD-C), you'll need to modify the touch code.

## 🚀 Quick Start with Web Flasher

The easiest way to get DigiCat running:

1. **[📱 Flash DigiCat Now](https://aquascape123.github.io/digicat/)**
2. Connect your CYD-R to your computer via USB
3. Click "Connect" and select your device
4. Click "Install DigiCat"
5. Enjoy your new virtual pet! 🎉

## 🔧 Manual Installation

### Prerequisites
- Arduino IDE 2.0+
- ESP32 Board Package
- Required Libraries (see below)

### Libraries Required
Install these through Arduino Library Manager:
```
TFT_eSPI by Bodmer
XPT2046_Touchscreen by Paul Stoffregen
```

### Setup Steps
1. **Clone this repository**
   ```bash
   git clone https://github.com/aquascape123/digicat.git
   cd digicat
   ```

2. **Configure TFT_eSPI**
   - Copy `User_Setup.h` to your TFT_eSPI library folder
   - Or replace the existing one in: `Arduino/libraries/TFT_eSPI/User_Setup.h`

3. **Upload the code**
   - Open `DigiCat.ino` in Arduino IDE
   - Select board: "ESP32 Dev Module"
   - Select correct COM port
   - Upload!

## 🎮 How to Play

### Controls
- **🍖 FEED Button** - Give food to your cat (reduces hunger)
- **🎾 PLAY Button** - Play with your cat (increases happiness)  
- **🔄 RESET Button** - Reset your cat back to kitten stage
- **👋 Pet the Cat** - Touch the cat directly for bonus happiness and hearts!

### Stats
- **🍽️ Hunger** - Increases slowly over time (feed to reduce)
- **😸 Happiness** - Decreases without attention (play to increase)
- **❤️ Health** - Depends on hunger and happiness levels
- **📅 Age** - Your cat evolves: Kitten → Young Cat → Adult Cat

### Tips
- 🏃‍♂️ Playing makes your cat hungry, so balance food and fun!
- 😴 If health gets too low, your cat will sleep until it recovers
- ❤️ Petting gives happiness without making the cat hungry
- 🎯 DigiCat is designed to be chill - stats change slowly

## 📁 Project Structure

```
digicat/
├── DigiCat.ino           # Main Arduino sketch
├── User_Setup.h          # TFT_eSPI configuration
├── web_flasher/          # Web flasher files
│   ├── index.html
│   ├── manifest.json
│   └── firmware.bin
├── images/               # Screenshots and demos
├── 3d_case/             # 3D printable case files
│   └── DigiCat_Case.stl
├── schematic/           # Wiring diagrams
└── README.md
```

## 🎨 Customization

### Changing Cat Colors
Edit these constants in the code:
```cpp
const uint16_t CAT_BODY = TFT_ORANGE;    // Cat body color
const uint16_t CAT_STRIPE = TFT_BLACK;   // Stripe color  
const uint16_t CAT_EYES = TFT_GREEN;     // Eye color
const uint16_t CAT_NOSE = TFT_PINK;      // Nose color
```

### Adjusting Game Speed
Modify timing constants:
```cpp
// Update frequency (30 seconds = chill mode)
if (currentTime - pet.lastUpdate > 30000) 

// Hunger rate (5 minutes between increases)
if (millis() - pet.lastFeed > 300000)
```

## 🐛 Troubleshooting

### Touch Not Working
- Verify you have the CYD-R (resistive) model
- Check wiring connections
- Try adjusting touch calibration values in the code

### Display Issues
- Ensure User_Setup.h is properly configured
- Check display rotation setting: `tft.setRotation(2)`
- Verify pin definitions match your board

### Cat Behavior Issues
- Stats change slowly by design (every 30 seconds)
- Try feeding/playing and wait for the next update cycle
- Use Serial Monitor to debug stat values

## 🤝 Contributing

We love contributions! Here's how you can help:

- 🐛 **Bug Reports** - Found an issue? Open an issue!
- ✨ **Feature Requests** - Ideas for new cat behaviors?
- 🎨 **Art Improvements** - Better pixel art or animations
- 📚 **Documentation** - Help improve this README
- 🌍 **Translations** - Localize DigiCat for other languages

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Credits

- **Pixel Art Design** - Custom cat sprites and animations
- **TFT_eSPI Library** - Bodmer's excellent display library
- **XPT2046 Library** - Paul Stoffregen's touch library
- **CYD Community** - For hardware documentation and support
- **ESP32 Arduino Core** - Espressif's Arduino framework
