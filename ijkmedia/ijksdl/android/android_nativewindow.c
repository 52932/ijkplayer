/*****************************************************************************
 * android_nativewindow.c
 *****************************************************************************
 *
 * Copyright (c) 2013 Bilibili
 * copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
 *
 * This file is part of ijkPlayer.
 *
 * ijkPlayer is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * ijkPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with ijkPlayer; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "android_nativewindow.h"

#include <assert.h>
#include <android/native_window.h>
#include "../ijksdl_vout.h"
#include "../ijksdl_vout_internal.h"
#include "../ffmpeg/ijksdl_inc_ffmpeg.h"
#include "ijksdl_inc_internal_android.h"

#ifdef SDLTRACE
#undef SDLTRACE
#define SDLTRACE(...)
#endif

static int android_render_yv12_on_yv12(ANativeWindow_Buffer *out_buffer, const SDL_VoutOverlay *overlay)
{
    // SDLTRACE("SDL_VoutAndroid: android_render_yv12_on_yv12(%p)", overlay);
    assert(overlay->format == SDL_FCC_YV12);
    assert(overlay->planes == 3);

    int min_height = IJKMIN(out_buffer->height, overlay->h);
    int dst_y_stride = out_buffer->stride;
    int dst_c_stride = IJKALIGN(out_buffer->stride / 2, 16);
    int dst_y_size = dst_y_stride * out_buffer->height;
    int dst_c_size = dst_c_stride * out_buffer->height / 2;

    // ALOGE("stride:%d/%d, size:%d/%d", dst_y_stride, dst_c_stride, dst_y_size, dst_c_size);

    uint8_t *dst_pixels_array[] = {
        out_buffer->bits,
        out_buffer->bits + dst_y_size,
        out_buffer->bits + dst_y_size + dst_c_size,
    };
    int dst_line_height[] = { min_height, min_height / 2, min_height / 2 };
    int dst_line_size_array[] = { dst_y_stride, dst_c_stride, dst_c_stride };

    for (int i = 0; i < 3; ++i) {
        int dst_line_size = dst_line_size_array[i];
        int src_line_size = overlay->pitches[i];
        int line_height = dst_line_height[i];
        uint8_t *dst_pixels = dst_pixels_array[i];
        const uint8_t *src_pixels = overlay->pixels[i];

        if (dst_line_size == src_line_size) {
            int plane_size = src_line_size * line_height;

            // ALOGE("sdl_image_copy_plane %p %p %d", dst_pixels, src_pixels, dst_plane_size);
            memcpy(dst_pixels, src_pixels, plane_size);
        } else {
            // TODO: 9 padding
            int bytewidth = IJKMIN(dst_line_size, src_line_size);

            // ALOGE("av_image_copy_plane %p %d %p %d %d %d", dst_pixels, dst_line_size, src_pixels, src_line_size, bytewidth, line_height);
            av_image_copy_plane(dst_pixels, dst_line_size, src_pixels, src_line_size, bytewidth, line_height);
        }
    }

    return 0;
}

static int android_render_on_yv12(ANativeWindow_Buffer *out_buffer, const SDL_VoutOverlay *overlay)
{
    assert(out_buffer);
    assert(overlay);

    switch (overlay->format) {
    case SDL_FCC_YV12: {
        return android_render_yv12_on_yv12(out_buffer, overlay);
    }
    }

    return -1;
}

static int android_render_rgb_on_rgb(ANativeWindow_Buffer *out_buffer, const SDL_VoutOverlay *overlay, int bpp)
{
    // SDLTRACE("SDL_VoutAndroid: android_render_rgb_on_rgb(%p)", overlay);
    assert(overlay->format == SDL_FCC_RV16);
    assert(overlay->planes == 1);

    // 展示的最小高度
    int min_height = IJKMIN(out_buffer->height, overlay->h);
    // 获得窗口缓冲区一行在内存中接受的像素个数
    int dst_stride = out_buffer->stride;
    // 获得一行被展示的数据的字节个数
    int src_line_size = overlay->pitches[0];
    // 计算窗口缓冲区展示一行数据的字节个数。bpp为一个像素字节数，如果是RGB32表示每个像素用32比特位表示，所以计算字节总数时需要乘32
    int dst_line_size = dst_stride * bpp / 8;

    // 指针指向窗口缓冲区地址
    uint8_t *dst_pixels = out_buffer->bits;
    // 存储一帧像素的首地址
    const uint8_t *src_pixels = overlay->pixels[0];

    // 如果展示数据大小等于窗口需要大小，则直接拷贝到窗口缓存区中
    if (dst_line_size == src_line_size) {
        int plane_size = src_line_size * min_height;
        // ALOGE("android_render_rgb_on_rgb (pix-match) %p %p %d", dst_pixels, src_pixels, plane_size);
        // 直接拷贝到窗口缓存区中
        memcpy(dst_pixels, src_pixels, plane_size);
    } else {
        // TODO: 9 padding
        int bytewidth = IJKMIN(dst_line_size, src_line_size);// 获得一个最小的宽度

        // 将展示数据拷贝到窗口缓冲区，
        // 如果展示数据>窗口缓冲区的大小，就不完全展示图片
        // 否则就窗口就缩放
        // ALOGE("android_render_rgb_on_rgb (pix-mismatch) %p %d %p %d %d %d", dst_pixels, dst_line_size, src_pixels, src_line_size, bytewidth, min_height);
        av_image_copy_plane(dst_pixels, dst_line_size, src_pixels, src_line_size, bytewidth, min_height);
    }

    return 0;
}

static int android_render_rgb565_on_rgb565(ANativeWindow_Buffer *out_buffer, const SDL_VoutOverlay *overlay)
{
    return android_render_rgb_on_rgb(out_buffer, overlay, 16);
}

static int android_render_on_rgb565(ANativeWindow_Buffer *out_buffer, const SDL_VoutOverlay *overlay)
{
    assert(out_buffer);
    assert(overlay);

    switch (overlay->format) {
    case SDL_FCC_RV16: {
        return android_render_rgb565_on_rgb565(out_buffer, overlay);
    }
    }

    return -1;
}

static int android_render_rgb32_on_rgb8888(ANativeWindow_Buffer *out_buffer, const SDL_VoutOverlay *overlay)
{
    return android_render_rgb_on_rgb(out_buffer, overlay, 32);
}

static int android_render_on_rgb8888(ANativeWindow_Buffer *out_buffer, const SDL_VoutOverlay *overlay)
{
    assert(out_buffer);
    assert(overlay);

    switch (overlay->format) {
    case SDL_FCC_RV32: {
        return android_render_rgb32_on_rgb8888(out_buffer, overlay);
    }
    }

    return -1;
}

typedef struct AndroidHalFourccDescriptor {
    Uint32 fcc_or_hal;
    const char* name;

    int hal_format;

    int (*render)(ANativeWindow_Buffer *native_buffer, const SDL_VoutOverlay *overlay);
} AndroidHalFourccDescriptor;

static AndroidHalFourccDescriptor g_hal_fcc_map[] = {
    // YV12
    { HAL_PIXEL_FORMAT_YV12, "HAL_YV12", HAL_PIXEL_FORMAT_YV12, android_render_on_yv12 },
    { SDL_FCC_YV12, "YV12", HAL_PIXEL_FORMAT_YV12, android_render_on_yv12 },

    // RGB565
    { HAL_PIXEL_FORMAT_RGB_565, "HAL_RGB_565", HAL_PIXEL_FORMAT_RGB_565, android_render_on_rgb565 },
    { SDL_FCC_RV16, "RV16", HAL_PIXEL_FORMAT_RGB_565, android_render_on_rgb565 },

    // RGB8888
    { HAL_PIXEL_FORMAT_RGBX_8888, "HAL_RGBX_8888", HAL_PIXEL_FORMAT_RGBX_8888, android_render_on_rgb8888 },
    { HAL_PIXEL_FORMAT_RGBA_8888, "HAL_RGBA_8888", HAL_PIXEL_FORMAT_RGBA_8888, android_render_on_rgb8888 },
    { HAL_PIXEL_FORMAT_BGRA_8888, "HAL_BGRA_8888", HAL_PIXEL_FORMAT_BGRA_8888, android_render_on_rgb8888 },
    { SDL_FCC_RV32, "RV32", HAL_PIXEL_FORMAT_RGBX_8888, android_render_on_rgb8888 },
};

/*
 * 获得播放格式对应的操作结构体
 *
 * 由于系统会支持播放很多视频格式，而不同格式的视频数据会做一些相同操作，如都需要向窗口缓存区发送一帧数据，
 * 所以不同视频格式的数据发送的方式会有不同。所以系统自定义了一些结构体，捆绑了格式与对应的操作。
 */
