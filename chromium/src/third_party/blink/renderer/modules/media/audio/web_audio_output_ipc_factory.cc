// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/public/web/modules/media/audio/web_audio_output_ipc_factory.h"

#include <utility>

#include "base/bind.h"
#include "base/check_op.h"
#include "base/task/single_thread_task_runner.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "third_party/blink/public/common/browser_interface_broker_proxy.h"
#include "third_party/blink/public/mojom/media/renderer_audio_output_stream_factory.mojom-blink.h"
#include "third_party/blink/public/platform/platform.h"
#include "third_party/blink/renderer/modules/media/audio/mojo_audio_output_ipc.h"
#include "third_party/blink/renderer/platform/wtf/hash_map.h"
#include "third_party/blink/renderer/platform/wtf/std_lib_extras.h"

namespace blink {

class WebAudioOutputIPCFactory::Impl {
 public:
  using StreamFactoryMap = WTF::HashMap<
      uint64_t,
      mojo::Remote<mojom::blink::RendererAudioOutputStreamFactory>>;

  explicit Impl(scoped_refptr<base::SingleThreadTaskRunner> io_task_runner)
      : io_task_runner_(std::move(io_task_runner)) {}

  Impl(const Impl&) = delete;
  Impl& operator=(const Impl&) = delete;

  ~Impl() { DCHECK(factory_remotes_.IsEmpty()); }

  mojom::blink::RendererAudioOutputStreamFactory* GetRemoteFactory(
      const blink::LocalFrameToken& frame_token) const;

  void RegisterRemoteFactoryOnIOThread(
      const blink::LocalFrameToken& frame_token,
      mojo::PendingRemote<mojom::blink::RendererAudioOutputStreamFactory>
          factory_pending_remote);

  void MaybeDeregisterRemoteFactoryOnIOThread(
      const blink::LocalFrameToken& frame_token);

  // Maps frame id to the corresponding factory.
  StreamFactoryMap factory_remotes_;
  const scoped_refptr<base::SingleThreadTaskRunner> io_task_runner_;
};

// static
WebAudioOutputIPCFactory& WebAudioOutputIPCFactory::GetInstance() {
  DEFINE_THREAD_SAFE_STATIC_LOCAL(WebAudioOutputIPCFactory, instance,
                                  (Platform::Current()->GetIOTaskRunner()));
  return instance;
}

WebAudioOutputIPCFactory::WebAudioOutputIPCFactory(
    scoped_refptr<base::SingleThreadTaskRunner> io_task_runner)
    : impl_(std::make_unique<Impl>(std::move(io_task_runner))) {}

WebAudioOutputIPCFactory::~WebAudioOutputIPCFactory() = default;

std::unique_ptr<media::AudioOutputIPC>
WebAudioOutputIPCFactory::CreateAudioOutputIPC(
    const blink::LocalFrameToken& frame_token) const {
  // Unretained is safe due to the contract at the top of the header file.
  return std::make_unique<MojoAudioOutputIPC>(
      base::BindRepeating(&WebAudioOutputIPCFactory::Impl::GetRemoteFactory,
                          base::Unretained(impl_.get()), frame_token),
      io_task_runner());
}

void WebAudioOutputIPCFactory::RegisterRemoteFactory(
    const blink::LocalFrameToken& frame_token,
    blink::BrowserInterfaceBrokerProxy* interface_broker) {
  mojo::PendingRemote<mojom::blink::RendererAudioOutputStreamFactory>
      factory_remote;
  interface_broker->GetInterface(
      factory_remote.InitWithNewPipeAndPassReceiver());
  // Unretained is safe due to the contract at the top of the header file.
  // It's safe to pass the |factory_remote| PendingRemote between threads.
  io_task_runner()->PostTask(
      FROM_HERE,
      base::BindOnce(
          &WebAudioOutputIPCFactory::Impl::RegisterRemoteFactoryOnIOThread,
          base::Unretained(impl_.get()), frame_token,
          std::move(factory_remote)));
}

void WebAudioOutputIPCFactory::MaybeDeregisterRemoteFactory(
    const blink::LocalFrameToken& frame_token) {
  io_task_runner()->PostTask(
      FROM_HERE, base::BindOnce(&WebAudioOutputIPCFactory::Impl::
                                    MaybeDeregisterRemoteFactoryOnIOThread,
                                base::Unretained(impl_.get()), frame_token));
}

const scoped_refptr<base::SingleThreadTaskRunner>&
WebAudioOutputIPCFactory::io_task_runner() const {
  return impl_->io_task_runner_;
}

mojom::blink::RendererAudioOutputStreamFactory*
WebAudioOutputIPCFactory::Impl::GetRemoteFactory(
    const blink::LocalFrameToken& frame_token) const {
  DCHECK(io_task_runner_->BelongsToCurrentThread());
  auto it = factory_remotes_.find(LocalFrameToken::Hasher()(frame_token));
  return it == factory_remotes_.end() ? nullptr : it->value.get();
}

void WebAudioOutputIPCFactory::Impl::RegisterRemoteFactoryOnIOThread(
    const blink::LocalFrameToken& frame_token,
    mojo::PendingRemote<mojom::blink::RendererAudioOutputStreamFactory>
        factory_pending_remote) {
  DCHECK(io_task_runner_->BelongsToCurrentThread());
  mojo::Remote<mojom::blink::RendererAudioOutputStreamFactory> factory_remote(
      std::move(factory_pending_remote));

  auto emplace_result = factory_remotes_.insert(
      LocalFrameToken::Hasher()(frame_token), std::move(factory_remote));

  DCHECK(emplace_result.is_new_entry) << "Attempt to register a factory for a "
                                         "frame which already has a factory "
                                         "registered.";

  auto& emplaced_factory = emplace_result.stored_value->value;
  DCHECK(emplaced_factory.is_bound())
      << "Factory is not bound to a remote implementation.";

  // Unretained is safe because |this| owns the remote, so a connection error
  // cannot trigger after destruction.
  emplaced_factory.set_disconnect_handler(base::BindOnce(
      &WebAudioOutputIPCFactory::Impl::MaybeDeregisterRemoteFactoryOnIOThread,
      base::Unretained(this), frame_token));
}

void WebAudioOutputIPCFactory::Impl::MaybeDeregisterRemoteFactoryOnIOThread(
    const blink::LocalFrameToken& frame_token) {
  DCHECK(io_task_runner_->BelongsToCurrentThread());
  // This function can be called both by the frame and the connection error
  // handler of the factory remote. Calling erase multiple times even though
  // there is nothing to erase is safe, so we don't have to handle this in any
  // particular way.
  factory_remotes_.erase(LocalFrameToken::Hasher()(frame_token));
}

}  // namespace blink
