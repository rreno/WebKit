/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#if HAVE(WEBGPU_IMPLEMENTATION)

#include <WebGPU/WebGPU.h>
#include <WebGPU/WebGPUExt.h>
#include <type_traits>
#include <wtf/RefPtr.h>
#include <wtf/RefTracker.h>

namespace WebCore::WebGPU {

template<typename WGPUT>
struct WebGPUPtrTraits {
    using StorageType = WGPUT;

    template<typename U>
    static ALWAYS_INLINE StorageType exchange(StorageType& ptr, U&& newValue) { return std::exchange(ptr, newValue); }

    static ALWAYS_INLINE void swap(StorageType& a, StorageType& b) { std::swap(a, b); }
    static ALWAYS_INLINE WGPUT unwrap(const StorageType& ptr) { return ptr; }

    static StorageType hashTableDeletedValue() { return std::bit_cast<StorageType>(static_cast<uintptr_t>(-1)); }
    static ALWAYS_INLINE bool isHashTableDeletedValue(const StorageType& ptr) { return ptr == hashTableDeletedValue(); }
};

template <typename T, void (*reference)(T), void(*release)(T)> struct BaseWebGPURefDerefTraits {
    static ALWAYS_INLINE T refIfNotNull(T t)
    {
        if (LIKELY(t))
            reference(t);
        return t;
    }

    static ALWAYS_INLINE void derefIfNotNull(T t)
    {
        if (LIKELY(t))
            release(t);
    }

    static void adoptRef(auto*) { }
    static void swapRef(auto&) { }
    static void moveRef(auto&) { }
};

template <typename> struct WebGPURefDerefTraits;

template <> struct WebGPURefDerefTraits<WGPUAdapterImpl> : public BaseWebGPURefDerefTraits<WGPUAdapter, wgpuAdapterReference, wgpuAdapterRelease> { };
template <> struct WebGPURefDerefTraits<WGPUBindGroupImpl> : public BaseWebGPURefDerefTraits<WGPUBindGroup, wgpuBindGroupReference, wgpuBindGroupRelease> { };
template <> struct WebGPURefDerefTraits<WGPUBindGroupLayoutImpl> : public BaseWebGPURefDerefTraits<WGPUBindGroupLayout, wgpuBindGroupLayoutReference, wgpuBindGroupLayoutRelease> { };
template <> struct WebGPURefDerefTraits<WGPUBufferImpl> : public BaseWebGPURefDerefTraits<WGPUBuffer, wgpuBufferReference, wgpuBufferRelease> { };
template <> struct WebGPURefDerefTraits<WGPUCommandBufferImpl> : public BaseWebGPURefDerefTraits<WGPUCommandBuffer, wgpuCommandBufferReference, wgpuCommandBufferRelease> { };
template <> struct WebGPURefDerefTraits<WGPUCommandEncoderImpl> : public BaseWebGPURefDerefTraits<WGPUCommandEncoder, wgpuCommandEncoderReference, wgpuCommandEncoderRelease> { };
template <> struct WebGPURefDerefTraits<WGPUComputePassEncoderImpl> : public BaseWebGPURefDerefTraits<WGPUComputePassEncoder, wgpuComputePassEncoderReference, wgpuComputePassEncoderRelease> { };
template <> struct WebGPURefDerefTraits<WGPUComputePipelineImpl> : public BaseWebGPURefDerefTraits<WGPUComputePipeline, wgpuComputePipelineReference, wgpuComputePipelineRelease> { };
template <> struct WebGPURefDerefTraits<WGPUDeviceImpl> : public BaseWebGPURefDerefTraits<WGPUDevice, wgpuDeviceReference, wgpuDeviceRelease> { };
template <> struct WebGPURefDerefTraits<WGPUInstanceImpl> : public BaseWebGPURefDerefTraits<WGPUInstance, wgpuInstanceReference, wgpuInstanceRelease> { };
template <> struct WebGPURefDerefTraits<WGPUPipelineLayoutImpl> : public BaseWebGPURefDerefTraits<WGPUPipelineLayout, wgpuPipelineLayoutReference, wgpuPipelineLayoutRelease> { };
template <> struct WebGPURefDerefTraits<WGPUQuerySetImpl> : public BaseWebGPURefDerefTraits<WGPUQuerySet, wgpuQuerySetReference, wgpuQuerySetRelease> { };
template <> struct WebGPURefDerefTraits<WGPUQueueImpl> : public BaseWebGPURefDerefTraits<WGPUQueue, wgpuQueueReference, wgpuQueueRelease> { };
template <> struct WebGPURefDerefTraits<WGPURenderBundleImpl> : public BaseWebGPURefDerefTraits<WGPURenderBundle, wgpuRenderBundleReference, wgpuRenderBundleRelease> { };
template <> struct WebGPURefDerefTraits<WGPURenderBundleEncoderImpl> : public BaseWebGPURefDerefTraits<WGPURenderBundleEncoder, wgpuRenderBundleEncoderReference, wgpuRenderBundleEncoderRelease> { };
template <> struct WebGPURefDerefTraits<WGPURenderPassEncoderImpl> : public BaseWebGPURefDerefTraits<WGPURenderPassEncoder, wgpuRenderPassEncoderReference, wgpuRenderPassEncoderRelease> { };
template <> struct WebGPURefDerefTraits<WGPURenderPipelineImpl> : public BaseWebGPURefDerefTraits<WGPURenderPipeline, wgpuRenderPipelineReference, wgpuRenderPipelineRelease> { };
template <> struct WebGPURefDerefTraits<WGPUSamplerImpl> : public BaseWebGPURefDerefTraits<WGPUSampler, wgpuSamplerReference, wgpuSamplerRelease> { };
template <> struct WebGPURefDerefTraits<WGPUShaderModuleImpl> : public BaseWebGPURefDerefTraits<WGPUShaderModule, wgpuShaderModuleReference, wgpuShaderModuleRelease> { };
template <> struct WebGPURefDerefTraits<WGPUSurfaceImpl> : public BaseWebGPURefDerefTraits<WGPUSurface, wgpuSurfaceReference, wgpuSurfaceRelease> { };
template <> struct WebGPURefDerefTraits<WGPUSwapChainImpl> : public BaseWebGPURefDerefTraits<WGPUSwapChain, wgpuSwapChainReference, wgpuSwapChainRelease> { };
template <> struct WebGPURefDerefTraits<WGPUTextureImpl> : public BaseWebGPURefDerefTraits<WGPUTexture, wgpuTextureReference, wgpuTextureRelease> { };
template <> struct WebGPURefDerefTraits<WGPUTextureViewImpl> : public BaseWebGPURefDerefTraits<WGPUTextureView, wgpuTextureViewReference, wgpuTextureViewRelease> { };
template <> struct WebGPURefDerefTraits<WGPUExternalTextureImpl> : public BaseWebGPURefDerefTraits<WGPUExternalTexture, wgpuExternalTextureReference, wgpuExternalTextureRelease> { };
template <> struct WebGPURefDerefTraits<WGPUXRBindingImpl> : public BaseWebGPURefDerefTraits<WGPUXRBinding, wgpuXRBindingReference, wgpuXRBindingRelease> { };
template <> struct WebGPURefDerefTraits<WGPUXRProjectionLayerImpl> : public BaseWebGPURefDerefTraits<WGPUXRProjectionLayer, wgpuXRProjectionLayerReference, wgpuXRProjectionLayerRelease> { };
template <> struct WebGPURefDerefTraits<WGPUXRSubImageImpl> : public BaseWebGPURefDerefTraits<WGPUXRSubImage, wgpuXRSubImageReference, wgpuXRSubImageRelease> { };
template <> struct WebGPURefDerefTraits<WGPUXRViewImpl> : public BaseWebGPURefDerefTraits<WGPUXRView, wgpuXRViewReference, wgpuXRViewRelease> { };

template <typename T> using WebGPUPtr = RefPtr<std::remove_pointer_t<T>, WebGPUPtrTraits<T>, WebGPURefDerefTraits>;

template <typename T> inline WebGPUPtr<T> adoptWebGPU(T t)
{
    return adoptRef<std::remove_pointer_t<T>, WebGPUPtrTraits<T>, WebGPURefDerefTraits>(t);
}

} // namespace WebCore::WebGPU

#endif // HAVE(WEBGPU_IMPLEMENTATION)
