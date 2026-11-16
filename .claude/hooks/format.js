// PostToolUse hook: run clang-format (repo .clang-format) on C/C++ files Claude edits.
const { execFileSync } = require("child_process");

let input = "";
process.stdin.on("data", (c) => (input += c));
process.stdin.on("end", () => {
  let file;
  try {
    const data = JSON.parse(input);
    file = data.tool_input?.file_path ?? data.tool_response?.filePath;
  } catch {
    return;
  }
  if (!file || !/\.(c|cc|cpp|cxx|h|hh|hpp|hxx|inl)$/i.test(file)) return;
  if (/[\\/]vendor[\\/]/i.test(file)) return; // never reformat third-party code
  try {
    execFileSync("clang-format", ["-i", "--style=file", file], { stdio: "pipe" });
  } catch (e) {
    process.stderr.write(`clang-format failed on ${file}: ${e.message}\n`);
  }
});