AndroidHalFourccDescriptor *native_window_get_desc(int fourcc_or_hal)
{
    // // 通过播放格式遍历寻找对应的结构体
    for (int i = 0; i < NELEM(g_hal_fcc_map); ++i) {
        AndroidHalFourccDescriptor *desc = &g_hal_fcc_map[i];
        if (desc->fcc_or_hal == fourcc_or_hal)
            return desc;
    }

    return NULL;
}
// TODO ijk默认渲染模式
int SDL_Android_NativeWindow_display_l(ANativeWindow *native_window, SDL_VoutOverlay *overlay)
{
    int retval;

    if (!native_window)
        return -1;

    if (!overlay) {
        ALOGE("SDL_Android_NativeWindow_display_l: NULL overlay");
        return -1;
    }

    if (overlay->w <= 0 || overlay->h <= 0) {
        ALOGE("SDL_Android_NativeWindow_display_l: invalid overlay dimensions(%d, %d)", overlay->w, overlay->h);
        return -1;
    }
    // 获取展示的屏幕宽高
    int curr_w = ANativeWindow_getWidth(native_window);
    int curr_h = ANativeWindow_getHeight(native_window);
    // 获得展示窗口需要的视频格式（图片）
    int curr_format = ANativeWindow_getFormat(native_window);
    int buff_w = IJKALIGN(overlay->w, 2);
    int buff_h = IJKALIGN(overlay->h, 2);

    // 获得格式对应的结构体
    // overlay->format=842225234 对应的AndroidHalFourccDescriptor名字---RV32
    AndroidHalFourccDescriptor *overlayDesc = native_window_get_desc(overlay->format);
    if (!overlayDesc) {
        ALOGE("SDL_Android_NativeWindow_display_l: unknown overlay format: %d", overlay->format);
        return -1;
    }
    // curr_format=2 对应的AndroidHalFourccDescriptor名字---HAL_RGBX_8888
    AndroidHalFourccDescriptor *voutDesc = native_window_get_desc(curr_format); // 获得播放格式对应的操作结构体。

    // 如果播放格式与展示窗口需要格式不同，一般设置一次
    if (!voutDesc || voutDesc->hal_format != overlayDesc->hal_format) {
        ALOGD("ANativeWindow_setBuffersGeometry: w=%d, h=%d, f=%.4s(0x%x) => w=%d, h=%d, f=%.4s(0x%x)",
            curr_w, curr_h, (char*) &curr_format, curr_format,
            buff_w, buff_h, (char*) &overlay->format, overlay->format);
        // 设置窗口缓冲区的格式和大小------LayerBuffer
        retval = ANativeWindow_setBuffersGeometry(native_window, buff_w, buff_h, overlayDesc->hal_format);
        if (retval < 0) {
            ALOGE("SDL_Android_NativeWindow_display_l: ANativeWindow_setBuffersGeometry: failed %d", retval);
            return retval;
        }

        if (!voutDesc) {
            ALOGE("SDL_Android_NativeWindow_display_l: unknown hal format %d", curr_format);
            return -1;
        }
    }

    ANativeWindow_Buffer out_buffer;
    // 锁定窗口的下一个绘图面（即缓存区）以便写入
    // 通过向out_buffer赋值，作为窗口的展示数据
    retval = ANativeWindow_lock(native_window, &out_buffer, NULL);
    if (retval < 0) {
        ALOGE("SDL_Android_NativeWindow_display_l: ANativeWindow_lock: failed %d", retval);
        return retval;
    }

    // 如果窗口缓存区宽高与展示数据不匹配，说明视频宽高发生了改变，重新窗口属性，丢弃帧或者图片
    if (out_buffer.width != buff_w || out_buffer.height != buff_h) {
        ALOGE("unexpected native window buffer (%p)(w:%d, h:%d, fmt:'%.4s'0x%x), expecting (w:%d, h:%d, fmt:'%.4s'0x%x)",
            native_window,
            out_buffer.width, out_buffer.height, (char*)&out_buffer.format, out_buffer.format,
            buff_w, buff_h, (char*)&overlay->format, overlay->format);
        // TODO: 8 set all black
        ANativeWindow_unlockAndPost(native_window);// 在锁定后解锁窗口的绘制面
        // 更改窗口缓冲区的格式和大小------LayerBuffer
        ANativeWindow_setBuffersGeometry(native_window, buff_w, buff_h, overlayDesc->hal_format);
        return -1;
    }
    // TODO 将视频帧发送到窗口缓存区中
    // 最终调用了 android/ijkplayer/ijkplayer-x86/src/main/jni/ijkmedia/ijksdl/android/android_nativewindow.c
    // android_render_rgb_on_rgb 函数中
    int render_ret = voutDesc->render(&out_buffer, overlay);
    if (render_ret < 0) {
        // TODO: 8 set all black
        // return after unlock image;
    }
    // 解锁窗口的绘制面，将新的缓冲区发送到显示器
    retval = ANativeWindow_unlockAndPost(native_window);
    if (retval < 0) {
        ALOGE("SDL_Android_NativeWindow_display_l: ANativeWindow_unlockAndPost: failed %d", retval);
        return retval;
    }

    return render_ret;
}
