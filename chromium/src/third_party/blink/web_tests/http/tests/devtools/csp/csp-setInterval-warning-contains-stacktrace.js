// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(async function() {
    TestRunner.addResult(
      `This test should trigger a CSP violation by attempting to evaluate a string with setInterval.\n`);
  await TestRunner.loadModule('console'); await TestRunner.loadTestModule('console_test_runner');

  await TestRunner.loadHTML(`
    <!DOCTYPE html>
    <meta http-equiv='Content-Security-Policy' content="script-src 'self' 'unsafe-inline'">
  `);

  await TestRunner.evaluateInPagePromise(`
      function thisTest() {
          setInterval('log("FAIL")', 0);
      }
  `);
  await TestRunner.evaluateInPagePromise(`thisTest()`);
  ConsoleTestRunner.dumpStackTraces();
  TestRunner.completeTest();
})();
