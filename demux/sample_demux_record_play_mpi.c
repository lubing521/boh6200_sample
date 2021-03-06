/*
 * Copyright (C) Hisilicon Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: Define functions used to test function of demux driver
 * Author: sdk
 * Create: 2019-5-25
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include "hi_unf_system.h"
#include "hi_mpi_demux.h"

#define SAMPLE_GET_INPUTCMD(input_cmd) fgets((char *)(input_cmd), (sizeof(input_cmd) - 1), stdin)
#define HI_GET_INPUTCMD(input_cmd) fgets((char *)(input_cmd), (sizeof(input_cmd) - 1), stdin)

#define MAX_THREAD_NUM (32)

FILE               *g_ts_in_file = HI_NULL;
hi_handle          g_ts_buf;
static pthread_t   g_ts_put_thread;
static pthread_mutex_t g_ts_mutex;
static hi_bool     g_stop_ts_put_thread = HI_FALSE;
static hi_bool     g_stop_ts_get_thread = HI_FALSE;

typedef struct {
    FILE *out_file;
    pthread_t thread;
    hi_handle handle;
} rx_thread_info;

static rx_thread_info g_play_thread[MAX_THREAD_NUM];
static rx_thread_info g_record_thread[MAX_THREAD_NUM];

static hi_handle g_flt_handle[MAX_THREAD_NUM];

hi_void ts_tx_thread(hi_void *args)
{
    hi_u32                readlen;
    hi_s32                ret;
    dmx_ram_buffer        buf;

    printf("ts send start!\n");

    while (!g_stop_ts_put_thread) {
        pthread_mutex_lock(&g_ts_mutex);

        ret = hi_mpi_dmx_ram_get_buffer(g_ts_buf, 188 * 200, &buf, 1000);
        if (ret != HI_SUCCESS) {
            pthread_mutex_unlock(&g_ts_mutex);
            printf("ramport get buffer failed!\n");
            continue;
        }

        readlen = fread(buf.data, sizeof(hi_s8), buf.length, g_ts_in_file);
        if (readlen <= 0) {
            printf("read ts file end and rewind!\n");
            rewind(g_ts_in_file);
            pthread_mutex_unlock(&g_ts_mutex);
            continue;
        }

        ret = hi_mpi_dmx_ram_put_buffer(g_ts_buf, buf.length, 0);
        if (ret != HI_SUCCESS) {
            printf("call hi_unf_dmx_put_tsbuffer failed.\n");
        }

        pthread_mutex_unlock(&g_ts_mutex);

    }

    printf("ts send end!\n");

    ret = hi_mpi_dmx_ram_reset_buffer(g_ts_buf);
    if (ret != HI_SUCCESS) {
        printf("call hi_unf_dmx_reset_tsbuffer failed.\n");
    }

    return;
}

hi_void ts_play_rx_thread(hi_void *args)
{
    hi_s32 ret;
    hi_s32 i;
    hi_u32 acquire_num = 10;
    hi_u32 acquired_num;
    dmx_buffer st_buf[10]; /* max buffer number is 10 */
    rx_thread_info *thread_info = (rx_thread_info *)args;

    printf("receive start!\n");

    while (!g_stop_ts_get_thread) {
        if (HI_SUCCESS != hi_mpi_dmx_play_acquire_buf(thread_info->handle, acquire_num, &acquired_num, st_buf, 5000)) {
            usleep(10 * 1000);
            continue;
        }

        if (acquired_num > 10) {
            printf("acquired_num[0x%x] invalid!\n", acquired_num);
            usleep(10 * 1000);
            continue;
        }

        for (i = 0; i < acquired_num; i++) {
            ret = fwrite(st_buf[i].data, 1, st_buf[i].length, thread_info->out_file);
            if (st_buf[i].length != ret) {
                printf("fwrite failed, ret[0x%x], st_buf[i].length[0x%x]\n", ret, st_buf[i].length);
            }
        }

        ret = hi_mpi_dmx_play_release_buf(thread_info->handle, acquired_num, st_buf);
        if (HI_SUCCESS != ret) {
            printf("call hi_mpi_dmx_release_buf failed!\n");
        }
    }
    printf("receive over!\n");
    return;
}

