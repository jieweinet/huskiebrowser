// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(async function() {
  TestRunner.addResult(`Test the Timeline instrumentation contains Profile event.`);
  await TestRunner.loadModule('timeline'); await TestRunner.loadTestModule('performance_test_runner');
  await TestRunner.showPanel('timeline');

  await PerformanceTestRunner.evaluateWithTimeline('42');
  const profileEvent = PerformanceTestRunner.findTimelineEvent(TimelineModel.TimelineModel.RecordType.Profile);
  TestRunner.check(profileEvent, 'Profile trace event not found.');

  TestRunner.completeTest();
})();
