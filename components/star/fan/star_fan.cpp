#include "star_fan.h"
#include "esphome/core/log.h"

namespace esphome
{
  namespace star
  {

    static const char *const TAG = "star.fan";

    void StarFanComponent::setup() {
      auto restore = this->restore_state_();
      if (restore.has_value()) {
        restore->apply(*this);
        this->publish_state();
        ESP_LOGD(TAG, "Restored state: %s, speed: %d, direction: %s",
                 ONOFF(this->state), this->speed, this->direction == fan::FanDirection::FORWARD ? "FORWARD" : "REVERSE");
      }
    }

    int8_t StarFanComponent::get_speed(int8_t code)
    {
      switch (code)
      {
      case STAR_FAN_SPEED_OFF:
        return 0;
      case STAR_FAN_SPEED_1:
        return 1;
      case STAR_FAN_SPEED_2:
        return 2;
      case STAR_FAN_SPEED_3:
        return 3;
      case STAR_FAN_SPEED_4:
        return 4;
      case STAR_FAN_SPEED_5:
        return 5;
      case STAR_FAN_SPEED_6:
        return 6;
      default:
        return -1; // Invalid speed
      }
    }

    bool StarFanComponent::on_receive(uint16_t code)
    {
      ESP_LOGV(TAG, "Received code: %d 0x%02X (bin=0b%s)", code, code, std::bitset<6>(code).to_string().c_str());
      if (code == STAR_FAN_CHANGE_DIRECTION)
      {
        ESP_LOGD(TAG, "Changing fan direction");
        this->direction = this->direction == fan::FanDirection::FORWARD ? fan::FanDirection::REVERSE : fan::FanDirection::FORWARD;
        this->publish_state();
        return true; // Indicate that the code was processed
      }
      int8_t speed = this->get_speed(code);
      if (speed >= 0)
      {
        ESP_LOGV(TAG, "Received fan speed code: %d 0x%02X (bin=0b%s) = speed%d", code, code, std::bitset<6>(code).to_string().c_str(), speed);
        if (speed == 0)
        {
          ESP_LOGD(TAG, "Turning fan off");
          this->state = false;
        }
        else
        {
          ESP_LOGD(TAG, "Setting fan speed to %d", speed);
          this->state = true;
          this->speed = speed;
        }
        this->publish_state();
        return true; // Indicate that the code was processed
      }
      return false; // Indicate that the code was not processed
    }

    void StarFanComponent::control(const fan::FanCall &call)
    {
      ESP_LOGV(TAG, "Received StarFanComponent::control");
      bool did_change = false;
      int8_t speed = -1;
      if (call.get_state().has_value() || call.get_speed().has_value()) {
        bool requested_state = call.get_state().value_or(this->state);
        ESP_LOGV(TAG, "Setting fan state from %s to %s", ONOFF(this->state), ONOFF(requested_state));
        speed = call.get_speed().value_or(this->speed);
        ESP_LOGV(TAG, "Setting fan speed to %d (currently %d)", speed, this->speed);
        if (requested_state == true) {
          speed = speed != 0 ? speed : 3; // Default to speed 3 if not specified
          ESP_LOGV(TAG, "Sending command to set fan speed to %d", speed);
          this->star_->send_command(STAR_FAN_SPEEDS[speed]);
        } else {
          ESP_LOGV(TAG, "Turning fan off");
          this->star_->send_command(STAR_FAN_SPEED_OFF);
        }
        this->speed = speed;
        this->state = requested_state;
      }
      if (call.get_direction().has_value() && call.get_direction().value() != this->direction){
        ESP_LOGV(TAG, "Setting fan direction to %s", call.get_direction().value() == fan::FanDirection::FORWARD ? "FORWARD" : "REVERSE");
        this->star_->send_command(STAR_FAN_CHANGE_DIRECTION);
        this->direction = call.get_direction().value();
      }
      
      this->publish_state();
    }

    void StarFanComponent::dump_config()
    {
      ESP_LOGCONFIG(TAG, "Star Fan");
      // ESP_LOGCONFIG(tag, "  Controller '%s'", this->get_parent()->get_name().c_str());
      // ESP_LOGCONFIG(tag, "  Star '%s'", this->star_->get_n
    }

  } // namespace star
} // namespace esphome