#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
PROJECT="$REPO_ROOT/TorqueAtlas.uproject"

SKIP_GENERATE=0
SKIP_BUILD=0
UE_ROOT="${UE_ROOT:-}"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --ue-root)
      UE_ROOT="$2"
      shift 2
      ;;
    --skip-generate)
      SKIP_GENERATE=1
      shift
      ;;
    --skip-build)
      SKIP_BUILD=1
      shift
      ;;
    *)
      echo "Unknown argument: $1" >&2
      exit 2
      ;;
  esac
done

if [[ ! -f "$PROJECT" ]]; then
  echo "TorqueAtlas.uproject not found: $PROJECT" >&2
  exit 1
fi

if [[ -z "$UE_ROOT" ]]; then
  for candidate in \
    "/opt/UnrealEngine-5.8" \
    "/opt/UnrealEngine/UE_5.8" \
    "/Users/Shared/Epic Games/UE_5.8"; do
    if [[ -d "$candidate" ]]; then
      UE_ROOT="$candidate"
      break
    fi
  done
fi

if [[ -z "$UE_ROOT" || ! -d "$UE_ROOT" ]]; then
  echo "UE 5.8 root not found. Set UE_ROOT or pass --ue-root." >&2
  exit 1
fi

BUILD_VERSION="$UE_ROOT/Engine/Build/Build.version"
if [[ ! -f "$BUILD_VERSION" ]]; then
  echo "Unreal Build.version not found: $BUILD_VERSION" >&2
  exit 1
fi

read -r UE_MAJOR UE_MINOR UE_PATCH < <(
  python3 - "$BUILD_VERSION" <<PY
import json
import sys
with open(sys.argv[1], encoding="utf-8") as handle:
    version = json.load(handle)
print(version.get("MajorVersion", -1), version.get("MinorVersion", -1), version.get("PatchVersion", -1))
PY
)

if [[ "$UE_MAJOR" != "5" || "$UE_MINOR" != "8" ]]; then
  echo "Expected Unreal Engine 5.8, found $UE_MAJOR.$UE_MINOR.$UE_PATCH" >&2
  exit 1
fi

HOST="$(uname -s)"
case "$HOST" in
  Linux)
    TARGET_PLATFORM="Linux"
    EDITOR_CMD="$UE_ROOT/Engine/Binaries/Linux/UnrealEditor-Cmd"
    ;;
  Darwin)
    TARGET_PLATFORM="Mac"
    EDITOR_CMD="$UE_ROOT/Engine/Binaries/Mac/UnrealEditor-Cmd"
    ;;
  *)
    echo "Unsupported host: $HOST" >&2
    exit 1
    ;;
esac

STAMP="$(date -u +%Y%m%d-%H%M%S)"
VERIFY_ROOT="$REPO_ROOT/Saved/Verification/$STAMP"
REPORT_ROOT="$VERIFY_ROOT/AutomationReport"
mkdir -p "$REPORT_ROOT"

GIT_SHA="$(git -C "$REPO_ROOT" rev-parse HEAD 2>/dev/null || echo unknown)"
BUILD_EXIT="null"
AUTOMATION_EXIT="null"

write_metadata() {
  local status="$1"
  local failure="${2:-}"

  python3 - "$VERIFY_ROOT/metadata.json" "$status" "$BUILD_EXIT" "$AUTOMATION_EXIT" "$failure" "$REPO_ROOT" "$GIT_SHA" "$UE_ROOT" "$UE_MAJOR.$UE_MINOR.$UE_PATCH" "$PROJECT" "$TARGET_PLATFORM" <<PY
import json
import sys
from datetime import datetime, timezone

(path, status, build_exit, automation_exit, failure, repo_root, git_sha, ue_root, ue_version, project, platform) = sys.argv[1:]

def parse_exit(value):
    return None if value == "null" else int(value)

data = {
    "updated_utc": datetime.now(timezone.utc).isoformat(),
    "repo_root": repo_root,
    "git_sha": git_sha,
    "unreal_root": ue_root,
    "unreal_version": ue_version,
    "project": project,
    "target": "TorqueAtlasEditor",
    "platform": platform,
    "configuration": "Development",
    "automation_filter": "TorqueAtlas.",
    "status": status,
    "build_exit_code": parse_exit(build_exit),
    "automation_exit_code": parse_exit(automation_exit),
}
if failure:
    data["failure"] = failure

with open(path, "w", encoding="utf-8") as handle:
    json.dump(data, handle, indent=2)
    handle.write("\n")
PY
}

