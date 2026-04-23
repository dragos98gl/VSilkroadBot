# VSilkroadBot

A state-of-the-art YOLO-based bot for Silkroad Online, designed specifically for learning and educational purposes.

## Overview

VSilkroadBot (Visual Silkroad Bot) is a computer vision-based bot that uses YOLO (You Only Look Once) object detection to automate gameplay in Silkroad Online. This project is dedicated to learning and experimentation, not for widespread player usage.

## Features

- **YOLO-based Object Detection**: Uses state-of-the-art YOLO models for accurate mob detection
- **Computer Vision Navigation**: Visual-based character movement and navigation
- **Health Bar Monitoring**: Real-time health bar detection and tracking
- **Skill Management**: Automated skill usage based on mob types
- **Training Interface**: Complete training pipeline for custom models
- **Attacking area**: Based on player position recognition with configurable attacking radius; automatically switches spots when no mobs are detected; possibility to create a trajectory from town to the attacking area
- **Nearby player detection**: Detects and warns about nearby players to avoid competition or PvP encounters
- **Mobs timeout in case of getting stuck**: Automatically handles mobs that stop responding or get stuck during combat
- **Qt-based GUI**: User-friendly interface for configuration and monitoring
- **CUDA Acceleration**: GPU-accelerated inference for real-time performance

## Architecture

The bot is built using:
- **C++17** with Qt6 for the main application
- **OpenCV** for computer vision operations
- **Interception** for low-level keyboard and mouse input simulation
- **YOLOv8** for object detection
- **CUDA** for GPU acceleration
- **Python** for training and model management

## Demo

![Demo](assets/demo/demo.gif)

## Project Structure

```
VSilkroadBot/
├── assets/                   # Game assets, models, and configuration
│   ├── models/               # Trained YOLO models
│   ├── map/                  # Navigation map
│   ├── digitsNN_train/       # Digit recognition training data
│   ├── char_nn.onnx          # Character recognition neural network model
│   └── settings.json         # Bot configuration
├── src/                      # C++ source code
│   ├── Bot/                  # Core bot logic
│   ├── UiForms/              # Qt user interface
│   ├── CustomUiObj/          # Custom UI components
│   └── IO/                   # Input/output handling
├── include/                  # Header files
├── python_venvs/             # Python environments for training
│   ├── yolotrain/            # YOLO training environment
│   └── digitsNN/             # Digit recognition training environment
├── third_party/              # External dependencies
└── build/                    # Build artifacts
```

## Installation

### Prerequisites

- Windows 10/11
- CMake 3.16
- Qt 6.10.1 (MSVC 2022 x64)
- CUDA 12.4
- Visual Studio 2022

### Build Instructions

One click installer: install.bat which runs:
1. **1/5** deps.bat - installs third-party dependencies

2. **2/5** Download and install Interception drivers

3. **3/5** Instalation of Python environments and dependencies

4. **4/5** Preparation of digits recognition network

5. **5/5** Build the project

The executable can be found at build/Release/VSilkroadBot.exe

## Usage

### Main Window
![MainWindow](assets/demo/MainWindow.png)
1. **Configure the Silkroad Online window name**
2. **Model training**
![ModelTraining](assets/demo/TrainingWindow.png)
3. **Model loading**
4. **Modify existing model**
   - Available for tained models
   - Allows assisted detections based on the current model

### Bot Options
![BotSettings](assets/demo/BotOptions.png)

1. **Navigation Settings**
   - Define attacking areas, no mobs spots, trajectory
   
![NavigationSettings](assets/demo/NavigationSettings.png)

2. **Bot Key Points Settings**
   - Mobs health bar pixel data and position
   - Mobs type pixel data and position
   - Skills & Buffs pixel data and position

![KeypointsSettings](assets/demo/KeyPointsSettings.png)

3. **Bot Combat Settings**
   - Bot reaction timing
   - Combat behaviour
   - Mobs timeout configuration

![CombatSettings](assets/demo/CombatSettings.png)

### Bot Window
   - Mini-map preview
   - Detections preview
   - Only Attacking mode (uses only the attacking skills if the mob's health bar is detected)
![BotWindow](assets/demo/BotWindow.png)

## Important Notes

⚠️ **Educational Purpose Only**: This bot is designed for learning computer vision and game automation concepts. It is not intended for widespread use in live gameplay.

⚠️ **No Executable Distribution**: This project does not include pre-built executables. Users must build from source to understand the implementation.

⚠️ **Game Integrity**: Always respect game terms of service and fair play principles when experimenting with automation tools.

## Contributing

This project welcomes contributions focused on:
- Computer vision improvements
- Training pipeline enhancements
- Documentation updates
- Educational content

## License

This project is for educational purposes only. Use responsibly and in accordance with applicable game terms of service.

## Dependencies

- **Qt6**: GUI framework
- **OpenCV**: Computer vision library
- **Interception**: Low-level input device library for keyboard/mouse simulation
- **YOLOv8**: Object detection models
- **CUDA**: GPU acceleration
- **TensorFlow/Keras**: Model training
- **Ultralytics**: YOLO implementation

## Support

For questions about the implementation or educational use cases, please open an issue with the "question" label.

---

**Disclaimer**: This tool is provided for educational and research purposes. Users are responsible for complying with all applicable laws, regulations, and game terms of service.