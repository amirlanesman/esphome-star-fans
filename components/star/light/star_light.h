#pragma once

#include "../star.h"
#include "esphome/components/light/light_traits.h"
#include "esphome/components/light/light_state.h"
#include "esphome/components/light/light_output.h"
#include <bitset>

namespace esphome
{
  namespace star
  {
    const uint8_t STAR_LIGHT_TOGGLE = 0b111110;

    class StarLightComponent : public Component,
                               public StarTransmitterAndReceiverListener,
                               public light::LightOutput
    {
    public:
      void setup_state(light::LightState *state) override { this->state_ = state; }
      void write_state(light::LightState *state) override;
      light::LightTraits get_traits() override;

      void dump_config() override;

      void toggle_state();

    protected:
      /// Handle received IR Buffer
      bool on_receive(uint16_t code) override;

      light::LightState *state_{nullptr};
      bool next_write_{false}; // Flag to indicate if the next write should be processed
    };

    template <typename... Ts>
    class ToggleStateAction : public Action<Ts...>
    {
    public:
      ToggleStateAction(StarLightComponent *star_light) : star_light_(star_light) {}
      void play(Ts... x) override { this->star_light_->toggle_state(); }

    protected:
      StarLightComponent *star_light_;
    };
  } // namespace star
} // namespace esphome