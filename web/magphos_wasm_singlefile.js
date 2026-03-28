(function attachMagPhosStub(globalObj) {
  const root = globalObj || (typeof window !== 'undefined' ? window : globalThis);
  const moduleRef = root.Module && typeof root.Module === 'object' ? root.Module : {};

  function compileMagPhosFallback(source) {
    const lines = String(source).split('\n');
    const out = [
      '// Stub single-file mode (generated WASM artifact missing).',
      '// Run ./tools/scripts/build_web.sh for full C++ WASM compiler output.'
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

  moduleRef.compileMagPhos = compileMagPhosFallback;
  root.Module = moduleRef;

  if (typeof moduleRef.onRuntimeInitialized === 'function') {
    try {
      moduleRef.onRuntimeInitialized();
    } catch (_) {
      // ignore callback errors in stub mode
    }
  }
})(typeof window !== 'undefined' ? window : globalThis);