hi_void ts_record_rx_thread(hi_void *args)
{
    hi_u32 ret = HI_FAILURE;
    dmx_buffer st_buf;
    rx_thread_info *thread_info = (rx_thread_info *)args;

    printf("receive start!\n");

    while (!g_stop_ts_get_thread) {
        ret = hi_mpi_dmx_rec_acquire_buf(thread_info->handle, 500 * 188, &st_buf, 5000);
        if (ret != HI_SUCCESS ) {
            usleep(10 * 1000);
            continue;
        }

        ret = fwrite(st_buf.data, 1, st_buf.length, thread_info->out_file);
        if (st_buf.length != ret) {
            printf("fwrite failed, ret[0x%x], st_buf.length[0x%x]\n", ret, st_buf.length);
        }

        ret = hi_mpi_dmx_rec_release_buf(thread_info->handle, &st_buf);
        if (HI_SUCCESS != ret) {
            printf("call hi_mpi_dmx_rec_release_buf failed!\n");
        }
    }
    printf("receive over!\n");
    return;
}

hi_s32 main(hi_s32 argc, hi_char *argv[])
{
    hi_s32 ret = HI_FAILURE;
    hi_handle  band_handle;
    hi_handle  pid_ch_handle;
    hi_char input_cmd[32];
    hi_s32 pid = -1;
    dmx_ram_port_attr ram_attrs;
    dmx_port_attr port_attrs;
    dmx_play_attrs play_attrs;
    dmx_band_attr  band_entry;
    dmx_rec_attrs rec_attrs;
    hi_char file_name[128];
    dmx_play_type play_type = DMX_PLAY_TYPE_MAX;
    hi_u32 i;
    hi_u32 j;
    hi_u32 thread_num;
    dmx_filter_attrs flt_attrs;

    if (argc != 5) {
        printf("\nuseage: savets ts_file pid pid_type thread_num\n");
        printf("\ncreate pid play and record tsfile from ts stream through pid.\n");
        printf("\nargument:\n");
        printf("    ts_file : source ts file.\n");
        printf("    pid     : the pid of video or audio.\n");
        printf("    pid_type: sec/pes/aud/vid.\n");
        printf("  thread_num: thread num.\n");
        return -1;
    }

    if (strstr(argv[2], "0x") || strstr(argv[2], "0X")) {
        pid = strtoul(argv[2], NULL, 16);
    } else {
        pid = strtoul(argv[2], NULL, 10);
    }

    if (strcmp(argv[3], "sec") == 0) {
        play_type = DMX_PLAY_TYPE_SEC;
    } else if (strcmp(argv[3], "pes") == 0) {
        play_type = DMX_PLAY_TYPE_PES;
    } else if (strcmp(argv[3], "aud") == 0) {
        play_type = DMX_PLAY_TYPE_AUD;
    } else if (strcmp(argv[3], "vid") == 0) {
        play_type = DMX_PLAY_TYPE_VID;
    } else {
        printf("invalid pid type %s.\n", argv[3]);
        return -1;
    }

    if (strstr(argv[4], "0x") || strstr(argv[4], "0X")) {
        thread_num = strtoul(argv[4], NULL, 16);
    } else {
        thread_num = strtoul(argv[4], NULL, 10);
    }

    if (thread_num > MAX_THREAD_NUM) {
        printf("invalid thread num %u.\n", thread_num);
        return -1;
    }

    g_ts_in_file = fopen(argv[1], "rb");
    if (g_ts_in_file == HI_NULL) {
        printf("open input ts file failed!\n");
        goto out;
    }

    for (i = 0; i < thread_num; i++) {
        sprintf(file_name, "%s_play_%u", argv[1], i);
        g_play_thread[i].out_file = fopen(file_name, "wb");
        if (g_play_thread[i].out_file == HI_NULL) {
            printf("open output(%s) ts file failed!\n", file_name);
            for (j = 0; j < i; j++) {
                fclose(g_play_thread[j].out_file);
            }
            goto DMX_CLOSE_IN_FILE;
        }
    }

    for (i = 0; i < thread_num; i++) {
        sprintf(file_name, "%s_record_%u", argv[1], i);
        g_record_thread[i].out_file = fopen(file_name, "wb");
        if (g_record_thread[i].out_file == HI_NULL) {
            printf("open ouput(%s) ts file failed!\n", file_name);
            for (j = 0; j < i; j++) {
                fclose(g_record_thread[j].out_file);
            }
            goto DMX_CLOSE_PLAY_FILE;
        }
    }

    ret = hi_unf_sys_init();
    if (ret != HI_SUCCESS) {
        printf("call hi_unf_sys_init failed.\n");
        goto DMX_CLOSE_REC_FILE;
    }

    ret = hi_mpi_dmx_init();
    if (ret != HI_SUCCESS) {
        printf("call hi_mpi_dmx_init failed.\n");
        goto DMX_SYS_DEINIT;
    }

    ret = hi_mpi_dmx_ram_get_port_default_attrs(&ram_attrs);
    if (ret != HI_SUCCESS) {
        printf("call hi_mpi_dmx_ram_get_port_default_attrs failed.\n");
        goto DMX_DEINIT;
    }

    ret = hi_mpi_dmx_ram_open_port(DMX_RAM_PORT_0, &ram_attrs, &g_ts_buf);
    if (ret != HI_SUCCESS) {
        printf("call hi_mpi_dmx_ram_open_port failed.\n");
        goto DMX_DEINIT;
    }

    ret = hi_mpi_dmx_ram_get_port_attrs(g_ts_buf, &port_attrs);
    if (ret != HI_SUCCESS) {
        printf("call hi_mpi_dmx_ram_get_port_attrs failed.\n");
        goto DMX_RAM_CLOSE;
    }

    ret = hi_mpi_dmx_ram_set_port_attrs(g_ts_buf, &port_attrs);
    if (ret != HI_SUCCESS) {
        printf("call hi_mpi_dmx_ram_set_port_attrs failed.\n");
        goto DMX_RAM_CLOSE;
    }

    band_entry.band_attr = 0;
    ret = hi_mpi_dmx_band_open(DMX_BAND_0, &band_entry, &band_handle);
    if (ret != HI_SUCCESS) {
        printf("call dmx_api_utils_band_get_handle failed.\n");
        goto DMX_RAM_CLOSE;
    }

    ret = hi_mpi_dmx_band_attach_port(band_handle, DMX_RAM_PORT_0);
    if (ret != HI_SUCCESS) {
        printf("call hi_mpi_dmx_band_attach_port failed.\n");
        goto DMX_BAND_CLOSE;
    }


    ret = hi_mpi_dmx_pid_ch_create(band_handle, pid, &pid_ch_handle);
    if (ret != HI_SUCCESS) {
        printf("call hi_mpi_dmx_pid_ch_create failed.\n");
        goto DMX_BAND_DETACH;
    }

    ret = hi_mpi_dmx_play_get_default_attrs(&play_attrs);
    if (ret != HI_SUCCESS) {
        printf("call hi_mpi_dmx_play_get_default_attrs failed.\n");
        goto DMX_PIDCH_DESTROY;
    }

    play_attrs.type = play_type;

    for (i = 0; i < thread_num; i++) {
        ret = hi_mpi_dmx_play_create(&play_attrs, &g_play_thread[i].handle);
        if (ret != HI_SUCCESS) {
            printf("call hi_mpi_dmx_play_create failed, ret(%x).\n", ret);
            for (j = 0; j < i; j++) {
                hi_mpi_dmx_play_close(g_play_thread[j].handle);
                if (play_attrs.type == DMX_PLAY_TYPE_SEC || play_attrs.type == DMX_PLAY_TYPE_PES) {
                    hi_mpi_dmx_play_del_filter(g_play_thread[j].handle, g_flt_handle[j]);
                    hi_mpi_dmx_play_destroy_filter(g_flt_handle[j]);
                }
                hi_mpi_dmx_play_detach_pid_ch(g_play_thread[j].handle);
                hi_mpi_dmx_play_destroy(g_play_thread[j].handle);
            }
            goto DMX_PIDCH_DESTROY;
        }

        ret = hi_mpi_dmx_play_attach_pid_ch(g_play_thread[i].handle, pid_ch_handle);
        if (ret != HI_SUCCESS) {
            printf("call hi_mpi_dmx_play_attach_pid_ch failed, ret(%x).\n", ret);
            hi_mpi_dmx_play_destroy(g_play_thread[i].handle);
            for (j = 0; j < i; j++) {
                hi_mpi_dmx_play_close(g_play_thread[j].handle);
                if (play_attrs.type == DMX_PLAY_TYPE_SEC || play_attrs.type == DMX_PLAY_TYPE_PES) {
                    hi_mpi_dmx_play_del_filter(g_play_thread[j].handle, g_flt_handle[j]);
                    hi_mpi_dmx_play_destroy_filter(g_flt_handle[j]);
                }
                hi_mpi_dmx_play_detach_pid_ch(g_play_thread[j].handle);
                hi_mpi_dmx_play_destroy(g_play_thread[j].handle);
            }
            goto DMX_PIDCH_DESTROY;
        }

        if (play_attrs.type == DMX_PLAY_TYPE_SEC || play_attrs.type == DMX_PLAY_TYPE_PES) {
            flt_attrs.depth = 1;
            memset(flt_attrs.mask, 0x0, DMX_FILTER_MAX_DEPTH);
            memset(flt_attrs.match, 0x0, DMX_FILTER_MAX_DEPTH);
            if (play_attrs.type == DMX_PLAY_TYPE_PES) {
                memset(flt_attrs.negate, 0x1, DMX_FILTER_MAX_DEPTH);
            } else {
                memset(flt_attrs.negate, 0x0, DMX_FILTER_MAX_DEPTH);
                flt_attrs.match[0] = 2; /* only support pmt table id */
            }

            hi_mpi_dmx_play_create_filter(&flt_attrs, &g_flt_handle[i]);
            hi_mpi_dmx_play_add_filter(g_play_thread[i].handle, g_flt_handle[i]);
        }

        ret = hi_mpi_dmx_play_open(g_play_thread[i].handle);
        if (ret != HI_SUCCESS) {
            printf("call hi_mpi_dmx_play_open failed, ret(%x).\n", ret);
            hi_mpi_dmx_play_detach_pid_ch(g_play_thread[i].handle);
            if (play_attrs.type == DMX_PLAY_TYPE_SEC || play_attrs.type == DMX_PLAY_TYPE_PES) {
                    hi_mpi_dmx_play_del_filter(g_play_thread[i].handle, g_flt_handle[i]);
                    hi_mpi_dmx_play_destroy_filter(g_flt_handle[i]);
            }
            hi_mpi_dmx_play_destroy(g_play_thread[i].handle);
            for (j = 0; j < i; j++) {
                hi_mpi_dmx_play_close(g_play_thread[j].handle);
                hi_mpi_dmx_play_detach_pid_ch(g_play_thread[j].handle);
                if (play_attrs.type == DMX_PLAY_TYPE_SEC || play_attrs.type == DMX_PLAY_TYPE_PES) {
                    hi_mpi_dmx_play_del_filter(g_play_thread[j].handle, g_flt_handle[j]);
                    hi_mpi_dmx_play_destroy_filter(g_flt_handle[j]);
                }
                hi_mpi_dmx_play_destroy(g_play_thread[j].handle);
            }
            goto DMX_PIDCH_DESTROY;
        }
    }

    ret = hi_mpi_dmx_rec_get_default_attrs(&rec_attrs);
    if (ret != HI_SUCCESS) {
        printf("call hi_mpi_dmx_rec_get_default_attrs failed!\n");
        goto DMX_PLY_EXIT;
    }

    rec_attrs.rec_type = DMX_REC_TYPE_SELECT_PID;

    for (i = 0; i < thread_num; i++) {
        ret = hi_mpi_dmx_rec_create(&rec_attrs, &g_record_thread[i].handle);
        if (ret != HI_SUCCESS) {
            printf("call hi_mpi_dmx_rec_create failed, ret(%x).\n", ret);
            for (j = 0; j < i; j++) {
                hi_mpi_dmx_rec_close(g_record_thread[j].handle);
                hi_mpi_dmx_rec_del_all_ch(g_record_thread[j].handle);
                hi_mpi_dmx_rec_destroy(g_record_thread[j].handle);
            }
            goto DMX_PLY_EXIT;
        }

        ret = hi_mpi_dmx_rec_add_ch(g_record_thread[i].handle, pid_ch_handle);
        if (ret != HI_SUCCESS) {
            printf("call hi_mpi_dmx_rec_add_ch failed, ret(%x).\n", ret);
            hi_mpi_dmx_rec_destroy(g_record_thread[i].handle);
            for (j = 0; j < i; j++) {
                hi_mpi_dmx_rec_close(g_record_thread[j].handle);
                hi_mpi_dmx_rec_del_all_ch(g_record_thread[j].handle);
                hi_mpi_dmx_rec_destroy(g_record_thread[j].handle);
            }
            goto DMX_PLY_EXIT;
        }

        ret = hi_mpi_dmx_rec_open(g_record_thread[i].handle);
        if (ret != HI_SUCCESS) {
            printf("call hi_mpi_dmx_rec_open failed, ret(%x).\n", ret);
            hi_mpi_dmx_rec_del_all_ch(g_record_thread[i].handle);
            hi_mpi_dmx_rec_destroy(g_record_thread[i].handle);
            for (j = 0; j < i; j++) {
                hi_mpi_dmx_rec_close(g_record_thread[j].handle);
                hi_mpi_dmx_rec_del_all_ch(g_record_thread[j].handle);
                hi_mpi_dmx_rec_destroy(g_record_thread[j].handle);
            }
            goto DMX_PLY_EXIT;
        }
    }

    pthread_mutex_init(&g_ts_mutex, NULL);
    g_stop_ts_put_thread = HI_FALSE;
    g_stop_ts_get_thread = HI_FALSE;

    pthread_create(&g_ts_put_thread, HI_NULL, (hi_void *)ts_tx_thread, HI_NULL);

    for (i = 0; i < thread_num; i++) {
        pthread_create(&g_play_thread[i].thread, HI_NULL, (hi_void *)ts_play_rx_thread, (hi_void *)&g_play_thread[i]);
        pthread_create(&g_record_thread[i].thread, HI_NULL, (hi_void *)ts_record_rx_thread, (hi_void *)&g_record_thread[i]);
    }

    printf("please input 'q' to quit!\n");
    while (1) {
        SAMPLE_GET_INPUTCMD(input_cmd);
        if ('q' == input_cmd[0]) {
            printf("prepare to quit!\n");
            break;
        }
    }

    g_stop_ts_put_thread = HI_TRUE;
    g_stop_ts_get_thread = HI_TRUE;
    pthread_join(g_ts_put_thread, HI_NULL);
    for (i = 0; i < thread_num; i++) {
        pthread_join(g_play_thread[i].thread, HI_NULL);
        pthread_join(g_record_thread[i].thread, HI_NULL);
    }

    pthread_mutex_destroy(&g_ts_mutex);

    for (i = 0; i < thread_num; i++) {
        hi_mpi_dmx_rec_close(g_record_thread[i].handle);
        hi_mpi_dmx_rec_del_all_ch(g_record_thread[i].handle);
        hi_mpi_dmx_rec_destroy(g_record_thread[i].handle);
    }

DMX_PLY_EXIT:
    for (i = 0; i < thread_num; i++) {
        hi_mpi_dmx_play_close(g_play_thread[i].handle);

        if (play_attrs.type == DMX_PLAY_TYPE_SEC || play_attrs.type == DMX_PLAY_TYPE_PES) {
            hi_mpi_dmx_play_del_filter(g_play_thread[i].handle, g_flt_handle[i]);
            hi_mpi_dmx_play_destroy_filter(g_flt_handle[i]);
        }

        hi_mpi_dmx_play_detach_pid_ch(g_play_thread[i].handle);

        hi_mpi_dmx_play_destroy(g_play_thread[i].handle);
    }

DMX_PIDCH_DESTROY:
    hi_mpi_dmx_pid_ch_destroy(pid_ch_handle);

DMX_BAND_DETACH:
    hi_mpi_dmx_band_detach_port(band_handle);

DMX_BAND_CLOSE:
    hi_mpi_dmx_band_close(band_handle);

DMX_RAM_CLOSE:
    hi_mpi_dmx_ram_close_port(g_ts_buf);

DMX_DEINIT:
    hi_mpi_dmx_de_init();

DMX_SYS_DEINIT:
    hi_unf_sys_deinit();

DMX_CLOSE_REC_FILE:
    for (i = 0; i < thread_num; i++) {
        fclose(g_record_thread[i].out_file);
    }

DMX_CLOSE_PLAY_FILE:
    for (i = 0; i < thread_num; i++) {
        fclose(g_play_thread[i].out_file);
    }
DMX_CLOSE_IN_FILE:
    if (g_ts_in_file != HI_NULL) {
        fclose(g_ts_in_file);
        g_ts_in_file = HI_NULL;
    }

out:
    return 0;
}


