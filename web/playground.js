const STORAGE_KEY = 'magphos-web-studio-v2';
function trim(v) { return v.trim(); }

function compileMagPhos(source) {
  const lines = source.split(/\r?\n/);
  let indent = 0;
  let out = "'use strict';\n";
  let inBlockComment = false;
  const pad = (n) => ' '.repeat(n * 2);

  for (let i = 0; i < lines.length; i += 1) {
    const text = trim(lines[i]);

    if (inBlockComment) {
      if (text.includes('!/')) inBlockComment = false;
      continue;
    }
    if (text.startsWith('//!')) {
      inBlockComment = true;
      continue;
    }
    if (!text || text.startsWith('//') || text.startsWith('#') || text.startsWith('/!')) continue;

    if (text === '}') {
      indent = Math.max(0, indent - 1);
      out += `${pad(indent)}}\n`;
      continue;
    }

    if (text === 'else {') {
      indent = Math.max(0, indent - 1);
      out += `${pad(indent)}else {\n`;
      indent += 1;
      continue;
    }

    let m;
    if ((m = text.match(/^var\s+([A-Za-z_][A-Za-z0-9_]*)\s*=\s*(.+)$/))) {
      out += `${pad(indent)}let ${m[1]} = ${m[2]};\n`;
    } else if ((m = text.match(/^const\s+([A-Za-z_][A-Za-z0-9_]*)\s*=\s*(.+)$/))) {
      out += `${pad(indent)}const ${m[1]} = ${m[2]};\n`;
    } else if ((m = text.match(/^set\s+([A-Za-z_][A-Za-z0-9_]*)\s*=\s*(.+)$/))) {
      out += `${pad(indent)}${m[1]} = ${m[2]};\n`;
    } else if ((m = text.match(/^print\s+(.+)$/))) {
      out += `${pad(indent)}console.log(${m[1]});\n`;
    } else if ((m = text.match(/^ask\s+(.+)\s*->\s*([A-Za-z_][A-Za-z0-9_]*)$/))) {
      out += `${pad(indent)}let ${m[2]} = prompt(${m[1]}) ?? "";\n`;
    } else if ((m = text.match(/^if\s+(.+)\s*\{$/)) || (m = text.match(/^when\s+(.+)\s*\{$/))) {
      out += `${pad(indent)}if (${m[1]}) {\n`;
      indent += 1;
    } else if ((m = text.match(/^loop\s+(.+)\s*\{$/))) {
      out += `${pad(indent)}for (let __mp_i = 0; __mp_i < (${m[1]}); __mp_i += 1) {\n`;
      indent += 1;
    } else if ((m = text.match(/^repeat\s+while\s+(.+)\s*\{$/))) {
      out += `${pad(indent)}while (${m[1]}) {\n`;
      indent += 1;
    } else if ((m = text.match(/^fn\s+([A-Za-z_][A-Za-z0-9_]*)\s*\(([^)]*)\)\s*\{$/))) {
      out += `${pad(indent)}function ${m[1]}(${m[2]}) {\n`;
      indent += 1;
    } else if ((m = text.match(/^return\s+(.+)$/))) {
      out += `${pad(indent)}return ${m[1]};\n`;
    } else if (text === 'stop') {
      out += `${pad(indent)}break;\n`;
    } else if (text === 'next') {
      out += `${pad(indent)}continue;\n`;
    } else {
      throw new Error(`Syntax error at line ${i + 1}: ${text}`);
    }
  }

  if (indent !== 0) throw new Error('Syntax error: mismatched braces');
  return out;
}

function createDefaultProject() {
  const template = document.getElementById('defaultProgram').content.textContent;
  return {
    projectName: 'chromebook-studio',
    activeFile: 'main.mp',
    folders: ['src', 'assets'],
    files: {
      'main.mp': template,
      'src/notes.mp': '# Put extra snippets here'
    }
  };
}

function loadProject() {
  try {
    const raw = localStorage.getItem(STORAGE_KEY);
    if (!raw) return createDefaultProject();
    const parsed = JSON.parse(raw);
    if (!parsed || !parsed.files || !Object.keys(parsed.files).length) return createDefaultProject();
    parsed.folders = Array.isArray(parsed.folders) ? parsed.folders : [];
    return parsed;
  } catch (_) {
    return createDefaultProject();
  }
}

function saveProject(project) {
  localStorage.setItem(STORAGE_KEY, JSON.stringify(project));
}

const sourceEl = document.getElementById('source');
const fileTreeEl = document.getElementById('fileTree');
const activeFileLabel = document.getElementById('activeFileLabel');
const runtimeFrame = document.getElementById('runtimeFrame');
const stagePanel = document.getElementById('stagePanel');

const problemsPanel = document.getElementById('panel-problems');
const outputPanel = document.getElementById('panel-output');
const terminalPanel = document.getElementById('panel-terminal');
const debugPanel = document.getElementById('panel-debug');

let project = loadProject();

function ensureActiveFile() {
  if (!project.files[project.activeFile]) {
    project.activeFile = Object.keys(project.files).sort()[0];
  }
}

function listTreeItems() {
  const items = [];
  const folders = [...new Set(project.folders)].sort();
  folders.forEach((folder) => items.push({ type: 'folder', path: folder }));
  Object.keys(project.files).sort().forEach((filePath) => items.push({ type: 'file', path: filePath }));
  return items;
}

function indentFor(path) {
  return `${'\u00A0\u00A0'.repeat(path.split('/').length - 1)}`;
}

function renderExplorer() {
  ensureActiveFile();
  fileTreeEl.innerHTML = '';
  for (const item of listTreeItems()) {
    const li = document.createElement('li');
    li.className = 'tree-item';

    const btn = document.createElement('button');
    btn.className = `tree-btn ${item.type}`;
    btn.innerHTML = `${indentFor(item.path)}${item.type === 'folder' ? '📁' : '📄'} ${item.path.split('/').pop()}`;

    if (item.type === 'file' && item.path === project.activeFile) {
      btn.classList.add('active');
    }

    btn.addEventListener('click', () => {
      if (item.type === 'folder') {
        terminal(`Folder selected: ${item.path}`);
        return;
      }
      project.activeFile = item.path;
      syncEditorFromFile();
      renderExplorer();
      saveProject(project);
    });

    li.appendChild(btn);
    fileTreeEl.appendChild(li);
  }

  activeFileLabel.textContent = `Editor — ${project.activeFile}`;
}

function syncEditorFromFile() {
  sourceEl.value = project.files[project.activeFile] || '';
}

function syncFileFromEditor() {
  project.files[project.activeFile] = sourceEl.value;
  saveProject(project);
}

function normalizePath(path) {
  return path.trim().replace(/\\+/g, '/').replace(/^\/+|\/+$/g, '').replace(/\/+/g, '/');
}

function terminal(message) {
  const line = `[${new Date().toLocaleTimeString()}] ${message}`;
  terminalPanel.textContent = `${line}\n${terminalPanel.textContent}`.trim();
}

function debug(message) {
  debugPanel.textContent = `${message}\n${debugPanel.textContent}`.trim();
}

function setProblems(message) {
  problemsPanel.textContent = message || 'No problems.';
}

function clearOutput() {
  outputPanel.textContent = '(no output yet)';
}

function doCompile() {
  syncFileFromEditor();
  const source = sourceEl.value;
  try {
    const js = compileMagPhos(source);
    setProblems('No problems.');
    terminal(`Compiled ${project.activeFile}`);
    return js;
  } catch (err) {
    setProblems(err.message);
    throw err;
  }
}

function renderInRuntime(jsCode) {
  const runtimeHtml = `<!doctype html><html><body style="margin:0;background:#111;color:#eee;font-family:system-ui"></body><script>
  const send=(type,payload)=>parent.postMessage({type,payload},'*');
  ['log','warn','error','info'].forEach((kind)=>{
    const original=console[kind];
    console[kind]=(...args)=>{ send('console',{kind,args:args.map(String)}); original.apply(console,args); };
  });
  window.addEventListener('error',(e)=>send('runtime_error',String(e.message||e.error||e)));
  try { ${jsCode}\n send('run_complete','ok'); } catch (e) { send('runtime_error',String(e && e.message ? e.message : e)); }
  <\/script></html>`;
  runtimeFrame.srcdoc = runtimeHtml;
}

window.addEventListener('message', (event) => {
  const data = event.data || {};
  if (data.type === 'console') {
    const prefix = data.payload.kind.toUpperCase();
    const text = `[${prefix}] ${data.payload.args.join(' ')}`;
    outputPanel.textContent = `${outputPanel.textContent === '(no output yet)' ? '' : outputPanel.textContent + '\n'}${text}`.trim();
    debug(`console.${data.payload.kind}: ${data.payload.args.join(' | ')}`);
  } else if (data.type === 'runtime_error') {
    setProblems(`Runtime error: ${data.payload}`);
    debug(`runtime_error: ${data.payload}`);
  } else if (data.type === 'run_complete') {
    terminal(`Run complete: ${project.activeFile}`);
  }
});

sourceEl.addEventListener('input', () => {
  syncFileFromEditor();
});

document.getElementById('newFolderBtn').addEventListener('click', () => {
  const path = prompt('New folder path (example: src/game)');
  if (!path) return;
  const folder = normalizePath(path);
  if (!folder) return;
  if (project.folders.includes(folder)) {
    alert('Folder already exists.');
    return;
  }
  project.folders.push(folder);
  saveProject(project);
  renderExplorer();
  terminal(`Created folder: ${folder}`);
});

document.getElementById('newFileBtn').addEventListener('click', () => {
  const path = prompt('New file path (example: src/player.mp)');
  if (!path) return;
  const file = normalizePath(path);
  if (!file) return;
  if (project.files[file]) {
    alert('File already exists.');
    return;
  }
  const parent = file.includes('/') ? file.split('/').slice(0, -1).join('/') : '';
  if (parent && !project.folders.includes(parent)) project.folders.push(parent);
  project.files[file] = '# New file';
  project.activeFile = file;
  saveProject(project);
  syncEditorFromFile();
  renderExplorer();
  terminal(`Created file: ${file}`);
});

document.getElementById('renameFileBtn').addEventListener('click', () => {
  const oldPath = project.activeFile;
  const nextPath = prompt('Rename file path', oldPath);
  if (!nextPath) return;
  const file = normalizePath(nextPath);
  if (!file || file === oldPath) return;
  if (project.files[file]) {
    alert('A file with that path already exists.');
    return;
  }
  project.files[file] = project.files[oldPath];
  delete project.files[oldPath];
  project.activeFile = file;
  const parent = file.includes('/') ? file.split('/').slice(0, -1).join('/') : '';
  if (parent && !project.folders.includes(parent)) project.folders.push(parent);
  saveProject(project);
  renderExplorer();
  terminal(`Renamed file: ${oldPath} -> ${file}`);
});

document.getElementById('deleteFileBtn').addEventListener('click', () => {
  if (Object.keys(project.files).length <= 1) {
    alert('At least one file is required.');
    return;
  }
  const file = project.activeFile;
  if (!confirm(`Delete ${file}?`)) return;
  delete project.files[file];
  project.activeFile = Object.keys(project.files).sort()[0];
  saveProject(project);
  syncEditorFromFile();
  renderExplorer();
  terminal(`Deleted file: ${file}`);
});

document.getElementById('newProjectBtn').addEventListener('click', () => {
  if (!confirm('Create a new project? Current local project will be replaced.')) return;
  project = createDefaultProject();
  saveProject(project);
  syncEditorFromFile();
  renderExplorer();
  setProblems('No problems.');
  clearOutput();
  terminal('Created new project.');
});

document.getElementById('exportBtn').addEventListener('click', () => {
  syncFileFromEditor();
  const blob = new Blob([JSON.stringify(project, null, 2)], { type: 'application/json' });
  const a = document.createElement('a');
  const stem = (project.projectName || 'magphos-project').replace(/\s+/g, '-');
  a.href = URL.createObjectURL(blob);
  a.download = `${stem}.json`;
  document.body.appendChild(a);
  a.click();
  a.remove();
  terminal(`Exported project: ${a.download}`);
});

document.getElementById('importInput').addEventListener('change', async (event) => {
  const file = event.target.files?.[0];
  if (!file) return;
  try {
    const text = await file.text();
    const parsed = JSON.parse(text);
    if (!parsed.files || !Object.keys(parsed.files).length) throw new Error('Invalid project file.');
    parsed.folders = Array.isArray(parsed.folders) ? parsed.folders : [];
    project = parsed;
    ensureActiveFile();
    saveProject(project);
    syncEditorFromFile();
    renderExplorer();
    terminal(`Imported project: ${file.name}`);
  } catch (err) {
    setProblems(err.message);
  } finally {
    event.target.value = '';
  }
});

document.getElementById('compileBtn').addEventListener('click', () => {
  try {
    doCompile();
  } catch (_) {
    terminal('Compile failed.');
  }
});

document.getElementById('runBtn').addEventListener('click', () => {
  clearOutput();
  try {
    const js = doCompile();
    renderInRuntime(js);
    terminal(`Running: ${project.activeFile}`);
  } catch (_) {
    terminal('Run canceled because compile failed.');
  }
});

document.getElementById('fullscreenBtn').addEventListener('click', async () => {
  if (document.fullscreenElement) {
    await document.exitFullscreen();
    terminal('Exited fullscreen runtime.');
    return;
  }
  await stagePanel.requestFullscreen();
  terminal('Entered fullscreen runtime.');
});

document.querySelectorAll('.tab').forEach((tab) => {
  tab.addEventListener('click', () => {
    document.querySelectorAll('.tab').forEach((t) => t.classList.remove('active'));
    document.querySelectorAll('.dock-panel').forEach((panel) => panel.classList.remove('active'));
    tab.classList.add('active');
    document.getElementById(`panel-${tab.dataset.tab}`).classList.add('active');
  });
});

syncEditorFromFile();
renderExplorer();
setProblems('No problems.');
clearOutput();
terminal('Web Studio ready.');
