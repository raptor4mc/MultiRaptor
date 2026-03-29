(function (global) {
  function compileMagPhos(source) {
    return global.__magphos_compile_fallback
      ? global.__magphos_compile_fallback(source)
      : `console.log(${JSON.stringify('MagPhos fallback loader active, but compiler bootstrap is incomplete.')});`;
  }

  function analyzeMagPhos(source) {
    return JSON.stringify({ ok: true, mode: 'singlefile-js-fallback', bytes: String(source || '').length });
  }

  if (!global.__magphos_compile_fallback) {
    global.__magphos_compile_fallback = function (source) {
      const src = String(source || '');
      const lines = src.split(/\r?\n/);
      const js = [];
      js.push('const __out=[]; const __p=(v)=>__out.push(String(v));');
      for (const raw of lines) {
        let line = raw.replace(/#.*$/, '').trim();
        if (!line) continue;
        line = line.replace(/^fn\s+/, 'function ').replace(/^var\s+/, 'let ').replace(/^set\s+/, '');
        line = line.replace(/^repeat\s+while\s*\((.*)\)\s*\{$/, 'while ($1) {');
        line = line.replace(/^loop\s+(.+)\s*\{$/, 'for (let __i=0; __i<($1); __i++){');
        if (/^print\s+/.test(line)) {
          js.push(`__p(${line.replace(/^print\s+/, '')});`);
        } else {
          if (!/[;{}]$/.test(line)) line += ';';
          js.push(line);
        }
      }
      js.push('const __r=__out.join("\\n"); if(__r) console.log(__r);');
      return js.join('\n');
    };
  }

  if (global.Module && typeof global.Module === 'object') {
    global.Module.compileMagPhos = compileMagPhos;
    global.Module.analyzeMagPhos = analyzeMagPhos;
    if (typeof global.Module.onRuntimeInitialized === 'function') {
      queueMicrotask(() => global.Module.onRuntimeInitialized());
    }
  }
})(typeof window !== 'undefined' ? window : globalThis);
