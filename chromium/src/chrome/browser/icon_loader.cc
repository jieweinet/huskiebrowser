// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/icon_loader.h"

#include <utility>

#include "base/bind.h"
#include "base/task/thread_pool.h"
#include "base/threading/thread_task_runner_handle.h"
#include "content/public/browser/browser_thread.h"

using content::BrowserThread;

// static
IconLoader* IconLoader::Create(const base::FilePath& file_path,
                               IconSize size,
                               float scale,
                               IconLoadedCallback callback) {
  return new IconLoader(file_path, size, scale, std::move(callback));
}

void IconLoader::Start() {
  target_task_runner_ = base::ThreadTaskRunnerHandle::Get();

  base::ThreadPool::PostTask(
      FROM_HERE, traits(),
      base::BindOnce(&IconLoader::ReadGroup, base::Unretained(this)));
}

IconLoader::IconLoader(const base::FilePath& file_path,
                       IconSize size,
                       float scale,
                       IconLoadedCallback callback)
    : file_path_(file_path),
#if !defined(OS_ANDROID)
      icon_size_(size),
#endif  // defined(OS_ANDROID)
      scale_(scale),
      callback_(std::move(callback)) {
}

IconLoader::~IconLoader() {}

#if !defined(OS_WIN)
void IconLoader::ReadGroup() {
  group_ = GroupForFilepath(file_path_);

  GetReadIconTaskRunner()->PostTask(
      FROM_HERE, base::BindOnce(&IconLoader::ReadIcon, base::Unretained(this)));
}
#endif  // !defined(OS_WIN)
