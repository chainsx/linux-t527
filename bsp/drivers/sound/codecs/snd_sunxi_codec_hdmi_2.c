// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright(c) 2020 - 2024 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Allwinner's ALSA SoC Audio driver
 *
 * Copyright (c) 2023, lijingpsw <lijingpsw@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#define SUNXI_MODNAME		"sound-codec-hdmi"
#include "snd_sunxi_log.h"
#include <linux/module.h>
#include <linux/extcon.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include <sound/hdmi-codec.h>

#include "snd_sunxi_common.h"

#define DRV_NAME	"sunxi-snd-codec-hdmi"

struct sunxi_codec {
	struct platform_device *pdev;

	struct hdmi_codec_daifmt daifmt;
	struct hdmi_codec_params params;
	struct hdmi_codec_pdata *pdata;
	enum hdmi_audio_coding_type coding_type;
};

struct sunxi_codec_priv {
	struct platform_device pdev;

	struct hdmi_codec_daifmt daifmt;
	struct hdmi_codec_params params;
	struct hdmi_codec_pdata pdata;

	enum hdmi_audio_coding_type coding_type;
	bool update_param;
	bool working;
};

static struct sunxi_codec_priv g_hdmi_priv;

static void sunxi_hdmi_plugin_cb(struct device *dev, bool plugged)
{
	unsigned int ret;
	struct hdmi_codec_params *codec_params = &g_hdmi_priv.params;
	struct hdmi_codec_daifmt *daifmt = &g_hdmi_priv.daifmt;
	struct hdmi_codec_pdata *pdata = &g_hdmi_priv.pdata;

	SND_LOG_INFO("plugged:%d\n", plugged);

	if (dev == NULL || daifmt == NULL || codec_params == NULL) {
		SND_LOG_INFO("dev or daifmt or codec_params is null\n");
		return;
	}

	if (plugged && g_hdmi_priv.working) {
		ret = pdata->ops->hw_params(dev, NULL, daifmt, codec_params);
		if (ret) {
			SND_LOG_ERR("hdmi_hw_params fail\n");
			return;
		}

		ret = pdata->ops->audio_startup(dev, NULL);
		if (ret) {
			SND_LOG_ERR("hdmi_audio_startup fail\n");
			return;
		}
	}

	return;
}

static int sunxi_data_fmt_get_data_fmt(struct snd_kcontrol *kcontrol,
				       struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct sunxi_codec *codec = snd_soc_component_get_drvdata(component);

	ucontrol->value.integer.value[0] = codec->coding_type;

	return 0;
}

static int sunxi_data_fmt_set_data_fmt(struct snd_kcontrol *kcontrol,
				       struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct sunxi_codec *codec = snd_soc_component_get_drvdata(component);

	codec->coding_type = ucontrol->value.integer.value[0];

	return 0;
}

static const char *data_fmt[] = {
	"NULL", "PCM", "AC3", "MPEG1", "MP3", "MPEG2", "AAC", "DTS", "ATRAC",
	"ONE_BIT_AUDIO", "DOLBY_DIGITAL_PLUS", "DTS_HD", "MAT", "DST", "WMAPRO"
};
static SOC_ENUM_SINGLE_EXT_DECL(data_fmt_enum, data_fmt);
static const struct snd_kcontrol_new data_fmt_controls[] = {
	SOC_ENUM_EXT("audio data format", data_fmt_enum,
		     sunxi_data_fmt_get_data_fmt,
		     sunxi_data_fmt_set_data_fmt),
};

static int sunxi_codec_dai_set_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
	struct snd_soc_component *component = dai->component;
	struct sunxi_codec *codec = snd_soc_component_get_drvdata(component);
	struct hdmi_codec_daifmt *daifmt = &codec->daifmt;

	SND_LOG_DEBUG("\n");

	/* get dai mode */
	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		daifmt->fmt = HDMI_I2S;
		break;
	case SND_SOC_DAIFMT_RIGHT_J:
		daifmt->fmt = HDMI_RIGHT_J;
		break;
	case SND_SOC_DAIFMT_LEFT_J:
		daifmt->fmt = HDMI_LEFT_J;
		break;
	case SND_SOC_DAIFMT_DSP_A:
		daifmt->fmt = HDMI_DSP_A;
		break;
	case SND_SOC_DAIFMT_DSP_B:
		daifmt->fmt = HDMI_DSP_B;
		break;
	default:
		SND_LOG_ERR("hdmi_codec_daifmt fmt set fail\n");
		return -EINVAL;
	}

	return 0;
}

