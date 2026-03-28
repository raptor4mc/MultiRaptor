const STORAGE_KEY = 'magphos-web-studio-v2';

let wasmCompiler = null;
let wasmLoadError = null;

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
  const files = safe.files && typeof safe.files === 'object' ? safe.files : {};
  const folders = Array.isArray(safe.folders) ? safe.folders : [];

  return {
    projectName: safe.projectName || 'chromebook-studio',
    activeFile: safe.activeFile || Object.keys(files)[0] || 'main.mp',
    files,
    folders: [...new Set(folders.map(normalizeFolderPath).filter(Boolean))]
  };
}

async function loadWasmCompiler() {
  const attempts = [];
  const loaderUrls = [
    new URL('./magphos_wasm.js', import.meta.url).href,
    new URL('../magphos_wasm.js', import.meta.url).href
  ];
  const wasmUrls = [
    new URL('./magphos_wasm.wasm', import.meta.url).href,
    new URL('../magphos_wasm.wasm', import.meta.url).href
  ];

  for (const loaderUrl of loaderUrls) {
    for (const wasmUrl of wasmUrls) {
      try {
        const moduleFactory = (await import(loaderUrl)).default;
        const wasmModule = await moduleFactory({
          locateFile(path) {
            if (path.endsWith('.wasm')) return wasmUrl;
            return new URL(path, loaderUrl).href;
          }
        });
        if (typeof wasmModule.compileMagPhos !== 'function') {
          throw new Error('WASM module loaded, but compileMagPhos export is missing.');
        }
        wasmCompiler = wasmModule.compileMagPhos;
        wasmLoadError = null;
        return;
      } catch (err) {
        attempts.push(`${loaderUrl} + ${wasmUrl}: ${err.message}`);
      }
    }
  }

  wasmLoadError = attempts.join(' | ');
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
    folders: ['src', 'notes'],
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
  for (let i = 1; i < parts.length; i += 1) {
    const folder = parts.slice(0, i).join('/');
    if (!project.folders.includes(folder)) {
      project.folders.push(folder);
      created.push(folder);
    }
  }
  if (created.length) {
    project.folders.sort((a, b) => a.localeCompare(b));
  }
}

const sourceEl = document.getElementById('source');
const compiledEl = document.getElementById('compiled');
const outputEl = document.getElementById('output');
const fileListEl = document.getElementById('fileList');
const activeFileLabel = document.getElementById('activeFileLabel');

let project = loadProject();

function ensureActiveFile() {
  if (!project.files[project.activeFile]) {
    project.activeFile = Object.keys(project.files).sort()[0];
  }
}

function renderFileList() {
  ensureActiveFile();
  fileListEl.innerHTML = '';

  const folderNames = [...project.folders].sort((a, b) => a.localeCompare(b));
  const fileNames = Object.keys(project.files).sort((a, b) => a.localeCompare(b));

  folderNames.forEach((folderName) => {
    const li = document.createElement('li');
    li.className = 'folder-item';
    li.textContent = `📁 ${folderName}`;
    fileListEl.appendChild(li);
  });

  fileNames.forEach((fileName) => {
    const li = document.createElement('li');
    const btn = document.createElement('button');
    btn.className = 'file-btn';
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
  const js = compileMagPhos(sourceEl.value);
  compiledEl.textContent = js;
  return js;
}

sourceEl.addEventListener('input', () => {
  syncFileFromEditor();
});

document.getElementById('compileBtn').addEventListener('click', () => {
  outputEl.textContent = '';
  try {
    doCompile();
  } catch (err) {
    outputEl.textContent = err.message;
  }
});

document.getElementById('runBtn').addEventListener('click', () => {
  outputEl.textContent = '';
  try {
    const js = doCompile();
    const logs = [];
    const originalLog = console.log;
    console.log = (...args) => logs.push(args.join(' '));
    try {
      new Function(js)();
    } finally {
      console.log = originalLog;
    }
    outputEl.textContent = logs.join('\n') || '(no output)';
  } catch (err) {
    outputEl.textContent = err.message;
  }
});

document.getElementById('newFolderBtn').addEventListener('click', () => {
  const raw = prompt('New folder path (example: src/utils)');
  if (!raw) return;
  const folder = normalizeFolderPath(raw);
  if (!folder) return;
  if (project.folders.includes(folder)) {
    alert('Folder already exists.');
    return;
  }
  ensureParentFolders(project, `${folder}/placeholder.mp`);
  if (!project.folders.includes(folder)) project.folders.push(folder);
  project.folders.sort((a, b) => a.localeCompare(b));
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
  compiledEl.textContent = '';
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
    outputEl.textContent = [
      'WASM compiler not found.',
      'Expected: web/magphos_wasm.js and web/magphos_wasm.wasm',
      'Build with Emscripten: cmake -S . -B build-web -DMAGPHOS_BUILD_WASM=ON && cmake --build build-web',
      wasmLoadError ? `Loader error: ${wasmLoadError}` : ''
    ].filter(Boolean).join('\n');
    return;
  }

  document.getElementById('compileBtn').click();
}

init();
