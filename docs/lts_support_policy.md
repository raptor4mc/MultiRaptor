# LTS release and support policy

## Release channels

- **Current**: latest feature release line (`X.Y.x`)
- **LTS**: designated stable line for enterprise production
- **EOL**: unsupported lines

## Support windows

- Current line: active feature + bugfix support
- LTS line: security + critical reliability fixes for **18 months** from LTS designation
- Overlap window: at least **90 days** where old and new LTS are both supported

## Security patch SLA

- Critical security issues: patch target within **7 calendar days** after confirmed triage
- High severity: patch target within **30 days**
- Medium/low: bundled in normal maintenance updates

## Compatibility guarantees for LTS

- No breaking syntax/AST/runtime contract changes in LTS patch updates
- Machine-readable API contracts (CLI JSON fields/error codes) remain backward-compatible

## End-of-life

When an LTS line reaches EOL:

- final advisory is published,
- upgrade target is documented,
- migration playbook references are provided.
