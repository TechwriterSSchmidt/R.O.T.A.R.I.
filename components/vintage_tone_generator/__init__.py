import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2s_audio
from esphome.const import CONF_ID

DEPENDENCIES = ['i2s_audio']

vintage_ns = cg.global_ns.namespace('rotary')
VintageToneGenerator = vintage_ns.class_('VintageToneGenerator', cg.Component)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(VintageToneGenerator),
    cv.Required('i2s_audio_id'): cv.use_id(i2s_audio.I2SAudioComponent),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID], await cg.get_variable(config['i2s_audio_id']))
    await cg.register_component(var, config)