static void sunxi_codec_params_update(struct sunxi_codec *codec)
{
	struct hdmi_codec_params *new_params = &codec->params;
	struct hdmi_audio_infoframe *new_cea = &new_params->cea;
	struct hdmi_codec_daifmt *new_daifmt = &codec->daifmt;
	struct hdmi_codec_params *old_params = &g_hdmi_priv.params;
	struct hdmi_audio_infoframe *old_cea = &old_params->cea;

	if ((new_params->sample_rate == old_params->sample_rate)
	    && (new_params->channels == old_params->channels)
	    && (new_cea->sample_size == old_cea->sample_size)
	    && (new_cea->coding_type == old_cea->coding_type)
	    && (new_cea->channel_allocation == old_cea->channel_allocation)) {
		g_hdmi_priv.update_param = 0;
		g_hdmi_priv.working = 1;
		return;
	    }

	memcpy(&g_hdmi_priv.pdev, codec->pdev, sizeof(struct platform_device));
	memcpy(&g_hdmi_priv.daifmt, new_daifmt, sizeof(struct hdmi_codec_daifmt));
	memcpy(&g_hdmi_priv.params, new_params, sizeof(struct hdmi_codec_params));
	memcpy(&g_hdmi_priv.params.cea, new_cea, sizeof(struct hdmi_audio_infoframe));
	memcpy(&g_hdmi_priv.pdata, codec->pdata, sizeof(struct hdmi_codec_pdata));

	g_hdmi_priv.coding_type = codec->coding_type;

	g_hdmi_priv.update_param = 1;
	g_hdmi_priv.working = 1;

	return;
}

static int sunxi_codec_dai_hw_params(struct snd_pcm_substream *substream,
				     struct snd_pcm_hw_params *params,
				     struct snd_soc_dai *dai)
{
	struct snd_soc_component *component = dai->component;
	struct sunxi_codec *codec = snd_soc_component_get_drvdata(component);
	struct hdmi_codec_params *codec_params = &codec->params;
	struct hdmi_codec_daifmt *daifmt = &codec->daifmt;
	struct hdmi_codec_pdata *pdata = codec->pdata;
	struct platform_device *pdev = codec->pdev;
	int ret;

	SND_LOG_DEBUG("\n");

	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S16_LE:
		codec_params->cea.sample_size = 16;
		break;
	case SNDRV_PCM_FORMAT_S20_3LE:
	case SNDRV_PCM_FORMAT_S24_3LE:
		codec_params->cea.sample_size = 20;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
	case SNDRV_PCM_FORMAT_S32_LE:
		codec_params->cea.sample_size = 24;
		break;
	default:
		return -EINVAL;
	}

	if (codec->coding_type > HDMI_AUDIO_CODING_TYPE_PCM) {
		codec_params->cea.sample_size = 24;
	}

	codec_params->cea.coding_type = codec->coding_type;

	codec_params->sample_rate = params_rate(params);

	codec_params->channels = params_channels(params);
	if (codec_params->channels == 8) {
		codec_params->cea.channel_allocation = 0x13;
	} else if (codec_params->channels == 6) {
		codec_params->cea.channel_allocation = 0x0b;
	} else if (codec_params->channels == 3) {
		codec_params->cea.channel_allocation = 0x1f;
	} else {
		codec_params->cea.channel_allocation = 0x0;
	}

	sunxi_codec_params_update(codec);

	if (g_hdmi_priv.update_param) {
		ret = pdata->ops->hw_params(&pdev->dev, NULL, daifmt, codec_params);
		if (ret < 0) {
			SND_LOG_ERR("hdmi_codec_pdata hw params fail\n");
			return ret;
		}

		ret = pdata->ops->audio_startup(&pdev->dev, NULL);
		if (ret < 0) {
			SND_LOG_ERR("hdmi_codec_pdata startup fail\n");
			return ret;
		}
	}
	return 0;
}

static int sunxi_codec_dai_startup(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
	struct snd_soc_component *component = dai->component;
	struct sunxi_codec *codec = snd_soc_component_get_drvdata(component);
	int ret;

	ret = snd_sunxi_extparam_set_state_sync(component->card->name,
						EXTPARAM_ID_HDMI_FMT,
						(void *)&codec->coding_type);
	if (ret) {
		SND_LOG_ERR("extparam set state sync failed\n");
		return ret;
	}

	return 0;
}

static int sunxi_codec_dai_mute_stream(struct snd_soc_dai *dai, int mute, int stream)
{
	struct snd_soc_component *component = dai->component;
	struct sunxi_codec *codec = snd_soc_component_get_drvdata(component);
	struct hdmi_codec_pdata *pdata = codec->pdata;
	struct platform_device *pdev = codec->pdev;
	int ret;

	SND_LOG_DEBUG("\n");

	ret = pdata->ops->mute_stream(&pdev->dev, NULL, mute, stream);
	if (ret < 0) {
		SND_LOG_ERR("hdmi_codec_pdata mute stream fail\n");
		return ret;
	}

	return 0;
}

static void sunxi_codec_dai_shutdown(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
	struct snd_soc_component *component = dai->component;
	struct sunxi_codec *codec = snd_soc_component_get_drvdata(component);
	struct hdmi_codec_pdata *pdata = codec->pdata;
	struct platform_device *pdev = codec->pdev;

	SND_LOG_DEBUG("\n");

	g_hdmi_priv.working = 0;

	pdata->ops->audio_shutdown(&pdev->dev, NULL);
}

static const struct snd_soc_dai_ops sunxi_codec_dai_ops = {
	.set_fmt	= sunxi_codec_dai_set_fmt,
	.hw_params	= sunxi_codec_dai_hw_params,
	.startup	= sunxi_codec_dai_startup,
	.mute_stream	= sunxi_codec_dai_mute_stream,
	.shutdown	= sunxi_codec_dai_shutdown,
};

