# ESPHome Star Fans RF Remote

This is a custom ESPHome component for controlling Star Fans via RF.  
It supports both sending and receiving RF signals, allowing for state synchronization when using the original remote.

## Features

- Full control of Star ceiling fan units via RF (send/receive)
- State updates when using the original remote
- Supports all standard light and fan options

## Example Configuration

```yaml
esphome:
  name: star_fan_controller

external_components:
  - source: github://amirlanesman/esphome-star-fans
    components: [star]

remote_receiver:
  pin: 
    number: GPIO5
    mode: INPUT_PULLUP
  tolerance: 30%
  filter: 230us
  idle: 8ms

remote_transmitter:
  pin: GPIO4
  carrier_duty_percent: 100%

star:
  id: star1
  remote_id: 6 # Replace with your actual remote ID (see below for details)

light:
  - platform: star
    star_id: star1
    name: Star Fan Light
    restore_mode: RESTORE_DEFAULT_OFF

fan:
  - platform: star
    star_id: star1
    name: Star Fan
```

---

## Configuration Options

### `star` Component

| Option                    | Required | Type   | Description                                                                 |
|---------------------------|----------|--------|-----------------------------------------------------------------------------|
| `id`                      | Yes      | id     | ID for the Star fan controller                                              |
| `remote_id`               | Yes      | int    | Star remote control ID (see how to find yours below)                                |
| `transmitter_id`          | No       | id     | ID of the remote_transmitter component                                      |
| `receiver_id`             | No       | id     | ID of the remote_receiver component                                         |
| `repeat.times`            | No       | int    | How many times to send each message                                         |
| `repeat.wait_time`        | No       | time   | How long to wait between message repeats                                    |
| `block_for_transmit_time` | No       | time   | How long to ignore a received message after sending (prevents echo)         |
| `notification_light`      | No       | id     | Light to blink when receiving/transmitting a signal                         |

---

### `light.star` Platform

| Option         | Required | Type   | Description                                  |
|----------------|----------|--------|----------------------------------------------|
| `platform`     | Yes      | string | Must be `star`                               |
| `star_id`      | Yes      | id     | ID of the `star` component to control        |
| `name`         | Yes      | string | Name of the light entity                     |
| `restore_mode` | No       | enum   | Light restore mode (see ESPHome docs)        |

---

### `fan.star` Platform

| Option     | Required | Type   | Description                                  |
|------------|----------|--------|----------------------------------------------|
| `platform` | Yes      | string | Must be `star`                               |
| `star_id`  | Yes      | id     | ID of the `star` component to control        |
| `name`     | Yes      | string | Name of the fan entity                       |

---

## Remote Control ID

Each star fan remote control has a specific 5 bit ID, It can be learnt from the debug console: 
1. Setup everything and ensure your debug level is at least DEBUG.
2. Use a button on the remote control.
3. Look for this line in the logs:
```
Remote ID mismatch: expected <The value you set in the config>, got <The actual value>
```
for example:
```
Remote ID mismatch: expected 06, got 02
```

Another option is calculating it:
1. Open the battery cover.
2. Look at the 6 toggles above the battery.
3. Each up toggle is a 0 bit and each down toggle is a 1 bit.
4. Convert the toggles to binary so that left-most toggle is 2^4 and the 5th toggle is 2^0.
5. If your toggles are: `up - up - down - down - up` => `0b00110 = 6`
6. That is your Remote Control ID

---

## Actions

### `star` Actions

- `star.send_command`: Send a raw Star RF command (advanced)

### `light.star` Actions

- `light.turn_on`: Turn on the fan light
- `light.turn_off`: Turn off the fan light
- `light.toggle`: Toggle the fan light
- `star_light.toggle_state`: Change the light state without sending a command (useful when out of sync)

### `fan.star` Actions

- `fan.turn_on`: Turn on the fan
- `fan.turn_off`: Turn off the fan
- `fan.toggle`: Toggle the fan
- `fan.set_speed`: Set fan speed

---

## Notes

- Only tested with Star Fans RF remotes.
- Make sure your RF receiver and transmitter are connected to the correct GPIO pins.
- This component is designed for Star ceiling fans using the standard RF protocol.
- To find your `remote_id`, use the ESPHome logs and original remote, or refer to the documentation.

## Credits

Based on the ESPHome platform and extended for RF receive support for Star Fans.