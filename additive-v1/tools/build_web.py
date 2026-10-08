#!/usr/bin/env python3
"""Builds web/oildrumkit.html (one self-contained file) from the template, engine.js and kit.json.
   Run from the project root:  python3 tools/build_web.py"""
import json, pathlib
root = pathlib.Path(__file__).resolve().parent.parent
t = (root / 'web/oildrumkit.template.html').read_text()
engine = (root / 'web/engine.js').read_text()
kit = json.dumps(json.loads((root / 'web/kit.json').read_text()), separators=(',', ':'))
assert '</script' not in engine
out = t.replace('/*ENGINE_JS*/', engine).replace('/*KIT_JSON*/', kit)
(root / 'web/oildrumkit.html').write_text(out)
print('wrote web/oildrumkit.html', len(out), 'bytes')
