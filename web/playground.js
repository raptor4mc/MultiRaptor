const STORAGE_KEY = 'magphos-web-studio-v2';

let wasmCompiler = null;
let wasmAnalyzer = null;
let wasmLoadError = null;
const FALLBACK_BANNER = 'Repo bundled fallback loader (WASM artifact not built).';

const SCRIPT_BASE_URL = (() => {
  const scriptSrc = document.currentScript && document.currentScript.src
    ? document.currentScript.src
    : window.location.href;
  return new URL('.', scriptSrc).href;
})();

function trim(v) {
  return v.trim();
}

function normalizeFolderPath(path) {
  return trim(path).replace(/\\/g, '/').replace(/^\/+|\/+$/g, '');
}

function normalizeFilePath(path) {
  return trim(path).replace(/\\/g, '/').replace(/^\/+/, '');
}

function ensureProjectShape(raw) {
  const safe = raw && typeof raw === 'object' ? raw : {};
  const rawFiles = safe.files && typeof safe.files === 'object' ? safe.files : {};
  const files = {};
  Object.entries(rawFiles).forEach(([filePath, content]) => {
    const normalized = normalizeFilePath(filePath);
    if (!normalized) return;
    files[normalized] = typeof content === 'string' ? content : String(content ?? '');
  });
  const folders = Array.isArray(safe.folders) ? safe.folders : [];

  const normalizedFolderMeta = [];
  const seenFolders = new Set();
  folders.forEach((entry) => {
    const path = normalizeFolderPath(
      typeof entry === 'string' ? entry : (entry && entry.path) || ''
    );
    if (!path || seenFolders.has(path)) return;
    seenFolders.add(path);
    normalizedFolderMeta.push({
      path,
      createdAt: (entry && typeof entry === 'object' && entry.createdAt) || new Date().toISOString()
    });
  });

  return {
    projectName: safe.projectName || 'chromebook-studio',
    activeFile: safe.activeFile || Object.keys(files)[0] || 'main.mp',
    files,
    folders: normalizedFolderMeta
  };
}

async function loadWasmCompiler() {
  const attempts = [];
  const loaderCandidates = [
    './magphos_wasm.js', './magphos_wasm_singlefile.js',
    '../magphos_wasm.js', '../magphos_wasm_singlefile.js',
    '/magphos_wasm.js', '/magphos_wasm_singlefile.js',
    '/web/magphos_wasm.js', '/web/magphos_wasm_singlefile.js'
  ];
  const wasmCandidates = [
    null,
    './magphos_wasm.wasm.64', './magphos_wasm.wasm', './magphos.wasm.64', './magphos.wasm',
    '../magphos_wasm.wasm.64', '../magphos_wasm.wasm', '../magphos.wasm.64', '../magphos.wasm',
    '/magphos_wasm.wasm.64', '/magphos_wasm.wasm', '/magphos.wasm.64', '/magphos.wasm'
  ];
  const loaderUrls = [...new Set(loaderCandidates.map((path) => new URL(path, SCRIPT_BASE_URL).href))];
  const wasmUrls = [...new Set(wasmCandidates.map((path) => (path ? new URL(path, SCRIPT_BASE_URL).href : null)))];

  for (const loaderUrl of loaderUrls) {
    const isSingleFileLoader = loaderUrl.includes('magphos_wasm_singlefile.js');
    try {
      await validateArtifactSize(loaderUrl, isSingleFileLoader ? 1024 : 64, 'JavaScript loader');
    } catch (err) {
      attempts.push(`${loaderUrl}: ${err.message}`);
      continue;
    }

    if (isSingleFileLoader) {
      try {
        const classicModule = await loadClassicScriptModule(loaderUrl, null);
        if (typeof classicModule.compileMagPhos !== 'function') {
          throw new Error('Single-file loader initialized, but compileMagPhos export is missing.');
        }
        assertRealWasmCompiler(classicModule.compileMagPhos);
        wasmCompiler = classicModule.compileMagPhos;
        wasmAnalyzer = typeof classicModule.analyzeMagPhos === 'function'
          ? classicModule.analyzeMagPhos
          : null;
        wasmLoadError = null;
        return;
      } catch (err) {
        attempts.push(`${loaderUrl}: ${err.message}`);
        continue;
      }
    }

    for (const wasmUrl of wasmUrls) {
      try {
        if (wasmUrl) await validateArtifactSize(wasmUrl, 1024, 'WASM binary');

        let moduleFactory = null;
        const imported = await import(loaderUrl);
        if (typeof imported.default === 'function') {
          moduleFactory = imported.default;
        } else if (typeof imported.MagPhosWasmFactory === 'function') {
          moduleFactory = imported.MagPhosWasmFactory;
        }
        if (typeof moduleFactory === 'function') {
          const wasmModule = await moduleFactory({
            locateFile(path) {
              if (path.endsWith('.wasm')) return wasmUrl;
              return new URL(path, loaderUrl).href;
            }
          });
          if (typeof wasmModule.compileMagPhos !== 'function') {
            throw new Error('WASM module loaded, but compileMagPhos export is missing.');
          }
          assertRealWasmCompiler(wasmModule.compileMagPhos);
          wasmCompiler = wasmModule.compileMagPhos;
          wasmAnalyzer = typeof wasmModule.analyzeMagPhos === 'function'
            ? wasmModule.analyzeMagPhos
            : null;
          wasmLoadError = null;
          return;
        }

        const classicModule = await loadClassicScriptModule(loaderUrl, wasmUrl);
        if (typeof classicModule.compileMagPhos !== 'function') {
          throw new Error('Classic WASM loader initialized, but compileMagPhos export is missing.');
        }
        assertRealWasmCompiler(classicModule.compileMagPhos);
        wasmCompiler = classicModule.compileMagPhos;
        wasmAnalyzer = typeof classicModule.analyzeMagPhos === 'function'
          ? classicModule.analyzeMagPhos
          : null;
        wasmLoadError = null;
        return;
      } catch (err) {
        const wasmPart = wasmUrl ? ` + ${wasmUrl}` : '';
        attempts.push(`${loaderUrl}${wasmPart}: ${err.message}`);
      }
    }
  }

  wasmLoadError = attempts.join(' | ');
}

