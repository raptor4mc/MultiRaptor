(function initMagPhosSinglefile(globalScope) {
  function transpileMagPhos(source) {
    const lines = String(source ?? '').split('\n');
    const out = [
      '// Browser JS compiler shim (MagPhos syntax -> JS runtime).'
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

  const moduleRef = (globalScope.Module && typeof globalScope.Module === 'object')
    ? globalScope.Module
    : {};

  moduleRef.compileMagPhos = transpileMagPhos;
  moduleRef.analyzeMagPhos = () => 'ok';
  globalScope.Module = moduleRef;

  if (typeof moduleRef.onRuntimeInitialized === 'function') {
    moduleRef.onRuntimeInitialized();
  }
})(window);
