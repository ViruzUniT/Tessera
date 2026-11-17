// Two modes:
//   mark  (PostToolUse Write|Edit): remember that Claude touched build-relevant files this session.
//   check (Stop): if anything was touched, build DEBUG (and RELEASE for premake/config changes).
//         Errors are fed back to Claude (exit 2), at most MAX_ATTEMPTS times per session.
const fs = require("fs");
const os = require("os");
const path = require("path");
const { spawnSync } = require("child_process");

const MAX_ATTEMPTS = 3;
const BUILD_EXT = /\.(c|cc|cpp|cxx|h|hh|hpp|hxx|inl|hlsl|hlsli|lua)$/i;
const RELEASE_TRIGGER = /(\.lua$|core\/config\.h$|imgui|dev_ui)/i;

const mode = process.argv[2];
let raw = "";
process.stdin.on("data", (c) => (raw += c));
process.stdin.on("end", () => {
  let input = {};
  try { input = JSON.parse(raw); } catch {}
  const root = process.env.CLAUDE_PROJECT_DIR || input.cwd || process.cwd();
  const marker = path.join(os.tmpdir(), `tessera-claude-build-${input.session_id || "unknown"}.json`);
  const load = () => { try { return JSON.parse(fs.readFileSync(marker, "utf8")); } catch { return null; } };
  const save = (s) => fs.writeFileSync(marker, JSON.stringify(s));
  const clear = () => { try { fs.unlinkSync(marker); } catch {} };

  if (mode === "mark") {
    const file = (input.tool_input || {}).file_path || (input.tool_response || {}).filePath;
    if (!file) return;
    const rel = path.relative(root, path.resolve(file)).replace(/\\/g, "/");
    if (!BUILD_EXT.test(rel) || rel.startsWith("vendor/") || rel.startsWith("..")) return;
    const s = load() || { files: [], attempts: 0, warned: false };
    if (!s.files.includes(rel)) s.files.push(rel);
    s.warned = false;
    save(s);
    return;
  }

  if (mode !== "check") return;
  const s = load();
  if (!s || s.files.length === 0) return;

  const configs = ["DEBUG"];
  if (s.files.some((f) => RELEASE_TRIGGER.test(f))) configs.push("RELEASE");

  const shell = spawnSync("pwsh", ["-v"], { stdio: "ignore" }).error ? "powershell" : "pwsh";
  const touched = s.files.join(", ");
  for (const cfg of configs) {
    const r = spawnSync(shell, ["-NoProfile", "-ExecutionPolicy", "Bypass", "-File", "build.ps1", "build", cfg],
      { cwd: root, encoding: "utf8", maxBuffer: 64 * 1024 * 1024 });
    const out = `${r.stdout || ""}\n${r.stderr || ""}`;
    const lines = out.split(/\r?\n/);
    const errors = lines.filter((l) => /\berror\b/i.test(l));

    if (r.status !== 0 || r.error) {
      s.attempts += 1;
      if (s.attempts >= MAX_ATTEMPTS) {
        clear();
        process.stdout.write(JSON.stringify({
          systemMessage: `build-check: ${cfg} build still failing after ${MAX_ATTEMPTS} attempts; giving up.`,
        }));
        return;
      }
      save(s);
      const shown = (errors.length ? errors : lines.slice(-60)).slice(0, 60).join("\n");
      process.stderr.write(
        `./build.ps1 build ${cfg} failed. Files you edited this session: ${touched}.\n` +
        `Fix errors caused by your edits. If the errors are in files you did not touch ` +
        `(the user's work in progress), do not fix them: report them to the user and stop.\n\n${shown}\n`);
      process.exit(2);
    }

    const warnings = lines.filter((l) => /\bwarning\b/i.test(l));
    if (warnings.length && !s.warned && s.attempts < MAX_ATTEMPTS - 1) {
      s.warned = true;
      s.attempts += 1;
      save(s);
      process.stderr.write(
        `./build.ps1 build ${cfg} succeeded with warnings. Files you edited this session: ${touched}.\n` +
        `Fix warnings in lines you changed; mention any others to the user without fixing them.\n\n` +
        warnings.slice(0, 40).join("\n") + "\n");
      process.exit(2);
    }
  }
  clear();
  process.stdout.write(JSON.stringify({ systemMessage: `build-check: ${configs.join(" + ")} build OK` }));
});
