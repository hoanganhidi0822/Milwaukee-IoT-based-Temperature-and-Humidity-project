# Milwaukee-IoT-based-Temperature-and-Humidity

**An IoT Solution for Monitoring Environmental Conditions Across Multiple Workspaces**

This project provides a comprehensive IoT-based solution for monitoring temperature and humidity levels in more than 20 rooms within a workspace environment. The system is built on ESP32-S3 T-HMI and DHT30 sensors, with advanced features like OTA updates, password encoding/decoding, watchdog timers, and data storage in a centralized database.

## Overview

Maintaining optimal environmental conditions in workspaces is crucial for productivity, safety, and comfort. The Milwaukee-IoT-based-Temperature-and-Humidity project automates this process by providing real-time monitoring and control over temperature and humidity across multiple rooms. The system leverages IoT technology to simplify deployment and management, including the ability to update all devices with a single click.

### Key Features

- **Multi-Room Monitoring**: Monitor temperature and humidity levels in over 20 rooms simultaneously.
- **OTA (Over-the-Air) Updates**: Update firmware across all devices with a single click, ensuring the system remains up-to-date without physical intervention.
- **Password Security**: Encode and decode passwords securely for device access and configuration.
- **Watchdog Timer**: Integrated watchdog timers to ensure system reliability and automatically reset devices in case of failure.
- **Centralized Database**: Store and retrieve environmental data in a centralized database for analysis, reporting, and historical tracking.
- **ESP32-S3 T-HMI**: Utilizes the ESP32-S3 T-HMI for powerful, interactive control and monitoring.
- **DHT30 Sensors**: Accurate temperature and humidity measurements provided by DHT30 sensors.

## Getting Started

### Prerequisites

Ensure you have the following:

- Python 3.x
- Arduino IDE or PlatformIO
- MQTT Broker (e.g., Mosquitto)
- ESP32-S3 T-HMI boards
- DHT30 sensors
- Database setup (e.g., MySQL, PostgreSQL)
- Necessary libraries: `PubSubClient.h`, `DHT.h`, etc.

### Installation

1. Clone the repository:

   ```bash
   git clone https://github.com/yourusername/Milwaukee-IoT-based-Temperature-and-Humidity.git
   cd Milwaukee-IoT-based-Temperature-and-Humidity
