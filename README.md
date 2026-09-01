# Smart-Meter

## IoT-Based Smart Electricity Monitoring & Bill Management System

Mart Meter is an IoT-based electricity monitoring system built around an ESP32.
It collects real-time electrical consumption data and provides a web-based
dashboard for monitoring power usage, energy consumption, estimated bills,
usage limits, and room-wise electricity distribution.

The system is designed to work over local Wi-Fi without requiring a cloud
backend.

---

## 🚀 Features

- ⚡ Real-time power consumption monitoring
- 🔌 Current and power measurement using ACS712 sensors
- 📊 Interactive energy monitoring dashboard
- 💰 Automatic electricity bill estimation
- 📈 Real-time power trend visualization
- 🧮 Room-wise electricity usage and bill calculation
- 📅 Daily and monthly energy tracking
- 🎯 Monthly consumption quota tracking
- 🚨 Usage-limit alerts
- 🌡️ 24-hour consumption heatmap
- 📡 Local ESP32 web server
- 🔄 Automatic dashboard data updates
- 🧪 Demo mode for testing the dashboard without hardware

---

## 🏗️ System Architecture

```text
        ┌─────────────────────┐
        │   Electrical Load   │
        └──────────┬──────────┘
                   │
                   ▼
        ┌─────────────────────┐
        │   ACS712 Sensors    │
        └──────────┬──────────┘
                   │
                   ▼
        ┌─────────────────────┐
        │        ESP32        │
        │                     │
        │  Sensor Processing  │
        │  Power Calculation  │
        │  Energy Tracking    │
        │  Local Web Server   │
        └──────────┬──────────┘
                   │
                Wi-Fi
                   │
                   ▼
        ┌─────────────────────┐
        │   Web Dashboard     │
        │                     │
        │  Power Monitoring   │
        │  Energy Analytics   │
        │  Bill Calculation   │
        │  Usage Alerts       │
        └─────────────────────┘

🔌 Hardware
ESP32
ACS712 Current Sensors
Electrical loads
Wi-Fi network


💻 Technology Stack
Hardware
ESP32
ACS712
Firmware
C++
Arduino Framework
ESPAsyncWebServer
ArduinoJson
Wi-Fi
Frontend
HTML
CSS
JavaScript
Chart.js
SVG

⚙️ How It Works
The ESP32 connects to a local Wi-Fi network.
The ESP32 reads electrical sensor values through its analog input pins.
Multiple samples are averaged to reduce ADC measurement noise.
Current is calculated from the ACS712 sensor readings.
Power consumption is calculated using the measured current.
Energy consumption is accumulated in kWh.
The ESP32 hosts a local HTTP server.
The /data endpoint provides the monitoring data as JSON.
The web dashboard fetches the data from the ESP32.
The dashboard updates power, energy, billing, usage limits, charts,
and alerts in real time.

🌐 API Endpoints
Endpoint	Method	Description
/data	GET	Returns real-time monitoring data as JSON
/status	GET	Displays ESP32 server status
/reset	GET	Resets the current monthly energy cycle


📊 Dashboard

The dashboard provides:

Current power in Watts
Current consumption in Amps
Daily and monthly energy usage
Monthly consumption limits
Remaining energy quota
Estimated electricity bill
Power trend charts
Energy distribution
Usage heatmap
Bill splitting
Usage history
Consumption alerts

🧪 Demo Mode

Mart Meter includes a demo mode that generates simulated power readings,
allowing the dashboard to be tested without an ESP32 or physical sensors.

To use real hardware, enter the ESP32's local IP address in the dashboard
and connect both devices to the same Wi-Fi network.
