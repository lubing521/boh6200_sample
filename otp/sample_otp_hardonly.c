/*
 * Copyright (C) Hisilicon Technologies Co., Ltd. 2019. All rights reserved.
 * Description: Test the file of the hard only interface.
 * Author: Linux SDK team
 * Create: 2019-09-24
 */

#include "hi_unf_otp.h"
#include "sample_otp_base.h"

static hi_s32 get_hard_only_stat(hi_s32 argc, hi_char *argv[]);
static hi_s32 set_enable_hard_only(hi_s32 argc, hi_char *argv[]);

static otp_sample g_otp_sample_data[] = {
    { 0, "help", NULL, { "Display this help and exit.", "example: ./sample_otp_hardonly help" } },
    {
        1, "set", set_enable_hard_only,
        {
            "Set enable hard only --> module(HI_UNF_OTP_MODULE_ALL) algorithm(HI_OTP_ALG_AES).",
            "example: ./sample_otp_hardonly set mcipher aes"
        }
    },
    {
        2, "set", set_enable_hard_only,
        {
            "Set enable hard only --> module(HI_UNF_OTP_MODULE_ALL) algorithm(HI_OTP_ALG_TDES).",
            "example: ./sample_otp_hardonly set mcipher tdes"
        }
    },
    {
        3, "set", set_enable_hard_only,
        {
            "Set enable hard only --> module(HI_UNF_OTP_MODULE_ALL) algorithm(HI_OTP_ALG_SM4).",
            "example: ./sample_otp_hardonly set mcipher sm4"
        }
    },
    {
        4, "set", set_enable_hard_only,
        {
            "Set enable hard only --> module(HI_UNF_OTP_MODULE_TSCIPHER) algorithm(HI_OTP_ALG_CSA2).",
            "example: ./sample_otp_hardonly set tscipher csa2"
        }
    },
    {
        5, "set", set_enable_hard_only,
        {
            "Set enable hard only --> module(HI_UNF_OTP_MODULE_TSCIPHER) algorithm(HI_OTP_ALG_CSA3).",
            "example: ./sample_otp_hardonly set tscipher csa3"
        }
    },
    {
        6, "set", set_enable_hard_only,
        {
            "Set enable hard only --> module(HI_UNF_OTP_MODULE_TSCIPHER) algorithm(HI_OTP_ALG_AES).",
            "example: ./sample_otp_hardonly set tscipher aes"
        }
    },
    {
        7, "set", set_enable_hard_only,
        {
            "Set enable hard only --> module(HI_UNF_OTP_MODULE_TSCIPHER) algorithm(HI_OTP_ALG_SM4).",
            "example: ./sample_otp_hardonly set tscipher sm4"
        }
    },
    {
        8, "set", set_enable_hard_only,
        {
            "Set enable hard only --> module(HI_UNF_OTP_MODULE_TSCIPHER) algorithm(HI_OTP_ALG_TDES).",
            "example: ./sample_otp_hardonly set tscipher tdes"
        }
    },
    {
        9, "set", set_enable_hard_only,
        {
            "Set enable hard only --> module(HI_UNF_OTP_MODULE_TSCIPHER) algorithm(HI_OTP_ALG_MULTI2).",
            "example: ./sample_otp_hardonly set tscipher multi2"
        }
    },
    {
        10, "get", get_hard_only_stat,
        {
            "Get hard only stat --> module(HI_UNF_OTP_MODULE_ALL) algorithm(HI_OTP_ALG_AES).",
            "example: ./sample_otp_hardonly get mcipher aes"
        }
    },
    {
        11, "get", get_hard_only_stat,
        {
            "Get hard only stat --> module(HI_UNF_OTP_MODULE_ALL) algorithm(HI_OTP_ALG_TDES).",
            "example: ./sample_otp_hardonly get mcipher tdes"
        }
    },
    {
        12, "get", get_hard_only_stat,
        {
            "Get hard only stat --> module(HI_UNF_OTP_MODULE_ALL) algorithm(HI_OTP_ALG_SM4).",
            "example: ./sample_otp_hardonly get mcipher sm4"
        }
    },
    {
        13, "get", get_hard_only_stat,
        {
            "Get hard only stat --> module(HI_UNF_OTP_MODULE_TSCIPHER) algorithm(HI_OTP_ALG_CSA2).",
            "example: ./sample_otp_hardonly get tscipher csa2"
        }
    },
    {
        14, "get", get_hard_only_stat,
        {
            "Get hard only stat --> module(HI_UNF_OTP_MODULE_TSCIPHER) algorithm(HI_OTP_ALG_CSA3).",
            "example: ./sample_otp_hardonly get tscipher csa3"
        }
    },
    {
        15, "get", get_hard_only_stat,
        {
            "Get hard only stat --> module(HI_UNF_OTP_MODULE_TSCIPHER) algorithm(HI_OTP_ALG_AES).",
            "example: ./sample_otp_hardonly get tscipher aes"
        }
    },
    {
        16, "get", get_hard_only_stat,
        {
            "Get hard only stat --> module(HI_UNF_OTP_MODULE_TSCIPHER) algorithm(HI_OTP_ALG_SM4).",
            "example: ./sample_otp_hardonly get tscipher sm4"
        }
    },
    {
        17, "get", get_hard_only_stat,
        {
            "Get hard only stat --> module(HI_UNF_OTP_MODULE_TSCIPHER) algorithm(HI_OTP_ALG_TDES).",
            "example: ./sample_otp_hardonly get tscipher tdes"
        }
    },
    {
        18, "get", get_hard_only_stat,
        {
            "Get hard only stat --> module(HI_UNF_OTP_MODULE_TSCIPHER) algorithm(HI_OTP_ALG_MULTI2).",
            "example: ./sample_otp_hardonly get tscipher multi2"
        }
    },
};

