#pragma once

#include "../star.h"
#include "esphome/components/fan/fan.h"
#include "esphome/core/component.h"
#include <bitset>

namespace esphome
{
  namespace star
  {
    const uint8_t STAR_FAN_SPEED_COUNT = 6; // Number of fan speeds supported by the Star fan
    const uint8_t STAR_FAN_CHANGE_DIRECTION = 0b111011;
    const uint8_t STAR_FAN_SPEED_OFF = 0b111101;
    const uint8_t STAR_FAN_SPEED_1 = 0b110111;
    const uint8_t STAR_FAN_SPEED_2 = 0b110101;
    const uint8_t STAR_FAN_SPEED_3 = 0b101111;
    const uint8_t STAR_FAN_SPEED_4 = 0b100111;
    const uint8_t STAR_FAN_SPEED_5 = 0b011101;
    const uint8_t STAR_FAN_SPEED_6 = 0b011111;
    const uint8_t STAR_FAN_SPEEDS[] = {
      STAR_FAN_SPEED_OFF,
      STAR_FAN_SPEED_1,
      STAR_FAN_SPEED_2,
      STAR_FAN_SPEED_3,
      STAR_FAN_SPEED_4,
      STAR_FAN_SPEED_5,
      STAR_FAN_SPEED_6,
    };

    class StarFanComponent : public fan::Fan,
                             public StarTransmitterAndReceiverListener,
                             public Component

    {
    public:
      void setup() override;
      fan::FanTraits get_traits() override { return fan::FanTraits(false, true, true, STAR_FAN_SPEED_COUNT); }

      void dump_config() override;

    protected:
      void control(const fan::FanCall &call) override;
      /// Handle received IR Buffer
      bool on_receive(uint16_t code) override;

      int8_t get_speed(int8_t code);
      int8_t get_speed_code(int8_t code);

      bool next_write_{false}; // Flag to indicate if the next write should be processed
    };
  } // namespace star
} // namespace esphome