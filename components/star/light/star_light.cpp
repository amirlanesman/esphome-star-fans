#include "star_light.h"
#include "esphome/core/log.h"

namespace esphome
{
  namespace star
  {

    static const char *const TAG = "star.light";

    light::LightTraits StarLightComponent::get_traits()
    {
      auto traits = light::LightTraits();
      traits.set_supported_color_modes({light::ColorMode::ON_OFF});
      return traits;
    }

    void StarLightComponent::write_state(light::LightState *state)
    {
      if (!this->next_write_)
      {
        ESP_LOGV(TAG, "Next write should be skipped, skipping");
        this->next_write_ = true; // Reset the flag for the next write
        return; // Skip if next write is not allowed
      }
      this->star_->send_command(STAR_LIGHT_TOGGLE);
    }

    bool StarLightComponent::on_receive(uint16_t code)
    {
      ESP_LOGV(TAG, "Received code: %d 0x%02X (bin=0b%s)", code, code, std::bitset<6>(code).to_string().c_str());
      if (code == STAR_LIGHT_TOGGLE)
      {
        ESP_LOGD(TAG, "Toggling light state");
        this->next_write_ = false; // Set the flag to indicate that the next write should be processed
        this->state_->toggle().perform();
        return true;
      }
      return false;
    }

    void StarLightComponent::dump_config()
    {
      ESP_LOGCONFIG(TAG, "Star Light");
    }

    void StarLightComponent::toggle_state() {
        ESP_LOGD(TAG, "Toggling light state manually");
        this->next_write_ = false; // Set the flag to indicate that the next write should not be processed
        this->state_->toggle().perform();
    }

  } // namespace star
} // namespace esphome