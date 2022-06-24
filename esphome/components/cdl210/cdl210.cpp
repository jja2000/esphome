#include "cdl210.h"
#include "esphome/core/log.h"

namespace esphome {
namespace cdl210 {

static const char *const TAG = "cdl210";

void CDL210Component::update() {
  this->write_byte(':');
  this->write_byte('\n');
}

void CDL210Component::loop() {
  static bool started = false;
  static uint8_t code = 0;
  static uint32_t last_time = 0;

  while (this->available()) {
    last_time = millis();
    uint8_t c;
    this->read_byte(&c);
    if (c == ' ') {
      this->rx_message_.clear();
      started = true;
      continue;
    }
    if (!started) {
      code = c;
      continue;
    }
    this->rx_message_.push_back(c);
  }
  if (started && (last_time - millis() > 32)) {
    started = false;
    if (code == ':') {
      float temp, hum;
      int co2;
      std::string data(this->rx_message_.begin(), this->rx_message_.end());
      int ret = sscanf(data.c_str(), "T%fC:C%dppm:H%f%%", &temp, &co2, &hum);
      if (ret < 3)
        ESP_LOGE(TAG, "error parsing data");
      if ((ret >= 1) && (this->temperature_sensor_ != nullptr))
        this->temperature_sensor_->publish_state(temp);
      if ((ret >= 2) && (this->co2_sensor_ != nullptr))
        this->co2_sensor_->publish_state(co2);
      if ((ret >= 3) && (this->humidity_sensor_ != nullptr))
        this->humidity_sensor_->publish_state(hum);
    }
    code = 0;
  }
}

void CDL210Component::dump_config() {
  ESP_LOGCONFIG(TAG, "CDL210:");
  LOG_SENSOR("  ", "CO2", this->co2_sensor_);
  LOG_SENSOR("  ", "Temperature", this->temperature_sensor_);
  LOG_SENSOR("  ", "Humidity", this->humidity_sensor_);
  this->check_uart_settings(9600);
}

}  // namespace cdl210
}  // namespace esphome
