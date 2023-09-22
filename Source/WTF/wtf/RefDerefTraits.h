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

#include <wtf/Forward.h>
#include <wtf/RefTrackingToken.h>

namespace WebCore {
// MARK: DOM base classes
class EventTarget;
class Node;
class ContainerNode;

// MARK: Documents
class Document;
class FTPDirectoryDocument;
class HTMLDocument;
class ImageDocument;
class MediaDocument;
class ModelDocument;
class PluginDocument;
class SinkDocument;
class SVGDocument;
class TextDocument;
class XMLDocument;

class CDATASection;
class CharacterData;
class Comment;
class ProcessingInstruction;
class Text;
class Attr;
class DocumentFragment;
class DocumentType;
class ShadowRoot;

// MARK: Elements
class Element;
class StyledElement;

// MARK: HTML Elements
class HTMLElement;
class AttachmentAssociatedElement;
class HTMLAnchorElement;
class HTMLAreaElement;
class HTMLArticleElement;
class HTMLAttachmentElement;
class HTMLAudioElement;
class HTMLBDIElement;
class HTMLBRElement;
class HTMLBaseElement;
class HTMLBodyElement;
class HTMLButtonElement;
class HTMLCanvasElement;
class HTMLDListElement;
class HTMLDataElement;
class HTMLDataListElement;
class HTMLDialogElement;
class HTMLDirectoryElement;
class HTMLDivElement;
class HTMLEmbedElement;
class HTMLFieldSetElement;
class HTMLFontElement;
class HTMLFormControlElement;
class HTMLFormElement;
class HTMLFrameElement;
class HTMLFrameOwnerElement;
class HTMLFrameSetElement;
class HTMLHRElement;
class HTMLHeadElement;
class HTMLHeadingElement;
class HTMLHtmlElement;
class HTMLIFrameElement;
class HTMLImageElement;
class HTMLInputElement;
class HTMLLIElement;
class HTMLLabelElement;
class HTMLLegendElement;
class HTMLLinkElement;
class HTMLMapElement;
class HTMLMarqueeElement;
class HTMLMaybeFormAssociatedCustomElement;
class HTMLMediaElement;
class HTMLMenuElement;
class HTMLMetaElement;
class HTMLMeterElement;
class HTMLModElement;
class HTMLOListElement;
class HTMLObjectElement;
class HTMLOptGroupElement;
class HTMLOptionElement;
class HTMLOrForeignElement;
class HTMLOutputElement;
class HTMLParagraphElement;
class HTMLParamElement;
class HTMLPictureElement;
class HTMLPlugInElement;
class HTMLPlugInImageElement;
class HTMLPreElement;
class HTMLProgressElement;
class HTMLQuoteElement;
class HTMLScriptElement;
class HTMLSelectElement;
class HTMLSlotElement;
class HTMLSourceElement;
class HTMLSpanElement;
class HTMLStyleElement;
class HTMLSummaryElement;
class HTMLTableCaptionElement;
class HTMLTableCellElement;
class HTMLTableColElement;
class HTMLTableElement;
class HTMLTablePartElement;
class HTMLTableRowElement;
class HTMLTableSelectionElement;
class HTMLTemplateElement;
class HTMLTextAreaElement;
class HTMLTextFormControlElement;
class HTMLTimeElement;
class HTMLTitleElement;
class HTMLTrackElement;
class HTMLUListElement;
class HTMLUnknownElement;
class HTMLVideoElement;
class HTMLWBRElement;
class AutoFillButtonElement;
class DataListButtonElement;
class DateTimeEditElement;
class DateTimeFieldElement;
class DateTimeNumericFieldElement;
class DateTimeSymbolicFieldElement;
class DetailsMarkerElement;
class MEdiaControlTextTrackContainerElement;
class ProgressShadowElement;
class SliderThumbElement;
class SpinButtonElement;
class TextPlaceholderElement;

// MARK: SVG Elements
class SVGElement;
class SVGGraphicsElement;
class SVGAElement;
class SVGAltGlyphElement;
class SVGAltGlyphDefElement;
class SVGAltGlyphItemElement;
class SVGAnimateElement;
class SVGAnimateElementBase;
class SVGAnimateMotionElement;
class SVGAnimateTransformElement;
class SVGAnimationElement;
class SVGCircleElement;
class SVGClipPathElement;
class SVGComponentTransferFunctionElement;
class SVGCursorElement;
class SVGDefsElement;
class SVGDescElement;
class SVGEllipseElement;
class SVGFEBlendElement;
class SVGFEColorMatrixElement;
class SVGFEComponentTransferElement;
class SVGCompositeElement;
class SVGFEConvolveMatrixElement;
class SVGFEDiffuseLightingElement;
class SVGFEDisplacementMapElement;
class SVGFEDistantLightElement;
class SVGFEDropShadowElement;
class SVGFEDropShadowElement;
class SVGFEFloodElement;
class SVGFEFuncAElement;
class SVGFEFuncBElement;
class SVGFEFuncGElement;
class SVGFEFuncRElement;
class SVGFEGaussianBlurElement;
class SVGFEImageElement;
class SVGFELightElement;
class SVGFEMergeElement;
class SVGFEMergeNodeElement;
class SVGFEMorphologyElement;
class SVGFEOffsetElement;
class SVGFEPointLightElement;
class SVGFESpecularLightingElement;
class SVGFESpotLightElement;
class SVGFETileElement;
class SVGFETurbulenceElement;
class SVGFilterElement;
class SVGFilterPrimitiveStandardAttributes;
class SVGFontElement;
class SVGFontFaceElement;
class SVGFontFaceFormatElement;
class SVGFontFaceNameElement;
class SVGFontFaceSrcElement;
class SVGFontFaceUriElement;
class SVGForeignObjectElement;
class SVGGElement;
class SVGGeometryElement;
class SVGGlyphElement;
class SVGGlyphRefElement;
class SVGGradientElement;
class SVGGraphicsElement;
class SVGHKernElement;
class SVGImageElement;
class SVGLineElement;
class SVGLinearGradientElement;
class SVGMPathElement;
class SVGMarkerElement;
class SVGMaskElement;
class SVGMetadataElement;
class SVGMissingGlyphElement;
class SVGPathElement;
class SVGPatternElement;
class SVGPolyElement;
class SVGPolygonElement;
class SVGPolylineElement;
class SVGRadialGradientElement;
class SVGRectElement;
class SVGSMILElement;
class SVGSVGElement;
class SVGScriptElement;
class SVGSetElement;
class SVGStopElement;
class SVGStyleElement;
class SVGSwitchElement;
class SVGSymbolElement;
class SVGTRefElement;
class SVGTSpanElement;
class SVGTextContentElement;
class SVGTextElement;
class SVGTextPathElement;
class SVGTextPositioningElement;
class SVGTitleElement;
class SVGUnknownElement;
class SVGUseElement;
class SVGVKernElement;
class SVGViewElement;

// MARK: Images
class Image;
class GeneratedImage;
class SVGImage;
class SVGImageForContainer;
class SVGResourceImage;
class PDFDocumentImage;
class BitmapImage;
class CustomPaintImage;
class GradientImage;
class CrossfadeGeneratedImage;
class NamedImageGeneratedImage;
}

