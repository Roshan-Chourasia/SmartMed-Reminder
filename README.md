# IoT-Based Smart Medicine Reminder System

An intelligent IoT-enabled medicine reminder and dispensing system designed to improve medication adherence, especially for elderly users. The system integrates ESP32-based hardware, a cloud backend, and a web dashboard to enable automated reminders, controlled pill dispensing, and real-time adherence tracking.

---

## 🔗 Live Links

- 🌐 **Web Dashboard:** [LIVE](https://smart-medicine-reminder-frontend.vercel.app/)  
- 🔧 **Backend API:** https://smart-medicine-reminder-backend.onrender.com
- 🎥 **System Demo Video:** (Add YouTube / Drive link here)  

---

## 📌 Problem Statement

Medication non-adherence is a common and critical issue, particularly among elderly patients, leading to health complications and delayed recovery. Forgetting doses, taking medication at incorrect times, or confusion between before/after meal medicines are major contributors to this problem. Traditional reminder systems lack automation, verification, and remote monitoring capabilities.

---

## 💡 Proposed Solution

This project presents an **IoT-Based Smart Medicine Reminder System** that automatically reminds users to take medicines at scheduled times, physically dispenses the correct compartment, verifies pill intake, and logs adherence data to the cloud. Caregivers can remotely configure dose timings and monitor adherence through a web-based dashboard.

---

## 🏗️ System Architecture


---

## ⚙️ Key Features

- Automated medicine reminders based on real-time clock (RTC)
- Support for **before and after meal** medication scheduling
- Physical pill box actuation using servo motors
- IR-based verification of pill removal
- Audible alerts using buzzer
- Real-time logging of taken/missed doses
- Cloud-based backend with REST APIs
- Web dashboard for dose configuration and history monitoring
- Auto-refresh of dose schedules without restarting ESP32

---

## 🧰 Hardware Components

- ESP32 Development Board
- DS3231 Real-Time Clock (RTC) Module
- IR Obstacle Avoidance Sensors (3x)
- SG90 Servo Motors (3x)
- PCA9685 16-Channel Servo Driver
- 16x2 LCD Display (I2C)
- Active Buzzer
- External Power Supply
- Medicine Box with compartments

---

## 💻 Software & Technologies

### Embedded System
- Arduino IDE
- ESP32 WiFi & HTTPClient
- ArduinoJson
- RTClib
- Adafruit PWM Servo Driver Library

### Backend
- Node.js
- Express.js
- MongoDB Atlas
- Mongoose
- RESTful APIs
- Render (Deployment)

### Frontend
- HTML
- CSS
- JavaScript (Vanilla)
- Fetch API
- Vercel (Deployment)

---

## 🌐 Deployment Architecture

| Component  | Platform        |
|------------|-----------------|
| Frontend   | Vercel          |
| Backend    | Render          |
| Database   | MongoDB Atlas   |

This architecture ensures scalability, reliability, and global accessibility.

---

## 🔁 API Endpoints

```http
GET  /api/dose-time
POST /api/dose-time

GET  /api/dose-log?deviceId=DEVICE_001
POST /api/dose-log
```
## ▶️ How It Works (Flow Summary)

- User sets medication times via web dashboard
- Backend stores configuration in MongoDB
- ESP32 periodically fetches updated dose schedules
- RTC triggers reminder at scheduled time
- Buzzer alerts the user
- Servo opens the corresponding medicine compartment
- IR sensor detects pill removal
- Dose status (taken/missed) is logged to cloud
- Web dashboard displays real-time adherence history

## 📈 Future Enhancements

- User authentication and role-based access
- Multi-user and multi-device support
- Mobile application integration
- SMS / WhatsApp notifications
- AI-based medication adherence analytics
- Battery backup and power optimization

## 📜 License

This project is developed for academic and educational purposes.
