#include <obs-module.h>
#include <codec_api.h>
#include <codec_def.h>
#include <wels/codec_api.h>
#include <media-io/video-frame.h>

#include <stdlib.h>
#include <string.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-openh264", "en-US")

struct openh264_encoder {
	struct obs_encoder encoder;

	ISVCEncoder *encoder_instance;
	SEncParamBase enc_params;
};

static const char *openh264_get_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return "OpenH264 Encoder";
}

static void *openh264_create(obs_data_t *settings, obs_encoder_t *encoder)
{
	struct openh264_encoder *enc = bzalloc(sizeof(*enc));
	enc->encoder = *encoder;

	WelsCreateSVCEncoder(&enc->encoder_instance);

	memset(&enc->enc_params, 0, sizeof(enc->enc_params));
	enc->enc_params.iUsageType = CAMERA_VIDEO_REAL_TIME;
	enc->enc_params.fMaxFrameRate = 30.0;
	enc->enc_params.iPicWidth = 1280;
	enc->enc_params.iPicHeight = 720;
	enc->enc_params.iTargetBitrate = 1000000;

	if (enc->encoder_instance->Initialize(enc->encoder_instance, &enc->enc_params) != cmResultSuccess) {
		blog(LOG_ERROR, "OpenH264: Failed to initialize encoder");
		bfree(enc);
		return NULL;
	}

	return enc;
}

static void openh264_destroy(void *data)
{
	struct openh264_encoder *enc = data;
	if (enc->encoder_instance) {
		enc->encoder_instance->Uninitialize(enc->encoder_instance);
		WelsDestroySVCEncoder(enc->encoder_instance);
	}
	bfree(enc);
}

struct obs_encoder_info obs_openh264_encoder = {
	.id = "openh264",
	.type = OBS_ENCODER_VIDEO,
	.codec = "h264",
	.get_name = openh264_get_name,
	.create = openh264_create,
	.destroy = openh264_destroy,
	.encode = openh264_encode,
};

static bool openh264_encode(void *data, struct encoder_frame *frame,
			    struct encoder_packet *packet, bool *received_packet)
{
	struct openh264_encoder *enc = data;

	SSourcePicture pic = {0};
	pic.iPicWidth = enc->enc_params.iPicWidth;
	pic.iPicHeight = enc->enc_params.iPicHeight;
	pic.iColorFormat = videoFormatI420;
	pic.uiTimeStamp = (uint32_t)(frame->pts * 1000 / 1000000); // ms

	// OBS plane[0] = Y, plane[1] = U, plane[2] = V
	pic.pData[0] = frame->data[0];
	pic.pData[1] = frame->data[1];
	pic.pData[2] = frame->data[2];

	pic.iStride[0] = frame->linesize[0];
	pic.iStride[1] = frame->linesize[1];
	pic.iStride[2] = frame->linesize[2];

	SFrameBSInfo info = {0};
	int ret = enc->encoder_instance->EncodeFrame(enc->encoder_instance, &pic, &info);
	if (ret != cmResultSuccess || info.eFrameType == videoFrameTypeSkip) {
		*received_packet = false;
		return true;
	}

	// Calcular tamanho do pacote
	int total_size = 0;
	for (int i = 0; i < info.iLayerNum; i++) {
		SLayerBSInfo *layer = &info.sLayerInfo[i];
		for (int j = 0; j < layer->iNalCount; j++) {
			total_size += layer->pNalLengthInByte[j];
		}
	}

	uint8_t *data_buf = bmemdup(info.sLayerInfo[0].pBsBuf, total_size);
	if (!data_buf) {
		return false;
	}

	packet->data = data_buf;
	packet->size = total_size;
	packet->type = OBS_ENCODER_VIDEO;
	packet->pts = frame->pts;
	packet->dts = frame->dts;
	packet->keyframe = info.eFrameType == videoFrameTypeIDR;

	*received_packet = true;
	return true;
}