static struct snd_soc_dai_driver sunxi_codec_dai = {
	.name = DRV_NAME,
	.playback = {
		.stream_name	= "Playback",
		.channels_min	= 1,
		.channels_max	= 8,
		.rates		= SNDRV_PCM_RATE_8000_192000
				| SNDRV_PCM_RATE_KNOT,
		.formats	= SNDRV_PCM_FMTBIT_S16_LE
				| SNDRV_PCM_FMTBIT_S20_LE
				| SNDRV_PCM_FMTBIT_S24_LE
				| SNDRV_PCM_FMTBIT_S24_3LE
				| SNDRV_PCM_FMTBIT_S32_LE,
	},
	.ops = &sunxi_codec_dai_ops,
};

static int sunxi_codec_component_probe(struct snd_soc_component *component)
{
	int ret;
	struct sunxi_codec *codec = snd_soc_component_get_drvdata(component);
	struct hdmi_codec_pdata *pdata = codec->pdata;

	SND_LOG_DEBUG("\n");

	codec->coding_type = HDMI_AUDIO_CODING_TYPE_PCM;

	ret = snd_soc_add_component_controls(component,
					     data_fmt_controls,
					     ARRAY_SIZE(data_fmt_controls));
	if (ret)
		SND_LOG_ERR("add kcontrols failed\n");


	if (pdata->ops->hook_plugged_cb) {
		pdata->ops->hook_plugged_cb(component->dev->parent,
					    pdata->data,
					    sunxi_hdmi_plugin_cb,
					    component->dev);
	}

	return 0;
}

static void sunxi_codec_component_remove(struct snd_soc_component *component)
{
	struct sunxi_codec *codec = snd_soc_component_get_drvdata(component);
	struct hdmi_codec_pdata *pdata = codec->pdata;
	struct platform_device *pdev = codec->pdev;

	if (pdata->ops->hook_plugged_cb)
		pdata->ops->hook_plugged_cb(&pdev->dev,
					    pdata->data, NULL, NULL);
}

static struct snd_soc_component_driver sunxi_codec_component_dev = {
	.name		= DRV_NAME,
	.probe		= sunxi_codec_component_probe,
	.remove		= sunxi_codec_component_remove,
};

static int sunxi_codec_dev_probe(struct platform_device *pdev)
{
	struct hdmi_codec_pdata *pdata = pdev->dev.platform_data;
	struct device *dev = &pdev->dev;
	struct sunxi_codec *codec;
	int ret;
	const char *devname = dev_name(dev);

	SND_LOG_DEBUG("device name:%s\n", devname);

	if (!pdata) {
		SND_LOG_ERR("No hdmi codec pdata\n");
		return -EINVAL;
	}

	if (!pdata->ops || !pdata->ops->hw_params || !pdata->ops->audio_startup ||
	    !pdata->ops->audio_shutdown || !pdata->ops->mute_stream) {
		SND_LOG_ERR("ops incomplete\n");
		return -EINVAL;
	}

	/* sunxi codec info */
	codec = devm_kzalloc(dev, sizeof(struct sunxi_codec), GFP_KERNEL);
	if (!codec) {
		SND_LOG_ERR("can't allocate sunxi codec hdmi memory\n");
		return -ENOMEM;
	}
	codec->pdev = pdev;
	codec->pdata = pdata;
	dev_set_drvdata(dev, codec);

	/* alsa component register */
	ret = snd_soc_register_component(dev, &sunxi_codec_component_dev,
					 &sunxi_codec_dai, 1);
	if (ret) {
		SND_LOG_ERR("codec-hdmi component register failed\n");
		return -EINVAL;
	}

	SND_LOG_DEBUG("register codec-hdmi success\n");

	return 0;
}

static int sunxi_codec_dev_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct sunxi_codec *codec = dev_get_drvdata(dev);

	SND_LOG_DEBUG("\n");

	snd_soc_unregister_component(dev);

	devm_kfree(dev, codec);

	SND_LOG_DEBUG("unregister codec-hdmi success\n");

	return 0;
}

static struct platform_driver sunxi_codec_driver = {
	.driver = {
		.name = DRV_NAME,
	},
	.probe	= sunxi_codec_dev_probe,
	.remove	= sunxi_codec_dev_remove,
};

int __init sunxi_hdmi_codec_dev_init(void)
{
	int ret;

	ret = platform_driver_register(&sunxi_codec_driver);
	if (ret != 0) {
		SND_LOG_ERR("platform driver register failed\n");
		return -EINVAL;
	}

	return ret;
}

void __exit sunxi_hdmi_codec_dev_exit(void)
{
	platform_driver_unregister(&sunxi_codec_driver);
}

late_initcall(sunxi_hdmi_codec_dev_init);
module_exit(sunxi_hdmi_codec_dev_exit);

MODULE_AUTHOR("lijingpsw@allwinnertech.com");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0.0");
MODULE_DESCRIPTION("sunxi soundcard codec of new hdmi");