function assertRealWasmCompiler(compileFn) {
  const probeOutput = String(compileFn('print "__MAGPHOS_WASM_PROBE__"'));
  if (probeOutput.includes(FALLBACK_BANNER)) {
    throw new Error(
      'Detected bundled fallback loader instead of compiled C++ WASM artifact. Build real WASM via ./tools/scripts/build_web.sh.'
    );
  }
}

async function validateArtifactSize(url, minBytes, label) {
  const parsedUrl = new URL(url, window.location.href);
  if (parsedUrl.protocol === 'file:') {
    // `fetch(file://...)` is blocked in many browsers. For direct file usage,
    // skip size preflight and let script/module loading determine validity.
    return;
  }

  const response = await fetch(url, { cache: 'no-store' });
  if (!response.ok) {
    throw new Error(`${label} failed to load (${response.status}).`);
  }

  const contentLengthHeader = response.headers.get('content-length');
  if (contentLengthHeader) {
    const contentLength = Number(contentLengthHeader);
    if (Number.isFinite(contentLength) && contentLength > 0 && contentLength < minBytes) {
      throw new Error(`${label} looks corrupted (${contentLength} bytes).`);
    }
  }

  const bytes = await response.arrayBuffer();
  if (bytes.byteLength < minBytes) {
    throw new Error(`${label} looks corrupted (${bytes.byteLength} bytes).`);
  }
}

