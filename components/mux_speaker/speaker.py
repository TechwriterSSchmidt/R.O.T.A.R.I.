import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import speaker, i2s_audio, switch
from esphome.const import CONF_ID

DEPENDENCIES = ["i2s_audio"]

mux_speaker_ns = cg.global_ns.namespace("mux_speaker")
MuxSpeaker = mux_speaker_ns.class_("MuxSpeaker", speaker.Speaker, cg.Component)

CONF_HANDSET_BUS = "handset_bus"
CONF_BASE_BUS = "base_bus"
CONF_MODE_SWITCH = "mode_switch"

CONFIG_SCHEMA = speaker.SPEAKER_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(MuxSpeaker),
        cv.Required(CONF_HANDSET_BUS): cv.use_id(i2s_audio.I2SAudioComponent),
        cv.Required(CONF_BASE_BUS): cv.use_id(i2s_audio.I2SAudioComponent),
        cv.Optional(CONF_MODE_SWITCH): cv.use_id(switch.Switch),
    }
).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await speaker.register_speaker(var, config)
    
    handset = await cg.get_variable(config[CONF_HANDSET_BUS])
    cg.add(var.set_handset_bus(handset))
    
    base = await cg.get_variable(config[CONF_BASE_BUS])
    cg.add(var.set_base_bus(base))
    
    if CONF_MODE_SWITCH in config:
        sw = await cg.get_variable(config[CONF_MODE_SWITCH])
        cg.add(var.set_mode_switch(sw))
