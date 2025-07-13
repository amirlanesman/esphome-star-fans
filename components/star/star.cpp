#include "star.h"
#include "esphome/core/log.h"

#include <bitset>

namespace esphome
{
  namespace star
  {

    static const char *const TAG = "star.tranceiver";

    void set_bits(remote_base::RemoteTransmitData &data, uint16_t signal, uint32_t one_mark, uint32_t one_space, uint32_t zero_mark, uint32_t zero_space, uint8_t length)
    {
      // Set data bits
      ESP_LOGVV(TAG, "Setting bits for signal(len=%d): 0x%04X (bin: %s)", length, signal, std::bitset<12>(signal).to_string().c_str());
      for (int8_t j = length - 1; j >= 0; j--)
      {
        bool bit = signal & (1 << j);
        // ESP_LOGD(TAG, "Setting bit %d to %d", j, bit ? 1 : 0);
        data.mark(bit ? one_mark : zero_mark);
        data.space(bit ? one_space : zero_space);
      }
    }

    bool get_bits(remote_base::RemoteReceiveData &data, uint16_t &bits, uint32_t one_mark, uint32_t one_space, uint32_t zero_mark, uint32_t zero_space, uint8_t length)
    {
      for (uint8_t j = 0; j < length; j++)
      {
        bits = bits << 1;
        if (data.expect_item(one_mark, one_space))
        {
          bits |= 0b1;
        }
        else if (data.expect_item(zero_mark, zero_space))
        {
          bits |= 0b0;
        }
        else
        {
          ESP_LOGVV(TAG, "Bit %d failed. stream index=%d", j, data.get_index());
          return false;
        }
      }
      return true;
    }

    bool StarComponent::on_receive(remote_base::RemoteReceiveData data)
    {

      if (millis() - this->last_transmit_time_ < this->block_for_transmit_)
      {
        ESP_LOGV(TAG, "Blocked receive because of recent transmission");
        return false;
      }

      data.reset();
      auto raw = data.get_raw_data();

      // if (raw[0] < 6000 && raw[0] > 14000)
      // {
      //   ESP_LOGVV(TAG, "Raw data does not start with a valid mark: %d", raw[0]);
      //   return false;
      // }

      if (raw.size() < STAR_SIGNAL_LENGTH - 1)
      {
        ESP_LOGVV(TAG, "Received data too short: %zu items", raw.size());
        return false;
      }
      if (raw.size() > STAR_SIGNAL_LENGTH + 1)
      {
        ESP_LOGVV(TAG, "Received data too long: %zu items", raw.size());
        return false;
      }

      ESP_LOGV(TAG, "Raw data has %zu items.", raw.size());
      for (size_t i = 0; i < raw.size(); i++)
      {
        ESP_LOGVV(TAG, "[%03d] %d", i, raw[i]);
      }

      // Check if this looks like a Star signal
      if (!data.expect_item(get_time(STAR_SIGNAL_START_TIMES), get_time(1)) && !data.expect_space(get_time(1)))
      {
        ESP_LOGV(TAG, "Not a valid Star signal, expected start sequence (%d, %d) but got: %d, %d", get_time(STAR_SIGNAL_START_TIMES), get_time(1), raw[0], raw[1]);
        return false;
      }

      uint16_t signal;
      if (!get_bits(
              data,
              signal,
              get_time(STAR_SIGNAL_ONE_MARK),
              get_time(STAR_SIGNAL_ONE_SPACE),
              get_time(STAR_SIGNAL_ZERO_MARK),
              get_time(STAR_SIGNAL_ZERO_SPACE),
              STAR_SIGNAL_BITS))
      {
        ESP_LOGV(TAG, "Signal parsing failed");
        return false;
      }

      ESP_LOGV(TAG, "Received Star frame: 0x%04X (dec:%d) ", signal, signal);

      // Parse the received data
      return this->parse_state_frame_(signal);
    }

    bool StarComponent::parse_state_frame_(const uint16_t signal)
    {
      uint8_t remote_id = (signal >> 7) & 0b11111; // Extract the remote ID from the signal
      uint8_t no_dimmer = (signal >> 6) & 0b1;
      uint8_t code = signal & 0b111111; // Extract the code from the signal

      // const StarProtocol &parsed_frame = *reinterpret_cast<const StarProtocol *>(frame);

      if (signal == this->last_receive_signal_ && millis() - this->last_receive_time_ < 500)
      {
        // If the same signal is received within 500ms, ignore it
        // This prevents processing the same signal multiple times
        // which can happen due to noise or repeated signals
        ESP_LOGV(TAG, "Received repeat signal: %s", (std::bitset<12>(signal)).to_string().c_str());
        return true;
      }

      ESP_LOGD(TAG, "Received frame (binary): %04X (bin: %s)",
               signal,
               (std::bitset<12>(signal)).to_string().c_str());

      ESP_LOGV(TAG, "required remote id: %02X (bin: %s)", this->remote_id_,
               (std::bitset<5>(this->remote_id_)).to_string().c_str());
      ESP_LOGV(TAG, "Received remote id: %02X (bin: %s)", remote_id,
               (std::bitset<5>(remote_id)).to_string().c_str());
      ESP_LOGV(TAG, "Received no_dimmer: %d (bin: %s)", no_dimmer,
               (std::bitset<1>(no_dimmer)).to_string().c_str());
      ESP_LOGV(TAG, "Received code: %d (bin: %s)", code,
               (std::bitset<6>(code)).to_string().c_str());

      if (this->remote_id_ != remote_id)
      {
        ESP_LOGD(TAG, "Remote ID mismatch: expected %02X, got %02X", this->remote_id_, remote_id);
        return false;
      }

      this->last_receive_time_ = millis();
      this->last_receive_signal_ = signal;

      this->call_listeners_(code);

      return true;
    }

