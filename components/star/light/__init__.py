import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import light
from esphome.const import (
    CONF_ID,
    CONF_OUTPUT_ID,
)
from .. import (
    star_ns,
    StarTransmitterAndReceiverListener,
    STAR_LISTENER_SCHEMA,
    CONF_STAR_ID,
)

# AUTO_LOAD = ["remote_base"]
# DEPENDENCIES = ["star"]


StarLightComponent = star_ns.class_(
    "StarLightComponent",
    StarTransmitterAndReceiverListener,
    light.LightOutput,
)


CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(StarLightComponent),
        }
    )
    .extend(STAR_LISTENER_SCHEMA)
    .extend(light.BINARY_LIGHT_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    await light.register_light(var, config)
    star = await cg.get_variable(config[CONF_STAR_ID])
    cg.add(var.set_star_output(star))


ToggleStateAction = star_ns.class_("ToggleStateAction", automation.Action)

TOGGLE_STATE_ACTION_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(StarLightComponent),
    }
)


@automation.register_action(
    "star_light.toggle_state", ToggleStateAction, TOGGLE_STATE_ACTION_SCHEMA
)
async def star_light_toggle_state_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)

    return var
