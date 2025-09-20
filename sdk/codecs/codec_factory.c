#include "audio_codec.h"
#include "opus_codec.h"
#include "codec_stub.h"
#include "../log/linx_log.h"
#include <stdlib.h>
#include <string.h>

// 支持的编解码器类型数组（根据平台动态确定）
static const codec_type_t supported_codecs[] = {
#if defined(__APPLE__) || defined(__linux__)
    CODEC_TYPE_OPUS,        // macOS/Linux 支持 Opus
#endif
    CODEC_TYPE_STUB         // 所有平台都支持 stub
};

static const char* codec_names[] = {
    [CODEC_TYPE_OPUS] = "Opus Software Codec",
    [CODEC_TYPE_ES8388] = "ES8388 Hardware Codec",
    [CODEC_TYPE_STUB] = "Stub Codec (No-op)"
};

// 创建编解码器实例
audio_codec_t* codec_factory_create(codec_type_t type) {
    switch (type) {
        case CODEC_TYPE_OPUS:
#if defined(__APPLE__) || defined(__linux__)
            LOG_INFO("Creating Opus codec");
            return opus_codec_create();
#else
            LOG_ERROR("Opus codec not available on this platform");
            return NULL;
#endif
        
        case CODEC_TYPE_STUB:
            LOG_INFO("Creating stub codec");
            return codec_stub_create();
        
        default:
            LOG_ERROR("Unsupported codec type: %d", type);
            return NULL;
    }
}

// 销毁编解码器实例
void codec_factory_destroy(audio_codec_t* codec) {
    if (!codec) {
        LOG_WARN("Attempting to destroy NULL codec");
        return;
    }
    
    if (codec->vtable && codec->vtable->destroy) {
        codec->vtable->destroy(codec);
    } else {
        LOG_WARN("Codec has no destroy function, freeing directly");
        free(codec);
    }
}

// 获取编解码器名称
const char* codec_factory_get_name(codec_type_t type) {
    if (type >= 0 && type < CODEC_TYPE_COUNT) {
        return codec_names[type];
    }
    return "Unknown";
}

// 获取支持的编解码器数量
int codec_factory_get_supported_count(void) {
    return sizeof(supported_codecs) / sizeof(supported_codecs[0]);
}

// 获取支持的编解码器类型列表
codec_type_t* codec_factory_get_supported_types(void) {
    static codec_type_t types[sizeof(supported_codecs) / sizeof(supported_codecs[0])];
    
    for (size_t i = 0; i < sizeof(supported_codecs) / sizeof(supported_codecs[0]); i++) {
        types[i] = supported_codecs[i];
    }
    
    return types;
}