    uint16_t StarComponent::get_signal_to_send(const uint8_t code)
    {
      // Build the signal to send
      uint16_t signal = 0;
      signal |= ((this->remote_id_ & 0b11111) << 7); // bits 0-4
      signal |= (0b1 << 6);                          // no dimmer bit
      signal |= (code & 0b111111);                   // bits 6-11

      ESP_LOGD(TAG, "Signal to send: %04X (bin: %s)", signal, std::bitset<12>(signal).to_string().c_str());
      return signal;
    }

    void StarComponent::transmit_signal(const uint16_t signal)
    {
      ESP_LOGD(TAG, "Setting up signal to transmit...");
      auto transmit = this->transmitter_->transmit();
      // transmit.set_send_times(this->send_times_);
      // transmit.set_send_wait(this->send_wait_);
      // transmit.set_send_times(1);
      // transmit.set_send_wait(0);
      auto data = transmit.get_data();
      data->reset();
      data->reserve((STAR_SIGNAL_LENGTH * this->send_times_) + 1); // Reserve space for the signal

      data->set_carrier_frequency(STAR_FREQUENCY);

      ESP_LOGV(TAG, "Setting raw data with send_times_=%d send_wait_=%d...", this->send_times_, this->send_wait_);

      // Header
      ESP_LOGV(TAG, "Setting header 1");
      data->mark(get_time(STAR_SIGNAL_START_TIMES));

      for (int i = 0; i < this->send_times_; i++)
      // for (int i = 0; i < 1; i++)
      {
        ESP_LOGV(TAG, "Setting header 2");
        data->space(get_time(1));

        ESP_LOGV(TAG, "Setting bits");
        set_bits(*data, signal,
                 get_time(STAR_SIGNAL_ONE_MARK),
                 get_time(STAR_SIGNAL_ONE_SPACE),
                 get_time(STAR_SIGNAL_ZERO_MARK),
                 get_time(STAR_SIGNAL_ZERO_SPACE),
                 STAR_SIGNAL_BITS);

        ESP_LOGV(TAG, "Setting wait");
        data->mark(this->send_wait_); // Wait after sending the signal
      }

      auto raw = data->get_data();

      ESP_LOGV(TAG, "Sending raw data - with %zu items", raw.size());
      // size_t print_size = std::min(raw.size(), static_cast<size_t>(60));
      // for (size_t i = 0; i < print_size; i++)
      // {
      //   ESP_LOGD(TAG, "[%03d] %d", i, raw[i]);
      //   delay(10);
      // }
      // delay(100); // Wait for the transmitter to be ready
      this->last_transmit_time_ = millis();
      ESP_LOGD(TAG, "Sending Signal.");
      transmit.perform();
      this->last_transmit_time_ = millis();
    }

    void StarComponent::send_command(const uint8_t code)
    {
      uint16_t signal = this->get_signal_to_send(code);
      this->transmit_signal(signal);
      ESP_LOGD(TAG, "Sent code: %d (signal: %d (0x%04X))", code, signal, signal);
      this->flash_notification_light();
    }

    void StarComponent::call_listeners_(uint16_t code)
    {
      for (auto *listener : this->listeners_)
        listener->on_receive(code);
      this->flash_notification_light();
    }

    void StarComponent::flash_notification_light()
    {
      if (this->notification_light_ != nullptr)
      {
        this->notification_light_->turn_on().set_flash_length(this->notification_light_flash_length_).perform();
      }
    }

    void StarComponent::dump_config()
    {
      ESP_LOGCONFIG(TAG, "Star");
      // ESP_LOGCONFIG(tag, "  Controller '%s'", this->get_parent()->get_name().c_str());
      ESP_LOGCONFIG(TAG, "  Remote ID: %d (bin=0b%s)", this->remote_id_, std::bitset<5>(this->remote_id_).to_string().c_str());
      ESP_LOGCONFIG(TAG, "  Repeat - send times: %d", this->send_times_);
      ESP_LOGCONFIG(TAG, "  Repeat - send wait: %d", this->send_wait_);
    }

  } // namespace star
} // namespace esphome