static hi_void get_mcipher_algm_type(const hi_char *source, hi_unf_otp_alg_type *algm_type)
{
    if (case_strcmp("aes", source)) {
        *algm_type = HI_UNF_OTP_ALG_AES;
    } else if (case_strcmp("tdes", source)) {
        *algm_type = HI_UNF_OTP_ALG_TDES;
    } else if (case_strcmp("sm4", source)) {
        *algm_type = HI_UNF_OTP_ALG_SM4;
    } else {
        *algm_type = HI_UNF_OTP_ALG_MAX;
    }

    return;
}

static hi_void get_tscipher_algm_type(const hi_char *source, hi_unf_otp_alg_type *algm_type)
{
    if (case_strcmp("csa2", source)) {
        *algm_type = HI_UNF_OTP_ALG_CSA2;
    } else if (case_strcmp("csa3", source)) {
        *algm_type = HI_UNF_OTP_ALG_CSA3;
    } else if (case_strcmp("aes", source)) {
        *algm_type = HI_UNF_OTP_ALG_AES;
    } else if (case_strcmp("sm4", source)) {
        *algm_type = HI_UNF_OTP_ALG_SM4;
    } else if (case_strcmp("tdes", source)) {
        *algm_type = HI_UNF_OTP_ALG_TDES;
    } else if (case_strcmp("multi2", source)) {
        *algm_type = HI_UNF_OTP_ALG_MULTI2;
    } else {
        *algm_type = HI_UNF_OTP_ALG_MAX;
    }

    return;
}

static hi_void get_type(hi_s32 argc, hi_char *argv[],
                        hi_unf_otp_module_type *module_type,
                        hi_unf_otp_alg_type *algm_type)
{
    if (argv[0x2] == HI_NULL || argv[0x3] == HI_NULL) {
        sample_printf("argv[2] or argv[3] is NULL\n");
        goto out;
    }

    if (module_type == HI_NULL || algm_type == HI_NULL) {
        sample_printf("module_type or algm_type is NULL\n");
        goto out;
    }

    if (case_strcmp("mcipher", argv[0x2])) {
        *module_type = HI_UNF_OTP_MODULE_MCIPHER;
        get_mcipher_algm_type(argv[0x3], algm_type);
    } else if (case_strcmp("tscipher", argv[0x2])) {
        *module_type = HI_UNF_OTP_MODULE_TSCIPHER;
        get_tscipher_algm_type(argv[0x3], algm_type);
    } else {
        *module_type = HI_UNF_OTP_MODULE_MAX;
    }

out:
    return;
}

static hi_s32 get_hard_only_stat(hi_s32 argc, hi_char *argv[])
{
    hi_s32 ret;
    hi_bool stat = HI_FAILURE;
    hi_unf_otp_module_type module_type = HI_UNF_OTP_MODULE_MAX;
    hi_unf_otp_alg_type algm_type = HI_UNF_OTP_ALG_MAX;

    get_type(argc, argv, &module_type, &algm_type);
    if (module_type == HI_UNF_OTP_MODULE_MAX || algm_type == HI_UNF_OTP_ALG_MAX) {
        sample_printf("Don't have module type or algm type\n");
        ret = HI_FAILURE;
        goto out;
    }

    ret = hi_unf_otp_get_hard_only_stat(module_type, algm_type, &stat);
    if (ret != HI_SUCCESS) {
        sample_printf("Failed to get hard only stat, ret = 0x%x \n", ret);
        goto out;
    }

    sample_printf("Get hard only stat: %d\n", stat);

out:
    return ret;
}

static hi_s32 set_enable_hard_only(hi_s32 argc, hi_char *argv[])
{
    hi_s32 ret;
    hi_unf_otp_module_type module_type = HI_UNF_OTP_MODULE_MAX;
    hi_unf_otp_alg_type algm_type = HI_UNF_OTP_ALG_MAX;

    get_type(argc, argv, &module_type, &algm_type);
    if (module_type == HI_UNF_OTP_MODULE_MAX || algm_type == HI_UNF_OTP_ALG_MAX) {
        sample_printf("Don't have module type or algm type\n");
        ret = HI_FAILURE;
        goto out;
    }

    ret = hi_unf_otp_hard_only_enable(module_type, algm_type);
    if (ret != HI_SUCCESS) {
        sample_printf("Failed to enable hard only, ret = 0x%x \n", ret);
        goto out;
    }

out:
    return ret;
}

hi_s32 main(int argc, char *argv[])
{
    hi_s32 ret;

    if (argc < 0x4) {
        sample_printf("sample parameter error.\n");
        ret = HI_ERR_OTP_INVALID_PARA;
        goto out1;
    }

    if (case_strcmp("help", argv[1])) {
        show_usage(g_otp_sample_data, sizeof(g_otp_sample_data) / sizeof(g_otp_sample_data[0]));
        ret = HI_SUCCESS;
        goto out0;
    }

    ret = hi_unf_otp_init();
    if (ret != HI_SUCCESS) {
        sample_printf("OTP init failed, ret = 0x%x \n", ret);
        goto out1;
    }

    ret = run_cmdline(argc, argv, g_otp_sample_data, sizeof(g_otp_sample_data) / sizeof(g_otp_sample_data[0]));

    (hi_void)hi_unf_otp_deinit();

out1:
    show_returne_msg(g_otp_sample_data, sizeof(g_otp_sample_data) / sizeof(g_otp_sample_data[0]), ret);
out0:
    return ret;
}