namespace WebKit {
class RemoteLayerTreeNode;
}

namespace WTF {

template <typename T> concept RefCountingSmartPtr = T::isRef || T::isRefPtr;
template <typename T> concept RefTracked = RefDerefTraits<T>::isRefTracked;
template <typename T> concept RefTrackingSmartPtr = RefCountingSmartPtr<T> && RefTracked<typename T::ValueType>;

// Default Traits - this should cover almost all uses of Ref/RefPtr.
// This can be specialized if you want different behavior for a default Ref<T> or RefPtr<T>.
// For instance instrumenting Ref/Deref call pairs for leak checking (see RefTrackingTraits below).
template <typename T>
struct RefDerefTraits {
    static ALWAYS_INLINE T& ref(T& object)
    {
        object.ref();
        return object;
    }

    static ALWAYS_INLINE T* refIfNotNull(T* ptr)
    {
        if (LIKELY(ptr != nullptr))
            ptr->ref();
        return ptr;
    }

    static ALWAYS_INLINE void derefIfNotNull(T* ptr)
    {
        if (LIKELY(ptr != nullptr))
            ptr->deref();
    }

    static void adoptRef(auto*) { }
    static void swapRef(RefCountingSmartPtr auto&) { }
    static void moveRef(auto&) { }
};

// RefTrackingTraits allow for intrumenting ref/deref pairs. Helpful for finding leaks and circular references
// of ref counted objects. See <documentation-url>.
// Defined in RefTracker.h to avoid circular dependencies between Ref/RefPtr and the RefTracker's HashMap and Vector.
struct RefTrackingTraits {
    static constexpr bool isRefTracked = true;
    ALWAYS_INLINE void ref(auto&);
    ALWAYS_INLINE void refIfNotNull(auto*);
    ALWAYS_INLINE void derefIfNotNull(auto*);
    ALWAYS_INLINE void adoptRef(auto*);
    ALWAYS_INLINE void swapRef(RefTrackingSmartPtr auto&);
    ALWAYS_INLINE void swapRef(const RefCountingSmartPtr auto&);
    ALWAYS_INLINE void moveRef(RefTrackingSmartPtr auto&);
    ALWAYS_INLINE void moveRef(const RefCountingSmartPtr auto&);

