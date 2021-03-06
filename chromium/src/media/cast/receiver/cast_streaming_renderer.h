// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_CAST_RECEIVER_CAST_STREAMING_RENDERER_H_
#define MEDIA_CAST_RECEIVER_CAST_STREAMING_RENDERER_H_

#include "media/base/renderer.h"

namespace media {
namespace cast {

// This class defined a Renderer to be used with Cast streaming. Currently,
// this functionality is limited to the Cast Mirroring scenario, specifically to
// streams generated by screen or tab mirroring (i.e., that do not use the media
// remoting protocol). The Initialize call is delegated to the |real_renderer_|
// while all other calls are no-ops.
//
// The mirroring can be summarized as being that of a user is trying to take
// what's currently displayed on their device, and send those bits to a larger
// screen. So it should be an exact duplicate of what they see locally - a
// faithful copy. Streams generated by mirroring screen contents from the sender
// device consist of demuxed media frames, and are simply played out in real
// time on the delegated Renderer. Media commands on this renderer have no
// effect and are dropped.
//
// Currently, this renderer is only used on Chromecast receivers.
//
// NOTE: Currently, this renderer does NOT start playback of the media stream
// it receives
// TODO(b/187332037): Implement starting media playback.
class CastStreamingRenderer : public Renderer {
 public:
  // |renderer| is the Renderer to which the Initialize() call should be
  // delegated.
  explicit CastStreamingRenderer(std::unique_ptr<Renderer> renderer);
  CastStreamingRenderer(const CastStreamingRenderer&) = delete;
  CastStreamingRenderer(CastStreamingRenderer&&) = delete;

  ~CastStreamingRenderer() override;

  CastStreamingRenderer& operator=(const CastStreamingRenderer&) = delete;
  CastStreamingRenderer& operator=(CastStreamingRenderer&&) = delete;

  // Renderer overrides.
  //
  // Calls into |real_renderer_|'s method of the same name.
  void Initialize(MediaResource* media_resource,
                  RendererClient* client,
                  PipelineStatusCallback init_cb) override;

  // Further Renderer overrides as no-ops.
  void SetCdm(CdmContext* cdm_context, CdmAttachedCB cdm_attached_cb) override;
  void SetLatencyHint(absl::optional<base::TimeDelta> latency_hint) override;
  void Flush(base::OnceClosure flush_cb) override;
  void StartPlayingFrom(base::TimeDelta time) override;
  void SetPlaybackRate(double playback_rate) override;
  void SetVolume(float volume) override;
  base::TimeDelta GetMediaTime() override;

 private:
  std::unique_ptr<Renderer> real_renderer_;
};

}  // namespace cast
}  // namespace media

#endif  // MEDIA_CAST_RECEIVER_CAST_STREAMING_RENDERER_H_