write_metadata "running"

if [[ "$SKIP_GENERATE" -eq 0 ]]; then
  GENERATE="$UE_ROOT/Engine/Build/BatchFiles/GenerateProjectFiles.sh"
  if [[ -x "$GENERATE" ]]; then
    set +e
    "$GENERATE" -project="$PROJECT" -game -engine 2>&1 | tee "$VERIFY_ROOT/generate-project-files.log"
    GENERATE_EXIT=${PIPESTATUS[0]}
    set -e
    if [[ "$GENERATE_EXIT" -ne 0 ]]; then
      write_metadata "failed" "GenerateProjectFiles failed with exit code $GENERATE_EXIT"
      exit "$GENERATE_EXIT"
    fi
  else
    echo "GenerateProjectFiles.sh not found/executable; continuing directly to UBT build."
  fi
fi

if [[ "$SKIP_BUILD" -eq 0 ]]; then
  BUILD="$UE_ROOT/Engine/Build/BatchFiles/Build.sh"
  if [[ ! -x "$BUILD" ]]; then
    write_metadata "failed" "Build.sh not found: $BUILD"
    echo "Build.sh not found: $BUILD" >&2
    exit 1
  fi

  set +e
  "$BUILD" TorqueAtlasEditor "$TARGET_PLATFORM" Development "$PROJECT" -WaitMutex -NoHotReload 2>&1 | tee "$VERIFY_ROOT/build.log"
  BUILD_EXIT=${PIPESTATUS[0]}
  set -e
  write_metadata "running"

  if [[ "$BUILD_EXIT" -ne 0 ]]; then
    write_metadata "failed" "UnrealBuildTool build failed with exit code $BUILD_EXIT"
    exit "$BUILD_EXIT"
  fi
fi

if [[ ! -x "$EDITOR_CMD" ]]; then
  write_metadata "failed" "UnrealEditor-Cmd not found: $EDITOR_CMD"
  echo "UnrealEditor-Cmd not found: $EDITOR_CMD" >&2
  exit 1
fi

set +e
"$EDITOR_CMD" "$PROJECT" \
  -Unattended \
  -NullRHI \
  -NoSplash \
  -NoSound \
  -stdout \
  -FullStdOutLogOutput \
  "-ExecCmds=Automation RunTest TorqueAtlas." \
  "-TestExit=Automation Test Queue Empty" \
  "-ReportExportPath=$REPORT_ROOT" \
  "-abslog=$VERIFY_ROOT/automation.log" \
  2>&1 | tee "$VERIFY_ROOT/automation-stdout.log"
AUTOMATION_EXIT=${PIPESTATUS[0]}
set -e

if [[ "$AUTOMATION_EXIT" -ne 0 ]]; then
  write_metadata "failed" "Unreal Automation failed with exit code $AUTOMATION_EXIT"
  exit "$AUTOMATION_EXIT"
fi

REPORT_VALIDATOR="$REPO_ROOT/Scripts/validate_automation_report.py"
REPORT_SUMMARY="$VERIFY_ROOT/automation-report-summary.json"

if [[ ! -f "$REPORT_VALIDATOR" ]]; then
  write_metadata "failed" "Automation report validator not found: $REPORT_VALIDATOR"
  echo "Automation report validator not found: $REPORT_VALIDATOR" >&2
  exit 1
fi

set +e
python3 "$REPORT_VALIDATOR" "$REPORT_ROOT" --prefix "TorqueAtlas." --summary-output "$REPORT_SUMMARY" 2>&1 | tee "$VERIFY_ROOT/automation-report-validation.log"
REPORT_VALIDATION_EXIT=${PIPESTATUS[0]}
set -e

if [[ "$REPORT_VALIDATION_EXIT" -ne 0 ]]; then
  write_metadata "failed" "Automation JSON report validation failed with exit code $REPORT_VALIDATION_EXIT"
  exit "$REPORT_VALIDATION_EXIT"
fi

write_metadata "passed"

echo
echo "Torque Atlas UE verification completed successfully."
echo "Artifacts: $VERIFY_ROOT"
