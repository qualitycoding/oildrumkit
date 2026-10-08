// Headless-Chromium smoke test of web/oildrumkit.html: loads the page, strikes every pad via its
// computer key, and records the peak of what the ScriptProcessor callback produces.
//   NODE_PATH=$(npm root -g) node tests/browser_smoke.js web/oildrumkit.html
const { chromium } = require('playwright');
(async () => {
  const b = await chromium.launch({ executablePath: process.env.CHROMIUM || '/opt/pw-browsers/chromium', args: ['--autoplay-policy=no-user-gesture-required', '--no-sandbox'] }).catch(async () => chromium.launch({ args: ['--autoplay-policy=no-user-gesture-required', '--no-sandbox'] }));
  const p = await b.newPage(); const errs = [];
  p.on('pageerror', e => errs.push(String(e))); p.on('console', m => { if (m.type() === 'error' && !/ERR_TUNNEL|fonts\.g/.test(m.text())) errs.push(m.text()); });
  await p.addInitScript(() => {
    window.__peak = 0; window.__blocks = 0;
    const orig = AudioContext.prototype.createScriptProcessor;
    AudioContext.prototype.createScriptProcessor = function (...a) {
      const n = orig.apply(this, a);
      // the page assigns onaudioprocess synchronously; our listener is added afterwards so it sees the filled buffer
      setTimeout(() => n.addEventListener('audioprocess', ev => { window.__blocks++; const d = ev.outputBuffer.getChannelData(0); for (const x of d) window.__peak = Math.max(window.__peak, Math.abs(x)); }), 0);
      return n;
    };
  });
  await p.goto('file://' + require('path').resolve(process.argv[2] || 'web/oildrumkit.html'));
  const keys = 'qwertyuiopasdfg'.split(''); const res = [];
  for (const k of keys) { await p.evaluate(() => { window.__peak = 0; }); await p.keyboard.press(k); await p.waitForTimeout(250); res.push([k, await p.evaluate(() => window.__peak)]); }
  const blocks = await p.evaluate(() => window.__blocks);
  console.log('audio blocks processed:', blocks);
  console.log(res.map(([k, v]) => `${k}:${v.toFixed(3)}`).join(' '));
  console.log('page errors:', errs.length ? errs : 'none');
  const ok = errs.length === 0 && blocks > 0 && res.every(([, v]) => v > 0.005 && v < 1);
  console.log(ok ? 'PASS' : 'FAIL'); await b.close(); process.exit(ok ? 0 : 1);
})();
