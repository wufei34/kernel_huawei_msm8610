/* Copyright (c) 2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <mach/gpiomux.h>
#include "msm_sensor.h"
#include "msm_sd.h"
#include "camera.h"
#include "msm_cci.h"
#include "msm_camera_io_util.h"
#include "msm_camera_i2c_mux.h"
#include <mach/rpm-regulator.h>
#include <mach/rpm-regulator-smd.h>
#include <linux/regulator/consumer.h>

#define IMX134_SUNNY_P8N09F_SENSOR_NAME "imx134_sunny_p8n09f"
DEFINE_MSM_MUTEX(imx134_sunny_p8n09f_mut);

#undef CDBG
#define IMX134_SUNNY_P8N09F_DEBUG
#ifdef IMX134_SUNNY_P8N09F_DEBUG
#define CDBG(fmt, args...) pr_err(fmt, ##args)
#else
#define CDBG(fmt, args...) do { } while (0)
#endif

static struct msm_sensor_ctrl_t imx134_sunny_p8n09f_s_ctrl;

static struct msm_sensor_power_setting imx134_sunny_p8n09f_power_setting[] = {
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_VDIG,
		.config_val = GPIO_OUT_HIGH,
		.delay = 15,
	},
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VANA,
		.config_val = 0,
		.delay = 5,
	},	
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VIO,
		.config_val = 0,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_RESET,
		.config_val = GPIO_OUT_LOW,
		.delay = 40,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_AF_PWDM,
		.config_val = GPIO_OUT_LOW,
		.delay = 20,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_RESET,
		.config_val = GPIO_OUT_HIGH,
		.delay = 40,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_AF_PWDM,
		.config_val = GPIO_OUT_HIGH,
		.delay = 20,
	},
	{
		.seq_type = SENSOR_CLK,
		.seq_val = SENSOR_CAM_MCLK,
		.config_val = 24000000,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_I2C_MUX,
		.seq_val = 0,
		.config_val = 0,
		.delay = 0,
	},
};

static struct v4l2_subdev_info imx134_sunny_p8n09f_subdev_info[] = {
	{
		.code   = V4L2_MBUS_FMT_SGRBG10_1X10,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt    = 1,
		.order    = 0,
	},
};

static const struct i2c_device_id imx134_sunny_p8n09f_i2c_id[] = {
	{IMX134_SUNNY_P8N09F_SENSOR_NAME, (kernel_ulong_t)&imx134_sunny_p8n09f_s_ctrl},
	{ }
};

static int32_t msm_imx134_sunny_p8n09f_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	return msm_sensor_i2c_probe(client, id, &imx134_sunny_p8n09f_s_ctrl);
}

static struct i2c_driver imx134_sunny_p8n09f_i2c_driver = {
	.id_table = imx134_sunny_p8n09f_i2c_id,
	.probe  = msm_imx134_sunny_p8n09f_i2c_probe,
	.driver = {
		.name = IMX134_SUNNY_P8N09F_SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client imx134_sunny_p8n09f_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static const struct of_device_id imx134_sunny_p8n09f_dt_match[] = {
	{.compatible = "qcom,imx134_sunny_p8n09f", .data = &imx134_sunny_p8n09f_s_ctrl},
	{}
};

MODULE_DEVICE_TABLE(of, imx134_sunny_p8n09f_dt_match);

static struct platform_driver imx134_sunny_p8n09f_platform_driver = {
	.driver = {
		.name = "qcom,imx134_sunny_p8n09f",
		.owner = THIS_MODULE,
		.of_match_table = imx134_sunny_p8n09f_dt_match,
	},
};

static int32_t imx134_sunny_p8n09f_platform_probe(struct platform_device *pdev)
{
	int32_t rc = 0;
	const struct of_device_id *match;
	match = of_match_device(imx134_sunny_p8n09f_dt_match, &pdev->dev);
	rc = msm_sensor_platform_probe(pdev, match->data);
	return rc;
}

static int __init imx134_sunny_p8n09f_init_module(void)
{
	int32_t rc = 0;
	pr_info("%s:%d\n", __func__, __LINE__);
	rc = platform_driver_probe(&imx134_sunny_p8n09f_platform_driver,
		imx134_sunny_p8n09f_platform_probe);
	if (!rc)
		return rc;
	pr_err("%s:%d rc %d\n", __func__, __LINE__, rc);
	return i2c_add_driver(&imx134_sunny_p8n09f_i2c_driver);
}

static void __exit imx134_sunny_p8n09f_exit_module(void)
{
	pr_info("%s:%d\n", __func__, __LINE__);
	if (imx134_sunny_p8n09f_s_ctrl.pdev) {
		msm_sensor_free_sensor_data(&imx134_sunny_p8n09f_s_ctrl);
		platform_driver_unregister(&imx134_sunny_p8n09f_platform_driver);
	} else
		i2c_del_driver(&imx134_sunny_p8n09f_i2c_driver);
	return;
}

static struct msm_sensor_ctrl_t imx134_sunny_p8n09f_s_ctrl = {
	.sensor_i2c_client = &imx134_sunny_p8n09f_sensor_i2c_client,
	.power_setting_array.power_setting = imx134_sunny_p8n09f_power_setting,
	.power_setting_array.size = ARRAY_SIZE(imx134_sunny_p8n09f_power_setting),
	.msm_sensor_mutex = &imx134_sunny_p8n09f_mut,
	.sensor_v4l2_subdev_info = imx134_sunny_p8n09f_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(imx134_sunny_p8n09f_subdev_info),
};

module_init(imx134_sunny_p8n09f_init_module);
module_exit(imx134_sunny_p8n09f_exit_module);
MODULE_DESCRIPTION("imx134_sunny_p8n09f");
MODULE_LICENSE("GPL v2");
