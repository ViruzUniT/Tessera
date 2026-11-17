// PreToolUse (Write|Edit): asks the user before an edit adds a backend include to shared code.
// Only *new* includes are flagged, so existing known violations don't trigger it.
const fs = require("fs");
const path = require("path");

const SHARED = /^src\/(tessera\/(include\/tessera\/|src\/)(core|platform|renderer|rhi)\/|tessera\/include\/tessera\/pch\.h$|sandbox\/)/;
const FORBIDDEN = /^(d3d12|d3dx12|d3dcompiler|dxgi|dxcapi|wrl|vulkan|tessera\/rhi_dx12\/|backends\/imgui_impl_(dx12|vulkan))/i;
const INCLUDE = /^\s*#\s*include\s*[<"]([^>"]+)[>"]/gm;

function includes(text) {
  const out = new Set();
  for (const m of (text || "").matchAll(INCLUDE)) out.add(m[1].replace(/\\/g, "/"));
  return out;
}

let raw = "";
process.stdin.on("data", (c) => (raw += c));
process.stdin.on("end", () => {
  let input;
  try { input = JSON.parse(raw); } catch { process.exit(0); }
  const ti = input.tool_input || {};
  const file = ti.file_path;
  if (!file) process.exit(0);

  const root = process.env.CLAUDE_PROJECT_DIR || input.cwd || process.cwd();
  const rel = path.relative(root, path.resolve(file)).replace(/\\/g, "/");
  if (!SHARED.test(rel)) process.exit(0);

  let before, after;
  if (input.tool_name === "Write") {
    after = ti.content;
    try { before = fs.readFileSync(file, "utf8"); } catch { before = ""; }
  } else {
    after = ti.new_string;
    before = ti.old_string;
  }

  const old = includes(before);
  const added = [...includes(after)].filter((i) => FORBIDDEN.test(i) && !old.has(i));
  if (added.length === 0) process.exit(0);

  const reason =
    `Layering: ${rel} is shared code but this edit adds backend include(s): ${added.join(", ")}. ` +
    `Dependencies must flow app → renderer → rhi → rhi_dx12 (see CLAUDE.md, docs/architecture.md).`;
  process.stdout.write(JSON.stringify({
    hookSpecificOutput: {
      hookEventName: "PreToolUse",
      permissionDecision: "ask",
      permissionDecisionReason: reason,
    },
  }));
  process.exit(0);
});
