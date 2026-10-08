// engine.js -- JavaScript port of Source/DrumEngine.h (same algorithm, same order of operations,
// same xorshift noise generator). Used by the browser demo and by tests/test_web_parity.js.
// Recipes are NOT written here: they come from kit.json, which tools/export_recipes.cpp generates
// from the C++ tables. If you change the C++ engine, re-export and re-run the parity test.
(function (root, factory) {
  if (typeof module === 'object' && module.exports) module.exports = factory();
  else root.OilDrumEngine = factory();
})(typeof self !== 'undefined' ? self : this, function () {
  'use strict';
  const TWO_PI = 2 * Math.PI;

  class Biquad {
    constructor() { this.b0 = this.b1 = this.b2 = this.a1 = this.a2 = this.z1 = this.z2 = 0; }
    reset() { this.z1 = this.z2 = 0; }
    _common(freq, q, sr) {
      freq = Math.min(Math.max(freq, 20), sr * 0.45);
      const w0 = TWO_PI * freq / sr, alpha = Math.sin(w0) / (2 * Math.max(q, 0.05));
      return { c: Math.cos(w0), alpha, a0: 1 + alpha };
    }
    setBandpass(f, q, sr) {
      const { c, alpha, a0 } = this._common(f, q, sr);
      this.b0 = alpha / a0; this.b1 = 0; this.b2 = -alpha / a0; this.a1 = -2 * c / a0; this.a2 = (1 - alpha) / a0;
    }
    setHighpass(f, q, sr) {
      const { c, alpha, a0 } = this._common(f, q, sr);
      this.b0 = (1 + c) * 0.5 / a0; this.b1 = -(1 + c) / a0; this.b2 = this.b0; this.a1 = -2 * c / a0; this.a2 = (1 - alpha) / a0;
    }
    setLowpass(f, q, sr) {
      const { c, alpha, a0 } = this._common(f, q, sr);
      this.b0 = (1 - c) * 0.5 / a0; this.b1 = (1 - c) / a0; this.b2 = this.b0; this.a1 = -2 * c / a0; this.a2 = (1 - alpha) / a0;
    }
    process(x) {
      const y = this.b0 * x + this.z1;
      this.z1 = this.b1 * x - this.a1 * y + this.z2;
      this.z2 = this.b2 * x - this.a2 * y;
      return y;
    }
  }

  class NoiseGen {
    constructor() { this.state = 0x9E3779B9 >>> 0; }
    next() {
      let s = this.state;
      s ^= (s << 13); s >>>= 0;
      s ^= (s >>> 17);
      s ^= (s << 5); s >>>= 0;
      this.state = s;
      return (s | 0) / 2147483648;
    }
  }

  function polyBlep(t, dt) {
    if (t < dt) { t /= dt; return t + t - t * t - 1; }
    if (t > 1 - dt) { t = (t - 1) / dt; return t * t + t + t + 1; }
    return 0;
  }
  function blepSquare(phase, dt) {
    let v = phase < 0.5 ? 1 : -1;
    v += polyBlep(phase, dt);
    let p2 = phase + 0.5; if (p2 >= 1) p2 -= 1;
    v -= polyBlep(p2, dt);
    return v;
  }

  function makeVoice(kit) {
    const n = kit.maxPartials, m = kit.metalRatios.length;
    return {
      active: false, age: 0, hz: 200, velocity: 1, choking: false,
      phase: new Float32Array(n), envMul: new Float32Array(n), envCoef: new Float32Array(n),
      bp: Array.from({ length: n }, () => new Biquad()),
      mphase: new Float32Array(m), hp1: new Biquad(), hp2: new Biquad(), lp1: new Biquad(),
      metalEnv: 0, metalCoef: 0, clickEnv: 0, clickCoef: 0,
    };
  }

  class DrumEngine {
    // kit = parsed kit.json
    constructor(kit, sampleRate) {
      this.kit = kit;
      this.sr = sampleRate || 44100;
      this.noise = new NoiseGen();
      this.currentHz = kit.instruments.map(i => i.defaultHz);
      this.voices = kit.instruments.map(() => Array.from({ length: kit.maxInstances }, () => makeVoice(kit)));
    }
    timeConst(seconds) { return Math.exp(-1 / (seconds * this.sr)); }
    setPitchHz(inst, hz) {
      const r = this.kit.instruments[inst];
      this.currentHz[inst] = Math.min(Math.max(hz, r.minHz), r.maxHz);   // applies to the NEXT trigger only
    }
    trigger(inst, velocity) {
      const kit = this.kit, r = kit.instruments[inst], sr = this.sr;
      if (velocity === undefined) velocity = 1;
      if (r.chokeGroup !== 0)
        kit.instruments.forEach((o, k) => { if (k !== inst && o.chokeGroup === r.chokeGroup) this.voices[k].forEach(v => { if (v.active) v.choking = true; }); });
      let slot = null, oldest = -1;
      for (const v of this.voices[inst]) {
        if (!v.active) { slot = v; break; }
        if (v.age > oldest) { oldest = v.age; slot = v; }
      }
      slot.active = true; slot.age = 0; slot.hz = this.currentHz[inst];
      slot.velocity = Math.min(Math.max(velocity, 0), 1.5); slot.choking = false;
      slot.clickEnv = r.clickAmp * slot.velocity; slot.clickCoef = this.timeConst(r.clickDecay);
      for (let p = 0; p < kit.maxPartials; ++p) {
        const pt = r.partials[p];
        slot.phase[p] = 0;
        if (!pt) { slot.envMul[p] = 0; slot.envCoef[p] = 0; continue; }
        slot.envMul[p] = pt.amp * slot.velocity;
        slot.envCoef[p] = this.timeConst(Math.max(pt.decay, 0.001));
        if (pt.noise) { slot.bp[p].reset(); slot.bp[p].setBandpass(Math.max(20, slot.hz * pt.ratio), pt.q, sr); }
      }
      if (r.metal.amp > 0) {
        for (let k = 0; k < kit.metalRatios.length; ++k) slot.mphase[k] = 0.5 * (this.noise.next() + 1);
        slot.hp1.reset(); slot.hp2.reset(); slot.lp1.reset();
        slot.hp1.setHighpass(r.metal.hpHz, 0.7071, sr); slot.hp2.setHighpass(r.metal.hpHz, 0.7071, sr);
        slot.lp1.setLowpass(r.metal.lpHz, 0.7071, sr);
        slot.metalEnv = r.metal.amp * slot.velocity; slot.metalCoef = this.timeConst(Math.max(r.metal.decay, 0.001));
      } else slot.metalEnv = 0;
    }
    allNotesOff() { this.voices.forEach(b => b.forEach(v => { v.active = false; })); }

    // Final output (master gain + tanh), writes numSamples into out[offset..]. |out| < 1.
    render(out, numSamples, offset) {
      offset = offset || 0;
      for (let n = 0; n < numSamples; ++n) out[offset + n] = 0;
      this.renderRaw(out, numSamples, offset);
      const g = this.kit.masterGain;
      for (let n = 0; n < numSamples; ++n) out[offset + n] = Math.tanh(out[offset + n] * g);
    }
    // Adds the pre-master voice sum into out (caller clears).
    renderRaw(out, numSamples, offset) {
      offset = offset || 0;
      const kit = this.kit, sr = this.sr, noise = this.noise, ratios = kit.metalRatios;
      for (let inst = 0; inst < kit.instruments.length; ++inst) {
        const r = kit.instruments[inst], np = r.partials.length, metalOn = r.metal.amp > 0;
        for (const v of this.voices[inst]) {
          if (!v.active) continue;
          for (let n = 0; n < numSamples; ++n) {
            let sample = 0;
            const ageSec = v.age / sr;
            const pitchEnv = 1 + r.pitchEnvAmt * Math.exp(-ageSec / Math.max(r.pitchEnvDecay, 0.001));
            for (let p = 0; p < np; ++p) {
              const pt = r.partials[p];
              if (pt.noise) sample += v.envMul[p] * v.bp[p].process(noise.next()) * 3;
              else {
                const inc = TWO_PI * (v.hz * pt.ratio * pitchEnv) / sr;
                sample += v.envMul[p] * Math.sin(v.phase[p]);
                v.phase[p] += inc;
                if (v.phase[p] > TWO_PI) v.phase[p] -= TWO_PI;
              }
              v.envMul[p] *= v.choking ? 0.985 : v.envCoef[p];
            }
            if (metalOn) {
              let bank = 0;
              for (let k = 0; k < ratios.length; ++k) {
                const dt = v.hz * r.metal.baseScale * ratios[k] / sr;
                bank += blepSquare(v.mphase[k], dt);
                v.mphase[k] += dt; if (v.mphase[k] >= 1) v.mphase[k] -= 1;
              }
              const src = bank * (1 / ratios.length) + r.metal.noiseMix * noise.next();
              sample += v.metalEnv * v.lp1.process(v.hp2.process(v.hp1.process(src)));
              v.metalEnv *= v.choking ? 0.985 : v.metalCoef;
            }
            sample += v.clickEnv * noise.next();
            v.clickEnv *= v.clickCoef;
            out[offset + n] += sample;
            v.age += 1;
          }
          let audible = v.clickEnv > 0.0002 || (metalOn && v.metalEnv > 0.0002);
          for (let p = 0; p < np && !audible; ++p) if (v.envMul[p] > 0.0002) audible = true;
          if (!audible || v.age > sr * 8) v.active = false;
        }
      }
    }
  }
  return { DrumEngine, NoiseGen, Biquad };
});
