// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(async function() {
  TestRunner.addResult(
      'Verifies that modules can be loaded via import() in snippets\n');
  await TestRunner.loadModule('console'); await TestRunner.loadTestModule('console_test_runner');
  await TestRunner.loadModule('sources'); await TestRunner.loadTestModule('sources_test_runner');
  await TestRunner.showPanel('sources');

  const sourceCode = `
      (async() => {
        const myModule = await import('./resources/module.js');
        console.log('myModule.message: ' + myModule.message);
      })()
      'end of snippet'`;

  const projects =
      Workspace.workspace.projectsForType(Workspace.projectTypes.FileSystem);
  const snippetsProject = projects.find(
      project => Persistence.FileSystemWorkspaceBinding.fileSystemType(
                     project) === 'snippets');
  const uiSourceCode = await snippetsProject.createFile('');

  uiSourceCode.setContent(sourceCode);
  await Common.Revealer.reveal(uiSourceCode);
  await uiSourceCode.rename('my_snippet_name');
  Sources.SourcesPanel.instance().runSnippet();

  await ConsoleTestRunner.waitUntilNthMessageReceivedPromise(2);
  await ConsoleTestRunner.dumpConsoleMessages();
  Console.ConsoleView.clearConsole();
  TestRunner.completeTest();
})();