function loadClassicScriptModule(loaderUrl, wasmUrl) {
  return new Promise((resolve, reject) => {
    const prevModule = window.Module && typeof window.Module === 'object' ? window.Module : {};
    const prevInit = typeof prevModule.onRuntimeInitialized === 'function'
      ? prevModule.onRuntimeInitialized
      : null;

    let resolved = false;
    const settleResolve = (moduleRef) => {
      if (resolved) return;
      resolved = true;
      resolve(moduleRef);
    };
    const settleReject = (reason) => {
      if (resolved) return;
      resolved = true;
      reject(new Error(reason));
    };

    window.Module = {
      ...prevModule,
      locateFile(path) {
        if (path.endsWith('.wasm') && wasmUrl) return wasmUrl;
        return new URL(path, loaderUrl).href;
      },
      onRuntimeInitialized() {
        try {
          if (prevInit) prevInit();
        } catch (_) {
          // ignore previous callback errors
        }
        if (window.Module && typeof window.Module.compileMagPhos === 'function') {
          settleResolve(window.Module);
        } else {
          settleReject('Classic script loaded, but compileMagPhos was not exported.');
        }
      },
      onAbort(reason) {
        settleReject(`WASM aborted: ${reason || 'unknown reason'}`);
      }
    };

    const script = document.createElement('script');
    const bust = `magphos_retry=${Date.now()}_${Math.random().toString(36).slice(2)}`;
    const joiner = loaderUrl.includes('?') ? '&' : '?';
    script.src = `${loaderUrl}${joiner}${bust}`;
    script.async = true;
    script.dataset.magphosLoader = loaderUrl;
    script.addEventListener('error', () => {
      settleReject(`Failed to load script: ${loaderUrl}`);
    });
    script.addEventListener('load', () => {
      // If runtime is already initialized before callback wiring, resolve here.
      if (window.Module && typeof window.Module.compileMagPhos === 'function') {
        settleResolve(window.Module);
      }
    });
    document.head.appendChild(script);

    // Give runtime initialization callback a short chance to fire.
    setTimeout(() => {
      if (window.Module && typeof window.Module.compileMagPhos === 'function') {
        settleResolve(window.Module);
      } else {
        settleReject('Classic loader runtime did not initialize compileMagPhos.');
      }
    }, 1200);
  });
}

function compileMagPhos(source) {
  if (!wasmCompiler) {
    const why = wasmLoadError ? ` (${wasmLoadError})` : '';
    throw new Error(`WASM compiler is not available${why}. Build with MAGPHOS_BUILD_WASM using Emscripten.`);
  }
  return wasmCompiler(source);
}

function createDefaultProject() {
  const template = document.getElementById('defaultProgram').content.textContent;
  return {
    projectName: 'chromebook-studio',
    activeFile: 'main.mp',
    folders: [
      { path: 'src', createdAt: new Date().toISOString() },
      { path: 'notes', createdAt: new Date().toISOString() }
    ],
    files: {
      'main.mp': template,
      'notes/tips.mp': '# Put extra snippets here'
    }
  };
}

function loadProject() {
  try {
    const raw = localStorage.getItem(STORAGE_KEY);
    if (!raw) return createDefaultProject();
    const parsed = ensureProjectShape(JSON.parse(raw));
    Object.keys(parsed.files).forEach((filePath) => ensureParentFolders(parsed, filePath));
    if (!Object.keys(parsed.files).length) return createDefaultProject();
    return parsed;
  } catch (_) {
    return createDefaultProject();
  }
}

function saveProject(project) {
  localStorage.setItem(STORAGE_KEY, JSON.stringify(project));
}

function ensureParentFolders(project, filePath) {
  const parts = filePath.split('/');
  if (parts.length <= 1) return;
  const created = [];
  const existing = new Set(project.folders.map((f) => f.path));
  for (let i = 1; i < parts.length; i += 1) {
    const folder = parts.slice(0, i).join('/');
    if (!existing.has(folder)) {
      project.folders.push({ path: folder, createdAt: new Date().toISOString() });
      existing.add(folder);
      created.push(folder);
    }
  }
  if (created.length) {
    project.folders.sort((a, b) => a.path.localeCompare(b.path));
  }
}

const sourceEl = document.getElementById('source');
const programOutputEl = document.getElementById('programOutput');
const consoleOutputEl = document.getElementById('consoleOutput');
const terminalShellEl = document.getElementById('terminalOutput');
const terminalHistoryEl = document.getElementById('terminalHistory');
const terminalInputEl = document.getElementById('terminalInput');
const terminalPromptEl = document.getElementById('terminalPrompt');
const outputTitleEl = document.getElementById('outputTitle');
const fileListEl = document.getElementById('fileList');
const activeFileLabel = document.getElementById('activeFileLabel');
const runBtn = document.getElementById('runBtn');
const outputTabBtn = document.getElementById('outputTabBtn');
const consoleTabBtn = document.getElementById('consoleTabBtn');
const terminalTabBtn = document.getElementById('terminalTabBtn');
const deleteFolderBtn = document.getElementById('deleteFolderBtn');
runBtn.disabled = true;

