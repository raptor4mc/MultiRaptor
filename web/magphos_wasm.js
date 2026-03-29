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

function analyzeMagPhos() {
  return 'ok';
}

export default async function MagPhosWasmFactory() {
  return {
    compileMagPhos: transpileMagPhos,
    analyzeMagPhos
  };
}

export { MagPhosWasmFactory };
