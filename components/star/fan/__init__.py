import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import fan
from esphome.const import (
    CONF_ID,
    CONF_OUTPUT_ID,
)
from .. import (
    star_ns,
    STAR_LISTENER_SCHEMA,
    StarTransmitterAndReceiverListener,
    CONF_STAR_ID,
)

# AUTO_LOAD = ["remote_base"]
# DEPENDENCIES = ["star"]


StarFanComponent = star_ns.class_(
    "StarFanComponent", 
    cg.Component,
    StarTransmitterAndReceiverListener,
    fan.Fan
)


CONFIG_SCHEMA = (
    fan.FAN_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(StarFanComponent),
        }
    )
    .extend(STAR_LISTENER_SCHEMA)
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await fan.register_fan(var, config)
        
    star = await cg.get_variable(config[CONF_STAR_ID])
    cg.add(var.set_star_output(star))
    

