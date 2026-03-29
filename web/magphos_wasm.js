(function (global) {
  function toJs(source) {
    const lines = String(source || '').replace(/\r/g, '').split('\n');
    const out = [];
    out.push('"use strict";');
    out.push('const __mp_out = [];');
    out.push('const __mp_print = (v) => __mp_out.push(String(v));');

    for (let raw of lines) {
      let line = raw.replace(/#.*$/, '').trim();
      if (!line) continue;

      line = line.replace(/^fn\s+/, 'function ');
      line = line.replace(/^var\s+/, 'let ');
      line = line.replace(/^set\s+/, '');
      line = line.replace(/^repeat\s+while\s*\((.*)\)\s*\{$/, 'while ($1) {');
      line = line.replace(/^loop\s+(.+)\s*\{$/, 'for (let __mp_i = 0; __mp_i < ($1); __mp_i += 1) {');

      if (/^print\s+/.test(line)) {
        const expr = line.replace(/^print\s+/, '').replace(/;$/, '');
        out.push(`__mp_print(${expr});`);
        continue;
      }

      if (!/[;{}]$/.test(line)) line += ';';
      out.push(line);
    }

    out.push('return __mp_out.join("\\n");');
    return out.join('\n');
  }

  function compileMagPhos(source) {
    const transpiled = toJs(source);
    return `const __mp_result = (function(){\n${transpiled}\n})();\nif (__mp_result) console.log(__mp_result);`;
  }

  function analyzeMagPhos(source) {
    return JSON.stringify({ ok: true, mode: 'js-fallback', lines: String(source || '').split(/\n/).length });
  }

  function buildModule() {
    return { compileMagPhos, analyzeMagPhos };
  }

  global.MagPhosWasmFactory = async function MagPhosWasmFactory() {
    return buildModule();
  };

  if (global.Module && typeof global.Module === 'object') {
    global.Module.compileMagPhos = compileMagPhos;
    global.Module.analyzeMagPhos = analyzeMagPhos;
    if (typeof global.Module.onRuntimeInitialized === 'function') {
      queueMicrotask(() => global.Module.onRuntimeInitialized());
    }
  }
})(typeof window !== 'undefined' ? window : globalThis);
