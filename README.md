# IOT-based-smart-billing-trolly-using-streamlit-Dashboard
An ESP32 based RFID system that logs scanned tags and user selected quantities to Firebase. Includes OLED display, IR sensor start, rotary encoder input, and buzzer alert system.
# Smart RFID Quantity Logging System 🚀

This project is an **ESP32-based RFID quantity logging system** that combines:


**RFID Tag Reading** (via Serial2)
**Rotary Encoder** to select quantity
**IR Sensor** to start the system
**OLED Display (SH1106)** for status updates
**Buzzer Alert System**
**WiFi Connectivity** to send data to **Firebase Realtime Database**

---

## 🔧 Hardware Used

- ESP32 Dev Module  
- Rotary Encoder (DT, CLK, SW)  
- IR Sensor  
- OLED Display (SH1106, I2C 0x3C)  
- Buzzer  
- RFID Reader (connected to Serial2 pins RXD2 = GPIO16, TXD2 = GPIO17)

---

## 🔌 Pin Configuration

| Component       | Pin(s)         |
|----------------|----------------|
| RFID Reader     | RXD2 = 16, TXD2 = 17 |
| Rotary Encoder  | DT = GPIO4, CLK = GPIO5, SW = GPIO15 |
| IR Sensor       | GPIO26         |
| Buzzer          | GPIO13         |
| OLED (I2C)      | SDA/SCL via Wire |
| WiFi            | (Credentials are in the code) |

---

## 📶 Firebase Setup

This project sends tag and quantity data to [Firebase Realtime Database](https://firebase.google.com/products/realtime-database):

```json
{
  "tag": "RFID_TAG_HERE",
  "qty": 5
}
