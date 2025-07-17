#include <obs-module.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-openh264", "en-US")
MODULE_EXPORT const char *obs_module_description(void)
{
	return "OpenH264 based encoder";
}

extern struct obs_encoder_info obs_openh264_encoder;

bool obs_module_load(void)
{
	obs_register_encoder(&obs_openh264_encoder);
	return true;
}
