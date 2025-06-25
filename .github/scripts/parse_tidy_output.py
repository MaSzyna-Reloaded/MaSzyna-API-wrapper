import sys
import re
import os

def parse_clang_tidy_output(output_str):
    issues = []
    # Regex to capture file, line, column, type (warning/error), message, and rule
    # Example line: /path/to/file.cpp:123:45: warning: message [rule-name]
    # Another example (from run-clang-tidy.py, often without column):
    # /path/to/file.cpp:123: warning: message [rule-name]
    # Or errors that might not have a rule in brackets at the end immediately
    # /path/to/file.cpp:123:45: error: some error
    # ---
    # src/core/TrainSystem.hpp:35:1: error: no newline at end of file [readability-eof]
    # src/core/TrainSystem.hpp:35:1: warning: no newline at end of file [readability-eof]
    # src/core/TrainSystem.hpp:35: error: no newline at end of file [readability-eof]
    # src/core/TrainSystem.hpp:35: warning: no newline at end of file [readability-eof]

    # A more robust regex to capture the main parts
    # It handles optional column numbers and variations in how rules are presented.
    regex = re.compile(
        r"^(?P<file>[^:]+):(?P<line>\d+):(?:(?P<column>\d+):)?\s*"
        r"(?P<type>error|warning):\s*(?P<message>.*?)(?:\s*\[(?P<rule>[^\]]+)\])?$"
    )

    for line in output_str.splitlines():
        line = line.strip()
        if not line:
            continue

        match = regex.match(line)
        if match:
            data = match.groupdict()
            # Make file path relative to GITHUB_WORKSPACE for cleaner output
            file_path = data['file']
            if os.environ.get('GITHUB_WORKSPACE'):
                file_path = os.path.relpath(data['file'], os.environ['GITHUB_WORKSPACE'])

            issues.append({
                "file": file_path,
                "line": data["line"],
                "column": data.get("column") or "N/A", # Column might not always be present
                "type": data["type"].capitalize(),
                "message": data["message"].strip(),
                "rule": data.get("rule") or "N/A"
            })
        elif "error generated." in line or "warning generated." in line or "errors generated." in line or "warnings generated." in line :
            # This is summary line from clang-tidy, ignore it for the table
            pass
        elif "Suppressed " in line and " non-compiler errors" in line: # from run-clang-tidy.py
            pass
        elif line.startswith("Skipping"): # from run-clang-tidy.py
            pass
        elif "Use -header-filter" in line: # from run-clang-tidy.py
             pass
        elif line.startswith("Found compiler error(s)"): # from run-clang-tidy.py
            pass
        elif "Error while processing" in line: # from run-clang-tidy.py
            # This is a more serious error, maybe add it as a general error?
            # For now, just make sure it doesn't break parsing.
            issues.append({
                "file": "Build Process",
                "line": "N/A",
                "column": "N/A",
                "type": "Error",
                "message": line,
                "rule": "Build Error"
            })


    return issues

def format_issues_as_markdown_table(issues):
    if not issues:
        return "✅ No clang-tidy issues found."

    header = "| File | Line | Column | Type | Message | Rule |\n"
    separator = "|---|---|---|---|---|---|\n"
    body = ""

    for issue in issues:
        body += f"| {issue['file']} | {issue['line']} | {issue['column']} | {issue['type']} | {issue['message']} | {issue['rule']} |\n"

    return f"clang-tidy found {len(issues)} issues:\n\n{header}{separator}{body}"

if __name__ == "__main__":
    # The clang-tidy output is passed as a single string argument
    # when run from GitHub Actions like:
    # python script.py "${{ steps.run_tidy.outputs.report }}"
    if len(sys.argv) > 1:
        tidy_output = sys.argv[1]
    else:
        # Read from stdin if no argument is provided (for local testing)
        tidy_output = sys.stdin.read()

    parsed_issues = parse_clang_tidy_output(tidy_output)
    markdown_table = format_issues_as_markdown_table(parsed_issues)
    print(markdown_table)
