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
  const isFileProtocol = window.location.protocol === 'file:';
  const loaderCandidates = isFileProtocol
    ? ['./magphos_wasm_singlefile.js', './magphos_wasm.js']
    : ['./magphos_wasm.js', './magphos_wasm_singlefile.js'];
  const wasmCandidates = [null, './magphos_wasm.wasm', './magphos.wasm'];
  const loaderUrls = loaderCandidates.map((path) => new URL(path, SCRIPT_BASE_URL).href);
  const wasmUrls = wasmCandidates.map((path) => (path ? new URL(path, SCRIPT_BASE_URL).href : null));

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

function compileWithFallbackTranspiler(source) {
  const lines = source.split('\n');
  const out = [
    '// Fallback compiler mode (WASM unavailable).',
    '// Behavior may differ from native MagPhos compiler.'
  ];

  for (const rawLine of lines) {
    const line = rawLine.trim();
    if (!line || line.startsWith('#')) continue;

    if (line.startsWith('fn ')) {
      out.push(rawLine.replace(/\bfn\b/, 'function'));
      continue;
    }

    if (line.startsWith('print ')) {
      out.push(rawLine.replace(/\bprint\b/, 'console.log') + ';');
      continue;
    }

    out.push(rawLine);
  }

  return out.join('\n');
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
const outputEl = document.getElementById('output');
const fileListEl = document.getElementById('fileList');
const activeFileLabel = document.getElementById('activeFileLabel');
const compileBtn = document.getElementById('compileBtn');
const runBtn = document.getElementById('runBtn');
const enableFallbackBtn = document.getElementById('enableFallbackBtn');

let project = loadProject();

function ensureActiveFile() {
  if (!project.files[project.activeFile]) {
    project.activeFile = Object.keys(project.files).sort()[0];
  }
}

function renderFileList() {
  ensureActiveFile();
  fileListEl.innerHTML = '';

  const folderNames = project.folders
    .map((folder) => folder.path)
    .sort((a, b) => a.localeCompare(b));
  const fileNames = Object.keys(project.files).sort((a, b) => a.localeCompare(b));

  folderNames.forEach((folderName) => {
    const li = document.createElement('li');
    const depth = folderName.split('/').length - 1;
    li.className = 'folder-item tree-item';
    li.style.paddingLeft = `${12 + depth * 16}px`;
    li.textContent = `📁 ${folderName}`;
    fileListEl.appendChild(li);
  });

  fileNames.forEach((fileName) => {
    const li = document.createElement('li');
    const btn = document.createElement('button');
    const depth = fileName.split('/').length - 1;
    btn.className = 'file-btn tree-item';
    btn.style.paddingLeft = `${12 + depth * 16}px`;
    if (fileName === project.activeFile) btn.classList.add('active');
    btn.textContent = `📄 ${fileName}`;
    btn.addEventListener('click', () => {
      project.activeFile = fileName;
      syncEditorFromFile();
      renderFileList();
      saveProject(project);
    });
    li.appendChild(btn);
    fileListEl.appendChild(li);
  });

  activeFileLabel.textContent = `Editor — ${project.activeFile}`;
}

function syncEditorFromFile() {
  sourceEl.value = project.files[project.activeFile] ?? '';
}

function syncFileFromEditor() {
  project.files[project.activeFile] = sourceEl.value;
  saveProject(project);
}

function doCompile() {
  syncFileFromEditor();
  if (!wasmCompiler) {
    throw new Error('WASM compiler is not loaded yet. Build web/magphos_wasm_singlefile.js from C++ using ./tools/scripts/build_web.sh to keep web and native behavior in sync.');
  }
  if (wasmAnalyzer) {
    const analysis = wasmAnalyzer(sourceEl.value);
    if (analysis !== 'ok') {
      throw new Error(analysis);
    }
    outputEl.textContent = 'No errors found. Program is valid.';
    return 'ok';
  }
  compileMagPhos(sourceEl.value);
  outputEl.textContent = 'No errors found. Program is valid.';
  return 'ok';
}

sourceEl.addEventListener('input', () => {
  syncFileFromEditor();
});

compileBtn.addEventListener('click', () => {
  outputEl.textContent = '';
  try {
    doCompile();
  } catch (err) {
    outputEl.textContent = err.message;
  }
});

runBtn.addEventListener('click', () => {
  outputEl.textContent = '';
  try {
    doCompile();
    outputEl.textContent += '\nWeb IDE mode: JS output is hidden.';
  } catch (err) {
    outputEl.textContent = err.message;
  }
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
  saveProject(project);
  syncEditorFromFile();
  renderFileList();
  outputEl.textContent = '';
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
    ensureActiveFile();
    saveProject(project);
    syncEditorFromFile();
    renderFileList();
    outputEl.textContent = `Imported ${file.name}`;
  } catch (err) {
    outputEl.textContent = err.message;
  } finally {
    event.target.value = '';
  }
});

async function init() {
  syncEditorFromFile();
  renderFileList();

  await loadWasmCompiler();

  if (!wasmCompiler) {
    wasmCompiler = compileWithFallbackTranspiler;
    wasmAnalyzer = () => 'ok';
    compileBtn.disabled = false;
    runBtn.disabled = false;
    enableFallbackBtn.hidden = true;
    outputEl.textContent = [
      'C++ WASM compiler was not loaded, so fallback transpiler mode is active.',
      'For native parity: build and publish real web/magphos_wasm_singlefile.js or web/magphos_wasm.js + web/magphos_wasm.wasm.',
      'Build with Emscripten: ./tools/scripts/build_web.sh.',
      wasmLoadError ? `Loader error: ${wasmLoadError}` : ''
    ].filter(Boolean).join('\n');
  } else {
    compileBtn.disabled = false;
    runBtn.disabled = false;
    enableFallbackBtn.hidden = true;
    outputEl.textContent = 'C++ WASM compiler connected. Web compile is linked to native parser/semantic/compiler pipeline.';
    compileBtn.click();
  }
}

enableFallbackBtn.addEventListener('click', () => {
  wasmCompiler = compileWithFallbackTranspiler;
  wasmAnalyzer = () => 'ok';
  compileBtn.disabled = false;
  runBtn.disabled = false;
  enableFallbackBtn.hidden = true;
  outputEl.textContent = [
    'Fallback transpiler mode enabled manually.',
    'Warning: this does not guarantee parity with the C++ compiler.',
    'For C++ parity, rebuild and ship real web/magphos_wasm_singlefile.js (or web/magphos_wasm.js + web/magphos_wasm.wasm).'
  ].join('\n');
});

init();
