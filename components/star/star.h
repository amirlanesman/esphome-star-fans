#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/remote_base/remote_base.h"
#include "esphome/components/light/light_state.h"

namespace esphome
{
  namespace star
  {

    const uint8_t STAR_SIGNAL_LENGTH = 26;
    const uint8_t STAR_SIGNAL_BITS = 12;
    const uint8_t STAR_SIGNAL_BYTES = 2;
    const uint32_t STAR_SIGNAL_BASE_TIME = 330;
    const uint8_t STAR_SIGNAL_START_TIMES = 31;
    const uint8_t STAR_SIGNAL_ONE_MARK = 2;
    const uint8_t STAR_SIGNAL_ONE_SPACE = 1;
    const uint8_t STAR_SIGNAL_ZERO_MARK = 1;
    const uint8_t STAR_SIGNAL_ZERO_SPACE = 2;
    const uint32_t STAR_FREQUENCY = 38000;

    class StarReceiverListener
    {
    public:
      virtual bool on_receive(uint16_t code) = 0;
    };

    class StarComponent : public Component,
                          public remote_base::RemoteReceiverListener,
                          public remote_base::RemoteTransmittable
    {
    public:
      /// Enable WiFi function bits (some models)
      void set_send_times(int32_t send_times) { this->send_times_ = send_times; }
      void set_send_wait(int32_t send_wait) { this->send_wait_ = send_wait; }
      void set_remote_id(int8_t remote_id) { this->remote_id_ = remote_id; }
      void set_block_for_transmit(int32_t millis) { this->block_for_transmit_ = millis; }
      void set_notification_light(light::LightState *notification_light) { this->notification_light_ = notification_light; }
      void register_listener(StarReceiverListener *listener) { this->listeners_.push_back(listener); }

      void send_command(const uint8_t command);

      void dump_config() override;

    protected:
      ;
      /// Handle received IR Buffer
      bool on_receive(remote_base::RemoteReceiveData data) override;

      /// Parse received IR data into climate state
      bool parse_state_frame_(const uint16_t signal);

      uint16_t get_signal_to_send(const uint8_t code);
      void transmit_signal(const uint16_t signal);

      void call_listeners_(uint16_t code);

      void flash_notification_light();

      std::vector<StarReceiverListener *> listeners_;
      int32_t send_times_{10};
      int8_t remote_id_{0};
      int32_t send_wait_{10000};
      int32_t block_for_transmit_{700}; // milliseconds to block receiving after sending a command
      int32_t last_transmit_time_{};
      int32_t last_receive_time_{0};
      int16_t last_receive_signal_{0};
      light::LightState *notification_light_{nullptr};
      int32_t notification_light_flash_length_{1000}; // milliseconds

    private:
      uint32_t get_time(uint32_t times) { return STAR_SIGNAL_BASE_TIME * times; }
    };

    template <typename... Ts>
    class SendCommandAction : public Action<Ts...>
    {
    public:
      SendCommandAction(StarComponent *star) : star_(star) {}
      void set_code(uint8_t code) { this->code_ = code; }
      void play(Ts... x) override { this->star_->send_command(this->code_); }

    protected:
      StarComponent *star_;
      uint8_t code_;
    };

    class StarTransmitterAndReceiverListener : public StarReceiverListener
    {
    public:
      void set_star_output(StarComponent *star)
      {
        this->star_ = star;
        star->register_listener(this);
      }

    protected:
      StarComponent *star_;
    };
  } // namespace gree
} // namespace esphome