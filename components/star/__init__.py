import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import (
    remote_base,
    light,
)
from esphome.const import (
    CONF_ID,
    CONF_REPEAT,
    CONF_WAIT_TIME,
    CONF_TIMES,
    CONF_CODE,
)
CONF_REMOTE_ID = "remote_id"
CONF_STAR_ID = "star_id"
CONF_BLOCK_FOR_TRANSMIT = "block_for_transmit_time"
CONF_NOTIFICATION_LIGHT = "notification_light"

AUTO_LOAD = ["remote_base"]
# DEPENDENCIES = ["libretiny"]

star_ns = cg.esphome_ns.namespace("star")
remote_base_ns = cg.esphome_ns.namespace("remote_base")


StarComponent = star_ns.class_(
    "StarComponent", 
    cg.Component,
    remote_base.RemoteReceiverListener,
    remote_base.RemoteTransmittable,
)

# StarReceiverListener = star_ns.class_("StarReceiverListener")
StarTransmitterAndReceiverListener = star_ns.class_("StarTransmitterAndReceiverListener")

STAR_LISTENER_SCHEMA = cv.Schema({cv.Required(CONF_STAR_ID): cv.use_id(StarComponent)})


def validate_repeat(value):
    if isinstance(value, dict):
        return cv.Schema(
            {
                cv.Required(CONF_TIMES): cv.positive_int,
                cv.Optional(CONF_WAIT_TIME, default="10ms"): cv.positive_time_period_microseconds,
            }
        )(value)
    return validate_repeat({CONF_TIMES: value})


CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(StarComponent),
            cv.Required(remote_base.CONF_RECEIVER_ID): cv.use_id(remote_base.RemoteReceiverBase),
            cv.Required(remote_base.CONF_TRANSMITTER_ID): cv.use_id(remote_base.RemoteTransmitterBase),
            cv.Required(CONF_REMOTE_ID): cv.positive_int,
            cv.Optional(CONF_REPEAT, default={CONF_TIMES: 10, CONF_WAIT_TIME: "10ms"}): validate_repeat,
            cv.Optional(CONF_BLOCK_FOR_TRANSMIT, default="700ms"): cv.positive_time_period_milliseconds,
            cv.Optional(CONF_NOTIFICATION_LIGHT): cv.use_id(light.LightState),
            
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(remote_base.REMOTE_TRANSMITTABLE_SCHEMA)
    .extend(remote_base.REMOTE_LISTENER_SCHEMA)
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await remote_base.register_transmittable(var, config)
    if remote_base.CONF_RECEIVER_ID in config:
        await remote_base.register_listener(var, config)
    
    # if CONF_REPEAT in config:
    conf_repeat = config[CONF_REPEAT] 
    cg.add(var.set_send_times(conf_repeat[CONF_TIMES]))
    cg.add(var.set_send_wait(conf_repeat[CONF_WAIT_TIME]))
            
    cg.add(var.set_remote_id(config[CONF_REMOTE_ID]))
    cg.add(var.set_block_for_transmit(config[CONF_BLOCK_FOR_TRANSMIT]))
    
    
    if CONF_NOTIFICATION_LIGHT in config:
        notification_light = await cg.get_variable(config[CONF_NOTIFICATION_LIGHT])
        cg.add(var.set_notification_light(notification_light))
    
    await cg.register_component(var, config)
    
    


SendCommandAction = star_ns.class_("SendCommandAction", automation.Action)

SEND_COMMAND_ACTION_SCHEMA = cv.Schema(
    {
        # cv.Required(CONF_ID): cv.use_id(StarComponent),
        cv.GenerateID(): cv.use_id(StarComponent),
        cv.Required(CONF_CODE): cv.positive_int,
        # cv.Optional(CONF_REPEAT, default=10): cv.positive_int,
    }
)

@automation.register_action("star.send_command", SendCommandAction, SEND_COMMAND_ACTION_SCHEMA)
async def star_send_command_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)
    

    # repeats = await cg.templatable(config[CONF_REPEAT], args, int)
    code = config[CONF_CODE]

    # cg.add(var.set_repeats(repeats))
    cg.add(var.set_code(code))
    return var