let outputMode = 'output';
let terminalCwd = '';
let terminalLines = [];
let terminalHistoryIndex = -1;
const terminalCommandHistory = [];
const collapsedFolders = new Set();
let selectedFolder = '';

function setOutputMode(mode) {
  const allowedModes = new Set(['output', 'console', 'terminal']);
  outputMode = allowedModes.has(mode) ? mode : 'output';
  const showOutput = outputMode === 'output';
  const showConsole = outputMode === 'console';
  const showTerminal = outputMode === 'terminal';
  programOutputEl.hidden = !showOutput;
  consoleOutputEl.hidden = !showConsole;
  terminalShellEl.hidden = !showTerminal;
  outputTabBtn.classList.toggle('active', showOutput);
  consoleTabBtn.classList.toggle('active', showConsole);
  terminalTabBtn.classList.toggle('active', showTerminal);
  outputTitleEl.textContent = showOutput ? 'Output' : showConsole ? 'Console' : 'Terminal';
  if (showTerminal) {
    refreshTerminalPrompt();
    terminalInputEl.focus();
  }
}

function setOutputs({ output = '', console = '', terminal = '' } = {}) {
  programOutputEl.textContent = output;
  consoleOutputEl.textContent = console;
  const payload = terminal || output || console;
  pushTerminalLine(payload);
}

let project = loadProject();

function ensureActiveFile() {
  if (!project.files[project.activeFile]) {
    project.activeFile = Object.keys(project.files).sort()[0];
  }
}

function buildProjectTree(currentProject) {
  const root = { folders: {}, files: [] };

  const ensureTreeFolder = (folderPath) => {
    if (!folderPath) return root;
    const parts = folderPath.split('/').filter(Boolean);
    let cursor = root;
    parts.forEach((part) => {
      if (!cursor.folders[part]) cursor.folders[part] = { folders: {}, files: [] };
      cursor = cursor.folders[part];
    });
    return cursor;
  };

  currentProject.folders.forEach((folder) => {
    ensureTreeFolder(folder.path);
  });

  Object.keys(currentProject.files).forEach((filePath) => {
    const parts = filePath.split('/').filter(Boolean);
    const fileName = parts.pop();
    const parent = ensureTreeFolder(parts.join('/'));
    parent.files.push(fileName);
  });

  return root;
}

function renderFileList() {
  ensureActiveFile();
  fileListEl.innerHTML = '';

  const tree = buildProjectTree(project);

  const renderNode = (node, prefix = '') => {
    const sortedFolders = Object.keys(node.folders).sort((a, b) => a.localeCompare(b));
    sortedFolders.forEach((folderName) => {
      const fullPath = prefix ? `${prefix}/${folderName}` : folderName;
      const folderLi = document.createElement('li');
      folderLi.className = 'folder-item tree-item';
      const folderBtn = document.createElement('button');
      const depth = fullPath.split('/').length - 1;
      const expanded = !collapsedFolders.has(fullPath);
      folderBtn.className = 'folder-btn';
      if (selectedFolder === fullPath) folderBtn.classList.add('active');
      folderBtn.style.paddingLeft = `${12 + depth * 16}px`;
      folderBtn.textContent = `${expanded ? '📂' : '📁'} ${folderName}`;
      folderBtn.addEventListener('click', () => {
        selectedFolder = fullPath;
        if (expanded) collapsedFolders.add(fullPath);
        else collapsedFolders.delete(fullPath);
        renderFileList();
      });
      folderLi.appendChild(folderBtn);
      fileListEl.appendChild(folderLi);
      if (expanded) renderNode(node.folders[folderName], fullPath);
    });

    node.files
      .slice()
      .sort((a, b) => a.localeCompare(b))
      .forEach((fileName) => {
        const fullPath = prefix ? `${prefix}/${fileName}` : fileName;
        const li = document.createElement('li');
        const btn = document.createElement('button');
        const depth = fullPath.split('/').length - 1;
        btn.className = 'file-btn tree-item';
        btn.style.paddingLeft = `${12 + depth * 16}px`;
        if (fullPath === project.activeFile) btn.classList.add('active');
        btn.textContent = `📄 ${fileName}`;
        btn.addEventListener('click', () => {
          project.activeFile = fullPath;
          selectedFolder = '';
          syncEditorFromFile();
          renderFileList();
          saveProject(project);
        });
        li.appendChild(btn);
        fileListEl.appendChild(li);
      });
  };

  renderNode(tree);

  activeFileLabel.textContent = `Editor — ${project.activeFile}`;
}

