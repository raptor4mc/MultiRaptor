function fallbackCompileMagPhos(source) {
  const lines = String(source).split('\n');
  const out = [
    '// Stub loader mode (generated WASM artifact missing).',
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

export async function MagPhosWasmFactory() {
  return { compileMagPhos: fallbackCompileMagPhos };
}

export default MagPhosWasmFactory;