    ALWAYS_INLINE const RefTrackingToken& refTrackingToken() const;
    ALWAYS_INLINE RefTrackingToken& refTrackingToken();
private:
    RefTrackingToken m_refTrackingToken;
};

struct EventTargetRefDerefTraits {
    static constexpr bool isRefTracked = false;
    static ALWAYS_INLINE auto& ref(auto& object)
    {
        object.ref();
        return object;
    }

    static ALWAYS_INLINE auto* refIfNotNull(auto* ptr)
    {
        if (LIKELY(ptr != nullptr))
            ptr->ref();
        return ptr;
    }

    static ALWAYS_INLINE void derefIfNotNull(auto* ptr)
    {
        if (LIKELY(ptr != nullptr))
            ptr->deref();
    }

    static ALWAYS_INLINE void adoptRef(auto*) { }

    static ALWAYS_INLINE void swapRef(RefTrackingSmartPtr auto&);
    static ALWAYS_INLINE void swapRef(RefCountingSmartPtr auto&) { }

    static ALWAYS_INLINE void moveRef(RefTrackingSmartPtr auto&);
    static ALWAYS_INLINE void moveRef(const RefCountingSmartPtr auto&) { }
};

template<> struct RefDerefTraits<const WebCore::EventTarget> : public EventTargetRefDerefTraits { };
template<> struct RefDerefTraits<WebCore::EventTarget> : public RefDerefTraits<const WebCore::EventTarget> { };

#define DEFINE_REF_TRACKING_TRAITS_FOR(anObject) \
template<> struct RefDerefTraits<const anObject> : public RefTrackingTraits { }; \
template<> struct RefDerefTraits<anObject> : public RefDerefTraits<const anObject> { }

DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::Node);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::ContainerNode);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::Document);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::FTPDirectoryDocument);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLDocument);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::ImageDocument);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::MediaDocument);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::ModelDocument);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::PluginDocument);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SinkDocument);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGDocument);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::TextDocument);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::XMLDocument);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::Element);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::StyledElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::AttachmentAssociatedElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLAnchorElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLAreaElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLArticleElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLAttachmentElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLAudioElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLBDIElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLBRElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLBaseElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLBodyElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLButtonElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLCanvasElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLDListElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLDataElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLDataListElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLDialogElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLDirectoryElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLDivElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLEmbedElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLFieldSetElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLFontElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLFormControlElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLFormElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLFrameElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLFrameOwnerElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLFrameSetElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLHRElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLHeadElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLHeadingElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLHtmlElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLIFrameElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLImageElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLInputElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLLIElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLLabelElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLLegendElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLLinkElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLMapElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLMarqueeElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLMaybeFormAssociatedCustomElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLMediaElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLMenuElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLMetaElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLMeterElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLModElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLOListElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLObjectElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLOptGroupElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLOptionElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLOrForeignElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLOutputElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLParagraphElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLParamElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLPictureElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLPlugInElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLPlugInImageElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLPreElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLProgressElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLQuoteElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLScriptElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLSelectElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLSlotElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLSourceElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLSpanElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLStyleElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLSummaryElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLTableCaptionElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLTableCellElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLTableColElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLTableElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLTablePartElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLTableRowElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLTableSelectionElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLTemplateElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLTextAreaElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLTextFormControlElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLTimeElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLTitleElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLTrackElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLUListElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLUnknownElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLVideoElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::HTMLWBRElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::AutoFillButtonElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::DataListButtonElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::DateTimeEditElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::DateTimeFieldElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::DateTimeNumericFieldElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::DateTimeSymbolicFieldElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::DetailsMarkerElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::MEdiaControlTextTrackContainerElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::ProgressShadowElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SliderThumbElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SpinButtonElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::TextPlaceholderElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGGraphicsElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGAElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGAltGlyphElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGAltGlyphDefElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGAltGlyphItemElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGAnimateElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGAnimateElementBase);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGAnimateMotionElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGAnimateTransformElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGAnimationElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGCircleElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGClipPathElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGComponentTransferFunctionElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGCursorElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGDefsElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGDescElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGEllipseElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGFEBlendElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGFEColorMatrixElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGFEComponentTransferElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGCompositeElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGFEConvolveMatrixElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGFEDiffuseLightingElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGFEDisplacementMapElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGFEDistantLightElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGFEDropShadowElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGFEFloodElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGFEFuncAElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGFEFuncBElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGFEFuncGElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGFEFuncRElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGFEGaussianBlurElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGFEImageElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGFELightElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGFEMergeElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGFEMergeNodeElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGFEMorphologyElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGFEOffsetElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGFEPointLightElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGFESpecularLightingElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGFESpotLightElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGFETileElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGFETurbulenceElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGFilterElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGFilterPrimitiveStandardAttributes);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGFontElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGFontFaceElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGFontFaceFormatElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGFontFaceNameElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGFontFaceSrcElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGFontFaceUriElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGForeignObjectElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGGElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGGeometryElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGGlyphElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGGlyphRefElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGGradientElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGHKernElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGImageElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGLineElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGLinearGradientElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGMPathElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGMarkerElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGMaskElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGMetadataElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGMissingGlyphElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGPathElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGPatternElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGPolyElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGPolygonElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGPolylineElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGRadialGradientElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGRectElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGSMILElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGSVGElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGScriptElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGSetElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGStopElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGStyleElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGSwitchElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGSymbolElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGTRefElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGTSpanElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGTextContentElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGTextElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGTextPathElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGTextPositioningElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGTitleElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGUnknownElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGUseElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGVKernElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGViewElement);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::Image);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::GeneratedImage);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGImage);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGImageForContainer);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::SVGResourceImage);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::PDFDocumentImage);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::BitmapImage);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::CustomPaintImage);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::GradientImage);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::CrossfadeGeneratedImage);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::NamedImageGeneratedImage);

DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::CDATASection);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::CharacterData);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::Comment);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::ProcessingInstruction);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::Text);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::Attr);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::DocumentFragment);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::DocumentType);
DEFINE_REF_TRACKING_TRAITS_FOR(WebCore::ShadowRoot);

DEFINE_REF_TRACKING_TRAITS_FOR(WebKit::RemoteLayerTreeNode);
} // namespace WTF

using WTF::RefDerefTraits;
using WTF::RefCountingSmartPtr;
using WTF::RefTrackingSmartPtr;