function syncEditorFromFile() {
  sourceEl.value = project.files[project.activeFile] ?? '';
}

function syncFileFromEditor() {
  project.files[project.activeFile] = sourceEl.value;
  saveProject(project);
}

function refreshTerminalPrompt() {
  terminalPromptEl.textContent = `${terminalCwd || '~'} $`;
}

function pushTerminalLine(message = '') {
  const lines = String(message).split('\n');
  terminalLines.push(...lines);
  if (terminalLines.length > 300) {
    terminalLines = terminalLines.slice(terminalLines.length - 300);
  }
  terminalHistoryEl.textContent = terminalLines.join('\n');
  terminalHistoryEl.scrollTop = terminalHistoryEl.scrollHeight;
}

function resolveProjectPath(inputPath, basePath = terminalCwd) {
  const raw = String(inputPath || '').trim();
  if (!raw) return normalizeFolderPath(basePath || '');
  const absolute = raw.startsWith('/');
  const stack = [];
  const seed = absolute ? '' : normalizeFolderPath(basePath || '');
  if (seed) stack.push(...seed.split('/').filter(Boolean));
  raw.split('/').forEach((part) => {
    if (!part || part === '.') return;
    if (part === '..') stack.pop();
    else stack.push(part);
  });
  return stack.join('/');
}

function listDirectory(path = terminalCwd) {
  const dir = resolveProjectPath(path);
  const prefix = dir ? `${dir}/` : '';
  const folders = new Set();
  const files = [];

  project.folders.forEach(({ path: folderPath }) => {
    if (!folderPath.startsWith(prefix)) return;
    const remainder = folderPath.slice(prefix.length);
    if (!remainder || remainder.includes('/')) return;
    folders.add(remainder);
  });

  Object.keys(project.files).forEach((filePath) => {
    if (!filePath.startsWith(prefix)) return;
    const remainder = filePath.slice(prefix.length);
    if (!remainder || remainder.includes('/')) return;
    files.push(remainder);
  });

  return {
    dir,
    folders: [...folders].sort((a, b) => a.localeCompare(b)),
    files: files.sort((a, b) => a.localeCompare(b))
  };
}

