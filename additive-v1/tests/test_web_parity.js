// Parity test: the JavaScript engine (web/engine.js) against the C++ engine's reference renders.
//   build/dump_reference build/ref [sr]; node tests/test_web_parity.js build/ref [sr]
// Compares sample-by-sample (same PRNG, same order of operations). C++ runs in float32 and JS in
// float64, so tiny differences are expected; tolerance is stated below and reported as measured.
'use strict';
const fs = require('fs'), path = require('path');
const { DrumEngine } = require('../web/engine.js');
const kit = JSON.parse(fs.readFileSync(path.join(__dirname, '../web/kit.json'), 'utf8'));
const dir = process.argv[2] || 'build/ref', sr = Number(process.argv[3] || 44100);
const TOL_REL = 2e-3;   // max |js - cpp| as a fraction of the reference peak of that file
let fails = 0, checks = 0;
const check = (ok, msg) => { ++checks; if (!ok) { ++fails; console.log('FAIL', msg); } };
const readF32 = f => { const b = fs.readFileSync(f); return new Float32Array(b.buffer, b.byteOffset, b.length / 4); };
function compare(name, js, ref) {
  let peak = 0, maxd = 0, at = 0, n = Math.min(js.length, ref.length);
  for (let i = 0; i < n; ++i) { peak = Math.max(peak, Math.abs(ref[i])); const d = Math.abs(js[i] - ref[i]); if (d > maxd) { maxd = d; at = i; } }
  const rel = maxd / Math.max(peak, 1e-9);
  console.log(`${name.padEnd(16)} peak ${peak.toFixed(4)}  max|diff| ${maxd.toExponential(2)}  rel ${rel.toExponential(2)} (sample ${at})`);
  check(js.length === ref.length, `${name}: length ${js.length} vs ${ref.length}`);
  check(peak > 0.01, `${name}: reference not silent`);
  check(rel < TOL_REL, `${name}: rel diff ${rel} >= ${TOL_REL}`);
}
const n = Math.floor(sr * 1.5);
kit.instruments.forEach((inst, i) => {
  const e = new DrumEngine(kit, sr), out = new Float32Array(n);
  e.trigger(i, 1);
  for (let pos = 0; pos < n; pos += 256) e.render(out, Math.min(256, n - pos), pos);
  compare(inst.name, out, readF32(`${dir}/inst${i}.f32`));
});
{
  const e = new DrumEngine(kit, sr), out = new Float32Array(Math.floor(sr * 2));
  e.setPitchHz(1, 250); e.setPitchHz(3, 70);
  for (let pos = 0; pos < out.length; pos += 256) {
    if (pos === 0) { e.trigger(0, 1); e.trigger(10, 0.8); e.trigger(3, 0.9); e.trigger(1, 1); }
    if (pos === 256 * 40) e.trigger(12, 0.7);
    if (pos === 256 * 60) e.trigger(11, 0.7);
    if (pos === 256 * 80) { e.trigger(2, 0.6); e.trigger(2, 1); }
    e.render(out, Math.min(256, out.length - pos), pos);
  }
  compare('groove', out, readF32(`${dir}/groove.f32`));
}
console.log(`${checks} checks, ${fails} failed`);
process.exit(fails ? 1 : 0);