function runTerminalCommand(rawCommand) {
  const command = String(rawCommand || '').trim();
  if (!command) return;
  pushTerminalLine(`${terminalPromptEl.textContent} ${command}`);

  const [name, ...args] = command.split(/\s+/);
  const cmd = name.toLowerCase();

  if (cmd === 'help') {
    pushTerminalLine('Commands: help, clear, pwd, ls [path], tree, cd <path>, cat <file>, open <file>, run [file].');
    return;
  }
  if (cmd === 'clear') {
    terminalLines = [];
    pushTerminalLine('');
    return;
  }
  if (cmd === 'pwd') {
    pushTerminalLine(`/${terminalCwd}`);
    return;
  }
  if (cmd === 'ls') {
    const result = listDirectory(args[0] || terminalCwd);
    const output = [
      ...result.folders.map((f) => `📁 ${f}`),
      ...result.files.map((f) => `📄 ${f}`)
    ];
    pushTerminalLine(output.length ? output.join('\n') : '(empty)');
    return;
  }
  if (cmd === 'tree') {
    const tree = buildProjectTree(project);
    const lines = [];
    const walk = (node, depth = 0, prefix = '') => {
      Object.keys(node.folders).sort().forEach((folder) => {
        const path = prefix ? `${prefix}/${folder}` : folder;
        lines.push(`${'  '.repeat(depth)}📁 ${folder}`);
        walk(node.folders[folder], depth + 1, path);
      });
      node.files.slice().sort().forEach((file) => lines.push(`${'  '.repeat(depth)}📄 ${file}`));
    };
    walk(tree);
    pushTerminalLine(lines.length ? lines.join('\n') : '(empty project)');
    return;
  }
  if (cmd === 'cd') {
    const target = resolveProjectPath(args[0] || '');
    const validFolder = !target || project.folders.some((f) => f.path === target);
    if (!validFolder) {
      pushTerminalLine(`cd: folder not found: ${args[0] || ''}`);
      return;
    }
    terminalCwd = target;
    refreshTerminalPrompt();
    return;
  }
  if (cmd === 'cat') {
    const target = resolveProjectPath(args[0] || '', terminalCwd);
    if (!project.files[target]) {
      pushTerminalLine(`cat: file not found: ${args[0] || ''}`);
      return;
    }
    pushTerminalLine(project.files[target]);
    return;
  }
  if (cmd === 'open') {
    const target = resolveProjectPath(args[0] || '', terminalCwd);
    if (!project.files[target]) {
      pushTerminalLine(`open: file not found: ${args[0] || ''}`);
      return;
    }
    project.activeFile = target;
    syncEditorFromFile();
    renderFileList();
    saveProject(project);
    pushTerminalLine(`Opened ${target}`);
    return;
  }
  if (cmd === 'run') {
    const target = args[0] ? resolveProjectPath(args[0], terminalCwd) : project.activeFile;
    if (!project.files[target]) {
      pushTerminalLine(`run: file not found: ${args[0] || ''}`);
      return;
    }
    project.activeFile = target;
    syncEditorFromFile();
    renderFileList();
    try {
      doRun();
      pushTerminalLine('Done.');
    } catch (err) {
      pushTerminalLine(err.message);
    }
    return;
  }

  pushTerminalLine(`Unknown command: ${name}. Type "help".`);
}

function doRun() {
  syncFileFromEditor();
  if (!wasmCompiler) {
    const why = wasmLoadError ? `\nLoader error: ${wasmLoadError}` : '';
    throw new Error(`C++ WASM compiler is not loaded yet. Build/publish web WASM artifacts and refresh.${why}`);
  }
  let analyzerLog = '';
  if (wasmAnalyzer) {
    const analysisRaw = String(wasmAnalyzer(sourceEl.value) ?? '').trim();
    if (analysisRaw) {
      if (analysisRaw === 'ok') {
        analyzerLog = 'ok';
      } else {
        try {
          const parsed = JSON.parse(analysisRaw);
          if (parsed && typeof parsed === 'object') {
            if (parsed.ok === false) {
              throw new Error(parsed.error || parsed.message || analysisRaw);
            }
            analyzerLog = JSON.stringify(parsed);
          } else {
            analyzerLog = analysisRaw;
          }
        } catch (parseErr) {
          if (parseErr instanceof SyntaxError) {
            throw new Error(analysisRaw);
          }
          throw parseErr;
        }
      }
    }
  }

  const js = compileMagPhos(sourceEl.value);
  const logs = [];
  const errorLogs = [];
  const originalLog = console.log;
  const originalError = console.error;
  console.log = (...args) => logs.push(args.join(' '));
  console.error = (...args) => errorLogs.push(args.join(' '));
  try {
    new Function(js)();
  } finally {
    console.log = originalLog;
    console.error = originalError;
  }

  const runOutput = logs.join('\n') || '(no output)';
  const runtimeErrors = errorLogs.join('\n');
  const terminalLines = [`$ mp run ${project.activeFile}`];
  if (analyzerLog) terminalLines.push(analyzerLog);
  terminalLines.push(runOutput);
  setOutputs({
    output: runOutput,
    console: runtimeErrors,
    terminal: terminalLines.join('\n')
  });
  if (runtimeErrors) {
    setOutputMode('console');
  }
  return runOutput;
}

sourceEl.addEventListener('input', () => {
  syncFileFromEditor();
});

runBtn.addEventListener('click', () => {
  setOutputs({ output: '', console: '', terminal: '' });
  try {
    doRun();
    setOutputMode(consoleOutputEl.textContent ? 'console' : 'output');
  } catch (err) {
    setOutputs({
      output: '',
      console: err.message,
      terminal: `$ mp run ${project.activeFile}\n${err.message}`
    });
    setOutputMode('console');
  }
});

outputTabBtn.addEventListener('click', () => setOutputMode('output'));
consoleTabBtn.addEventListener('click', () => setOutputMode('console'));
terminalTabBtn.addEventListener('click', () => setOutputMode('terminal'));
terminalInputEl.addEventListener('keydown', (event) => {
  if (event.key === 'ArrowUp') {
    if (!terminalCommandHistory.length) return;
    event.preventDefault();
    terminalHistoryIndex = terminalHistoryIndex < 0
      ? terminalCommandHistory.length - 1
      : Math.max(0, terminalHistoryIndex - 1);
    terminalInputEl.value = terminalCommandHistory[terminalHistoryIndex] || '';
    return;
  }
  if (event.key === 'ArrowDown') {
    if (!terminalCommandHistory.length) return;
    event.preventDefault();
    terminalHistoryIndex = Math.min(terminalCommandHistory.length, terminalHistoryIndex + 1);
    terminalInputEl.value = terminalCommandHistory[terminalHistoryIndex] || '';
    return;
  }
  if (event.key !== 'Enter') return;
  event.preventDefault();
  const command = terminalInputEl.value.trim();
  if (!command) return;
  terminalCommandHistory.push(command);
  terminalHistoryIndex = terminalCommandHistory.length;
  terminalInputEl.value = '';
  runTerminalCommand(command);
});

document.getElementById('newFolderBtn').addEventListener('click', () => {
  const raw = prompt('New folder path (example: src/utils)');
  if (!raw) return;
  const folder = normalizeFolderPath(raw);
  if (!folder) return;
  if (project.folders.some((f) => f.path === folder)) {
    alert('Folder already exists.');
    return;
  }
  ensureParentFolders(project, `${folder}/placeholder.mp`);
  if (!project.folders.some((f) => f.path === folder)) {
    project.folders.push({ path: folder, createdAt: new Date().toISOString() });
  }
  project.folders.sort((a, b) => a.path.localeCompare(b.path));
  saveProject(project);
  renderFileList();
});

deleteFolderBtn.addEventListener('click', () => {
  const folderToDelete = selectedFolder || normalizeFolderPath(prompt('Folder path to delete'));
  if (!folderToDelete) return;
  const exists = project.folders.some((f) => f.path === folderToDelete);
  if (!exists) {
    alert(`Folder not found: ${folderToDelete}`);
    return;
  }
  if (!confirm(`Delete folder "${folderToDelete}" and all nested files/subfolders?`)) return;

  project.folders = project.folders.filter(
    (f) => !(f.path === folderToDelete || f.path.startsWith(`${folderToDelete}/`))
  );
  Object.keys(project.files).forEach((filePath) => {
    if (filePath === folderToDelete || filePath.startsWith(`${folderToDelete}/`)) {
      delete project.files[filePath];
    }
  });

  selectedFolder = '';
  const remainingFiles = Object.keys(project.files);
  if (!remainingFiles.length) {
    project.files['main.mp'] = '# New file';
    project.activeFile = 'main.mp';
  } else if (!project.files[project.activeFile]) {
    project.activeFile = remainingFiles.sort((a, b) => a.localeCompare(b))[0];
  }

  syncEditorFromFile();
  saveProject(project);
  renderFileList();
  setOutputs(
    {
      output: `Deleted folder ${folderToDelete}`,
      console: '',
      terminal: `${terminalPromptEl.textContent} rm -r ${folderToDelete}\nDeleted folder ${folderToDelete}`
    }
  );
});

document.getElementById('newFileBtn').addEventListener('click', () => {
  const name = prompt('New file path (example: src/utils/math.mp)');
  if (!name) return;
  const safeName = normalizeFilePath(name);
  if (!safeName) return;
  if (project.files[safeName]) {
    alert('File already exists.');
    return;
  }
  ensureParentFolders(project, safeName);
  project.files[safeName] = '# New file';
  project.activeFile = safeName;
  saveProject(project);
  syncEditorFromFile();
  renderFileList();
});

document.getElementById('renameFileBtn').addEventListener('click', () => {
  const oldName = project.activeFile;
  const nextName = prompt('Rename file path', oldName);
  if (!nextName || normalizeFilePath(nextName) === oldName) return;
  const safeName = normalizeFilePath(nextName);
  if (!safeName) return;
  if (project.files[safeName]) {
    alert('A file with that name already exists.');
    return;
  }
  ensureParentFolders(project, safeName);
  project.files[safeName] = project.files[oldName];
  delete project.files[oldName];
  project.activeFile = safeName;
  saveProject(project);
  renderFileList();
});

document.getElementById('deleteFileBtn').addEventListener('click', () => {
  const allFiles = Object.keys(project.files);
  if (allFiles.length <= 1) {
    alert('You need at least one file in the project.');
    return;
  }
  if (!confirm(`Delete ${project.activeFile}?`)) return;
  delete project.files[project.activeFile];
  project.activeFile = Object.keys(project.files).sort()[0];
  saveProject(project);
  syncEditorFromFile();
  renderFileList();
});

document.getElementById('newProjectBtn').addEventListener('click', () => {
  if (!confirm('Create a new project? This replaces the current local project.')) return;
  project = createDefaultProject();
  selectedFolder = '';
  saveProject(project);
  syncEditorFromFile();
  renderFileList();
  setOutputs({ output: '', console: '', terminal: '' });
});

document.getElementById('exportBtn').addEventListener('click', () => {
  syncFileFromEditor();
  const blob = new Blob([JSON.stringify(project, null, 2)], { type: 'application/json' });
  const a = document.createElement('a');
  const fileStem = (project.projectName || 'magphos-project').replace(/\s+/g, '-');
  a.href = URL.createObjectURL(blob);
  a.download = `${fileStem}.json`;
  document.body.appendChild(a);
  a.click();
  a.remove();
});

document.getElementById('importInput').addEventListener('change', async (event) => {
  const file = event.target.files?.[0];
  if (!file) return;
  try {
    const text = await file.text();
    const parsed = ensureProjectShape(JSON.parse(text));
    if (!parsed.files || !Object.keys(parsed.files).length) {
      throw new Error('Invalid project file.');
    }
    Object.keys(parsed.files).forEach((filePath) => ensureParentFolders(parsed, filePath));
    project = parsed;
    selectedFolder = '';
    ensureActiveFile();
    saveProject(project);
    syncEditorFromFile();
    renderFileList();
    setOutputs({
      output: `Imported ${file.name}`,
      console: '',
      terminal: `$ mp import ${file.name}\nImported.`
    });
  } catch (err) {
    setOutputs({
      output: '',
      console: err.message,
      terminal: err.message
    });
    setOutputMode('console');
  } finally {
    event.target.value = '';
  }
});

async function init() {
  syncEditorFromFile();
  renderFileList();
  terminalCwd = '';
  refreshTerminalPrompt();
  pushTerminalLine('MagPhos Terminal — type "help" for commands.');

  await loadWasmCompiler();

  if (!wasmCompiler) {
    runBtn.disabled = true;
    const lines = [
      'C++ WASM compiler failed to load. Run is disabled until compiler artifacts are published.',
      'Required artifacts: web/magphos_wasm_singlefile.js OR web/magphos_wasm.js + web/magphos_wasm.wasm.64 (or .wasm).',
      'Build with Emscripten: ./tools/scripts/build_web.sh.',
      wasmLoadError ? `Loader error: ${wasmLoadError}` : ''
    ];
    if (wasmLoadError && wasmLoadError.includes('404')) {
      lines.push('Detected 404 loading WASM JS loader. Publish/commit web/magphos_wasm.js and web/magphos_wasm_singlefile.js.');
    }
    setOutputs({
      output: '',
      console: lines.filter(Boolean).join('\n'),
      terminal: ''
    });
    setOutputMode('console');
  } else {
    runBtn.disabled = false;
    setOutputs({
      output: 'C++ WASM compiler connected. Press Run to execute MagPhos code.',
      console: '',
      terminal: ''
    });
    setOutputMode('output');
  }
}

setOutputMode('output');
init();
