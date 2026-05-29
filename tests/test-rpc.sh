#!/usr/bin/env bash
# test-rpc.sh — Integration tests for openmediavault-snapraid RPC methods.
#
# Usage: sudo ./tests/test-rpc.sh
#
# Exercises all SnapRaid RPC methods: settings CRUD, array/drive/rule CRUD,
# informational RPCs, and background command dispatch.  No physical devices
# are required; drive tests use synthetic mount-entry UUIDs to exercise DB
# operations without a real filesystem.
#
# WARNING: This script transiently modifies SnapRAID settings and creates/
# deletes arrays, drives, and rules in the OMV config database.  Run on a
# test system or during a maintenance window.

set -uo pipefail

# ---------------------------------------------------------------------------
# Colours / counters  (display → stderr; $() captures only JSON)
# ---------------------------------------------------------------------------
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m'

PASS=0
FAIL=0
declare -a FAILED_TESTS=()

section() { echo -e "\n${CYAN}${BOLD}=== $* ===${NC}" >&2; }
info()    { echo -e "  ${YELLOW}»${NC} $*" >&2; }

_pass() {
    echo -e "  ${GREEN}PASS${NC}  $1" >&2
    ((PASS++)) || true
}
_fail() {
    echo -e "  ${RED}FAIL${NC}  $1" >&2
    [ -n "${2:-}" ] && echo -e "         ${RED}→${NC} $2" >&2
    ((FAIL++)) || true
    FAILED_TESTS+=("$1")
}

# ---------------------------------------------------------------------------
# RPC helpers
# ---------------------------------------------------------------------------
rpc() {
    local svc=$1 method=$2 params=${3:-'{}'}
    omv-rpc -u admin "$svc" "$method" "$params"
}

assert_rpc() {
    local desc=$1 svc=$2 method=$3 params=${4:-'{}'} pattern=${5:-}
    local out ec=0
    out=$(omv-rpc -u admin "$svc" "$method" "$params" 2>&1) || ec=$?
    if [ $ec -ne 0 ]; then
        _fail "$desc" "$(echo "$out" | tail -3)"
        return 1
    fi
    if [ -n "$pattern" ] && ! echo "$out" | grep -q "$pattern"; then
        _fail "$desc" "Pattern '$pattern' not found in: ${out:0:200}"
        return 1
    fi
    _pass "$desc"
    echo "$out"
    return 0
}

assert_rpc_fails() {
    local desc=$1 svc=$2 method=$3 params=${4:-'{}'}
    local out ec=0
    out=$(omv-rpc -u admin "$svc" "$method" "$params" 2>&1) || ec=$?
    if [ $ec -eq 0 ] && ! echo "$out" | grep -qi "exception"; then
        _fail "$desc" "Expected failure but RPC succeeded: ${out:0:200}"
        return 1
    fi
    _pass "$desc"
    return 0
}

# ---------------------------------------------------------------------------
# State
# ---------------------------------------------------------------------------
ARRAY1_UUID=""
ARRAY2_UUID=""
DRIVE1_UUID=""
DRIVE2_UUID=""
RULE1_UUID=""
RULE2_UUID=""
ORIG_SETTINGS=""

LIST_PARAMS='{"start":0,"limit":null,"sortfield":null,"sortdir":null}'
OMV_NEW_UUID=$(. /etc/default/openmediavault 2>/dev/null; \
    echo "${OMV_CONFIGOBJECT_NEW_UUID:-fa4b1c66-ef79-11e5-87a0-0002b3a176b4}")

# Synthetic mount-entry UUIDs for drive tests.  These won't exist in
# conf.system.filesystem.mountpoint; setDrive skips the fs lookup when the
# mntentref is absent, so the drive is still written to the config DB.
FAKE_MNTENTREF1=$(python3 -c "import uuid; print(uuid.uuid4())")
FAKE_MNTENTREF2=$(python3 -c "import uuid; print(uuid.uuid4())")

# ---------------------------------------------------------------------------
# Cleanup — always runs on exit
# ---------------------------------------------------------------------------
cleanup() {
    section "Cleanup"

    # Delete drives before arrays (drives reference arrays via arrayref)
    if [ -n "$DRIVE1_UUID" ]; then
        info "Deleting drive $DRIVE1_UUID"
        rpc "SnapRaid" "deleteDrive" "{\"uuid\":\"$DRIVE1_UUID\"}" &>/dev/null || true
    fi
    if [ -n "$DRIVE2_UUID" ]; then
        info "Deleting drive $DRIVE2_UUID"
        rpc "SnapRaid" "deleteDrive" "{\"uuid\":\"$DRIVE2_UUID\"}" &>/dev/null || true
    fi

    if [ -n "$RULE1_UUID" ]; then
        info "Deleting rule $RULE1_UUID"
        rpc "SnapRaid" "deleteRule" "{\"uuid\":\"$RULE1_UUID\"}" &>/dev/null || true
    fi
    if [ -n "$RULE2_UUID" ]; then
        info "Deleting rule $RULE2_UUID"
        rpc "SnapRaid" "deleteRule" "{\"uuid\":\"$RULE2_UUID\"}" &>/dev/null || true
    fi

    if [ -n "$ARRAY1_UUID" ]; then
        info "Deleting array $ARRAY1_UUID"
        rpc "SnapRaid" "deleteArray" "{\"uuid\":\"$ARRAY1_UUID\"}" &>/dev/null || true
    fi
    if [ -n "$ARRAY2_UUID" ]; then
        info "Deleting array $ARRAY2_UUID"
        rpc "SnapRaid" "deleteArray" "{\"uuid\":\"$ARRAY2_UUID\"}" &>/dev/null || true
    fi

    if [ -n "$ORIG_SETTINGS" ]; then
        info "Restoring original SnapRAID settings"
        rpc "SnapRaid" "setSettings" "$ORIG_SETTINGS" &>/dev/null || true
    fi

    info "Done."
}
trap cleanup EXIT

# ---------------------------------------------------------------------------
# Pre-flight
# ---------------------------------------------------------------------------
section "Pre-flight"

if [ "$(id -u)" -ne 0 ]; then
    echo -e "${RED}Must be run as root.${NC}" >&2
    exit 1
fi

for cmd in omv-rpc python3; do
    if command -v "$cmd" &>/dev/null; then
        _pass "command available: $cmd"
    else
        _fail "command available: $cmd" "$cmd not found in PATH"
    fi
done

if ! omv-rpc -u admin "Config" "isDirty" '{}' &>/dev/null; then
    echo -e "\n${RED}omv-rpc not functional — aborting.${NC}" >&2
    exit 1
fi
_pass "omv-rpc functional"

# ---------------------------------------------------------------------------
# Settings — read
# ---------------------------------------------------------------------------
section "Settings — getSettings"

ORIG_SETTINGS=$(assert_rpc "getSettings" "SnapRaid" "getSettings") || {
    echo -e "\n${RED}getSettings failed — aborting.${NC}" >&2
    exit 1
}

for field in blocksize hashsize autosave percentscrub nohidden debug sendmail \
             runscrub scrubfreq updthreshold delthreshold scrubpercent prehash; do
    if echo "$ORIG_SETTINGS" | python3 -c \
            "import sys,json; d=json.load(sys.stdin); sys.exit(0 if '$field' in d else 1)" \
            2>/dev/null; then
        _pass "getSettings — field '$field' present"
    else
        _fail "getSettings — field '$field' missing"
    fi
done

# ---------------------------------------------------------------------------
# Settings — write
# ---------------------------------------------------------------------------
section "Settings — setSettings"

assert_rpc "setSettings — change blocksize to 512" "SnapRaid" "setSettings" \
    '{"blocksize":512,"hashsize":16,"autosave":0,"percentscrub":12,"nohidden":false,
      "defaultarray":"","debug":false,"sendmail":true,"runscrub":true,"scrubfreq":7,
      "updthreshold":0,"delthreshold":0,"scrubpercent":100,"prehash":true}' \
    '"blocksize":512' >/dev/null

assert_rpc "setSettings — nohidden, sendmail off, thresholds" "SnapRaid" "setSettings" \
    '{"blocksize":256,"hashsize":16,"autosave":0,"percentscrub":12,"nohidden":true,
      "defaultarray":"","debug":false,"sendmail":false,"runscrub":false,"scrubfreq":14,
      "updthreshold":10,"delthreshold":5,"scrubpercent":50,"prehash":false}' \
    '"nohidden":true' >/dev/null

# Restore before negative tests to avoid bad state on early failure
rpc "SnapRaid" "setSettings" "$ORIG_SETTINGS" &>/dev/null || true

# ---------------------------------------------------------------------------
# Settings — negative tests
# ---------------------------------------------------------------------------
section "Settings — negative tests"

assert_rpc_fails "setSettings — blocksize out of range (>65535)" "SnapRaid" "setSettings" \
    '{"blocksize":99999,"hashsize":16,"autosave":0,"percentscrub":12,"nohidden":false,
      "defaultarray":"","debug":false,"sendmail":true,"runscrub":true,"scrubfreq":7,
      "updthreshold":0,"delthreshold":0,"scrubpercent":100,"prehash":true}'

assert_rpc_fails "setSettings — percentscrub > 100" "SnapRaid" "setSettings" \
    '{"blocksize":256,"hashsize":16,"autosave":0,"percentscrub":101,"nohidden":false,
      "defaultarray":"","debug":false,"sendmail":true,"runscrub":true,"scrubfreq":7,
      "updthreshold":0,"delthreshold":0,"scrubpercent":100,"prehash":true}'

assert_rpc_fails "setSettings — scrubpercent > 100" "SnapRaid" "setSettings" \
    '{"blocksize":256,"hashsize":16,"autosave":0,"percentscrub":12,"nohidden":false,
      "defaultarray":"","debug":false,"sendmail":true,"runscrub":true,"scrubfreq":7,
      "updthreshold":0,"delthreshold":0,"scrubpercent":101,"prehash":true}'

assert_rpc_fails "setSettings — missing required fields" "SnapRaid" "setSettings" \
    '{"blocksize":256,"autosave":0}'

# ---------------------------------------------------------------------------
# Array — create
# ---------------------------------------------------------------------------
section "Array — setArray (create)"

ARRAY1_RESULT=$(rpc "SnapRaid" "setArray" \
    "{\"uuid\":\"$OMV_NEW_UUID\",\"name\":\"srtestarray1\"}" 2>&1) \
    && ARRAY1_EC=0 || ARRAY1_EC=$?
ARRAY1_UUID=$(echo "$ARRAY1_RESULT" | python3 -c \
    "import sys,json; print(json.load(sys.stdin).get('uuid',''))" 2>/dev/null || echo "")
if [ $ARRAY1_EC -eq 0 ] && [ -n "$ARRAY1_UUID" ] && [ "$ARRAY1_UUID" != "$OMV_NEW_UUID" ]; then
    _pass "setArray (create srtestarray1) — UUID: $ARRAY1_UUID"
else
    _fail "setArray (create)" "${ARRAY1_RESULT:0:200}"
    ARRAY1_UUID=""
fi

ARRAY2_RESULT=$(rpc "SnapRaid" "setArray" \
    "{\"uuid\":\"$OMV_NEW_UUID\",\"name\":\"srtestarray2\"}" 2>&1) \
    && ARRAY2_EC=0 || ARRAY2_EC=$?
ARRAY2_UUID=$(echo "$ARRAY2_RESULT" | python3 -c \
    "import sys,json; print(json.load(sys.stdin).get('uuid',''))" 2>/dev/null || echo "")
if [ $ARRAY2_EC -eq 0 ] && [ -n "$ARRAY2_UUID" ] && [ "$ARRAY2_UUID" != "$OMV_NEW_UUID" ]; then
    _pass "setArray (create srtestarray2) — UUID: $ARRAY2_UUID"
else
    _fail "setArray (create second)" "${ARRAY2_RESULT:0:200}"
    ARRAY2_UUID=""
fi

# ---------------------------------------------------------------------------
# Array — read
# ---------------------------------------------------------------------------
section "Array — getArray / getArrayList"

if [ -n "$ARRAY1_UUID" ]; then
    assert_rpc "getArray — by UUID" "SnapRaid" "getArray" \
        "{\"uuid\":\"$ARRAY1_UUID\"}" "\"uuid\":\"$ARRAY1_UUID\"" >/dev/null

    STORED_NAME=$(rpc "SnapRaid" "getArray" \
        "{\"uuid\":\"$ARRAY1_UUID\"}" 2>/dev/null | \
        python3 -c "import sys,json; print(json.load(sys.stdin).get('name',''))" \
        2>/dev/null || echo "")
    [ "$STORED_NAME" = "srtestarray1" ] \
        && _pass "getArray — name stored correctly" \
        || _fail "getArray — name mismatch (got '$STORED_NAME')"
fi

LIST_RESULT=$(assert_rpc "getArrayList" "SnapRaid" "getArrayList" "$LIST_PARAMS") || true

if [ -n "$ARRAY1_UUID" ]; then
    if echo "$LIST_RESULT" | grep -q "srtestarray1"; then
        _pass "getArrayList — srtestarray1 present"
    else
        _fail "getArrayList — srtestarray1 not found"
    fi
    # getArrayList decorates each array with a drive count
    if echo "$LIST_RESULT" | grep -q '"drives"'; then
        _pass "getArrayList — drives count field present"
    else
        _fail "getArrayList — drives count field missing"
    fi
fi

section "Array — enumerateArrays"

assert_rpc "enumerateArrays (addnone=false)" "SnapRaid" "enumerateArrays" \
    '{"addnone":false}' >/dev/null

assert_rpc "enumerateArrays (addnone=true)" "SnapRaid" "enumerateArrays" \
    '{"addnone":true}' '"name":"None"' >/dev/null

section "Array — setArray (update)"

if [ -n "$ARRAY1_UUID" ]; then
    # Spaces in name should be replaced with underscores
    assert_rpc "setArray — update name with spaces" "SnapRaid" "setArray" \
        "{\"uuid\":\"$ARRAY1_UUID\",\"name\":\"sr test renamed\"}" \
        '"sr_test_renamed"' >/dev/null
    # Reset to original name for subsequent tests
    rpc "SnapRaid" "setArray" \
        "{\"uuid\":\"$ARRAY1_UUID\",\"name\":\"srtestarray1\"}" &>/dev/null || true
fi

section "Array — negative tests"

assert_rpc_fails "setArray — duplicate name" "SnapRaid" "setArray" \
    "{\"uuid\":\"$OMV_NEW_UUID\",\"name\":\"srtestarray1\"}"

assert_rpc_fails "getArray — unknown UUID" "SnapRaid" "getArray" \
    '{"uuid":"00000000-0000-0000-0000-000000000000"}'

assert_rpc_fails "deleteArray — unknown UUID" "SnapRaid" "deleteArray" \
    '{"uuid":"00000000-0000-0000-0000-000000000000"}'

# ---------------------------------------------------------------------------
# Rule — create
# ---------------------------------------------------------------------------
section "Rule — setRule (create)"

# Exclusion rule (rtype 0), no prepend
RULE1_RESULT=$(rpc "SnapRaid" "setRule" \
    "{\"uuid\":\"$OMV_NEW_UUID\",\"rule1\":\"*.bak\",\"rtype\":0,\"prepend\":false}" \
    2>&1) && RULE1_EC=0 || RULE1_EC=$?
RULE1_UUID=$(echo "$RULE1_RESULT" | python3 -c \
    "import sys,json; print(json.load(sys.stdin).get('uuid',''))" 2>/dev/null || echo "")
if [ $RULE1_EC -eq 0 ] && [ -n "$RULE1_UUID" ] && [ "$RULE1_UUID" != "$OMV_NEW_UUID" ]; then
    _pass "setRule (exclude *.bak) — UUID: $RULE1_UUID"
else
    _fail "setRule (exclude *.bak)" "${RULE1_RESULT:0:200}"
    RULE1_UUID=""
fi

# Inclusion rule (rtype 1) with prepend — RPC should add a leading slash
RULE2_RESULT=$(rpc "SnapRaid" "setRule" \
    "{\"uuid\":\"$OMV_NEW_UUID\",\"rule1\":\"important\",\"rtype\":1,\"prepend\":true}" \
    2>&1) && RULE2_EC=0 || RULE2_EC=$?
RULE2_UUID=$(echo "$RULE2_RESULT" | python3 -c \
    "import sys,json; print(json.load(sys.stdin).get('uuid',''))" 2>/dev/null || echo "")
if [ $RULE2_EC -eq 0 ] && [ -n "$RULE2_UUID" ] && [ "$RULE2_UUID" != "$OMV_NEW_UUID" ]; then
    _pass "setRule (include with prepend) — UUID: $RULE2_UUID"
    STORED_RULE=$(rpc "SnapRaid" "getRule" \
        "{\"uuid\":\"$RULE2_UUID\"}" 2>/dev/null | \
        python3 -c "import sys,json; print(json.load(sys.stdin).get('rule1',''))" \
        2>/dev/null || echo "")
    if echo "$STORED_RULE" | grep -q "^/"; then
        _pass "setRule — prepend added leading slash: $STORED_RULE"
    else
        _fail "setRule — prepend did not add leading slash (got '$STORED_RULE')"
    fi
else
    _fail "setRule (include with prepend)" "${RULE2_RESULT:0:200}"
    RULE2_UUID=""
fi

# ---------------------------------------------------------------------------
# Rule — read
# ---------------------------------------------------------------------------
section "Rule — getRule / getRuleList"

if [ -n "$RULE1_UUID" ]; then
    assert_rpc "getRule" "SnapRaid" "getRule" \
        "{\"uuid\":\"$RULE1_UUID\"}" "\"uuid\":\"$RULE1_UUID\"" >/dev/null
fi

RULES_LIST=$(assert_rpc "getRuleList" "SnapRaid" "getRuleList" "$LIST_PARAMS") || true

if [ -n "$RULE1_UUID" ]; then
    if echo "$RULES_LIST" | grep -q '\\*.bak'; then
        _pass "getRuleList — *.bak rule present"
    else
        _fail "getRuleList — *.bak rule not found"
    fi
fi

# ---------------------------------------------------------------------------
# Rule — negative tests
# ---------------------------------------------------------------------------
section "Rule — negative tests"

assert_rpc_fails "setRule — rule ending with /* rejected" "SnapRaid" "setRule" \
    "{\"uuid\":\"$OMV_NEW_UUID\",\"rule1\":\"somedir/*\",\"rtype\":0,\"prepend\":false}"

assert_rpc_fails "getRule — unknown UUID" "SnapRaid" "getRule" \
    '{"uuid":"00000000-0000-0000-0000-000000000000"}'

assert_rpc_fails "deleteRule — unknown UUID" "SnapRaid" "deleteRule" \
    '{"uuid":"00000000-0000-0000-0000-000000000000"}'

# ---------------------------------------------------------------------------
# Rule — delete
# ---------------------------------------------------------------------------
section "Rule — deleteRule"

if [ -n "$RULE1_UUID" ]; then
    assert_rpc "deleteRule (rule1)" "SnapRaid" "deleteRule" \
        "{\"uuid\":\"$RULE1_UUID\"}" >/dev/null
    assert_rpc_fails "getRule after delete (rule1)" "SnapRaid" "getRule" \
        "{\"uuid\":\"$RULE1_UUID\"}"
    RULE1_UUID=""
fi

if [ -n "$RULE2_UUID" ]; then
    assert_rpc "deleteRule (rule2)" "SnapRaid" "deleteRule" \
        "{\"uuid\":\"$RULE2_UUID\"}" >/dev/null
    RULE2_UUID=""
fi

# ---------------------------------------------------------------------------
# Drive — create (synthetic mntentref, no physical device needed)
# ---------------------------------------------------------------------------
section "Drive — setDrive (data + parity, synthetic mntentref)"

if [ -n "$ARRAY1_UUID" ]; then
    DRIVE1_RESULT=$(rpc "SnapRaid" "setDrive" "$(python3 -c "
import json; print(json.dumps({
    'uuid':        '$OMV_NEW_UUID',
    'arrayref':    '$ARRAY1_UUID',
    'mntentref':   '$FAKE_MNTENTREF1',
    'name':        'srtestdata',
    'content':     True,
    'data':        True,
    'parity':      False,
    'paritynum':   1,
    'paritysplit': False,
}))")" 2>&1) && DRIVE1_EC=0 || DRIVE1_EC=$?
    DRIVE1_UUID=$(echo "$DRIVE1_RESULT" | python3 -c \
        "import sys,json; print(json.load(sys.stdin).get('uuid',''))" 2>/dev/null || echo "")
    if [ $DRIVE1_EC -eq 0 ] && [ -n "$DRIVE1_UUID" ] && [ "$DRIVE1_UUID" != "$OMV_NEW_UUID" ]; then
        _pass "setDrive (data, synthetic mntentref) — UUID: $DRIVE1_UUID"
    else
        _fail "setDrive (data drive)" "${DRIVE1_RESULT:0:200}"
        DRIVE1_UUID=""
    fi

    DRIVE2_RESULT=$(rpc "SnapRaid" "setDrive" "$(python3 -c "
import json; print(json.dumps({
    'uuid':        '$OMV_NEW_UUID',
    'arrayref':    '$ARRAY1_UUID',
    'mntentref':   '$FAKE_MNTENTREF2',
    'name':        'srtestparity',
    'content':     False,
    'data':        False,
    'parity':      True,
    'paritynum':   1,
    'paritysplit': False,
}))")" 2>&1) && DRIVE2_EC=0 || DRIVE2_EC=$?
    DRIVE2_UUID=$(echo "$DRIVE2_RESULT" | python3 -c \
        "import sys,json; print(json.load(sys.stdin).get('uuid',''))" 2>/dev/null || echo "")
    if [ $DRIVE2_EC -eq 0 ] && [ -n "$DRIVE2_UUID" ] && [ "$DRIVE2_UUID" != "$OMV_NEW_UUID" ]; then
        _pass "setDrive (parity, synthetic mntentref) — UUID: $DRIVE2_UUID"
    else
        _fail "setDrive (parity drive)" "${DRIVE2_RESULT:0:200}"
        DRIVE2_UUID=""
    fi
else
    info "No array available — skipping drive CRUD tests"
fi

# ---------------------------------------------------------------------------
# Drive — read
# ---------------------------------------------------------------------------
section "Drive — getDrive / getDriveList"

if [ -n "$DRIVE1_UUID" ]; then
    assert_rpc "getDrive" "SnapRaid" "getDrive" \
        "{\"uuid\":\"$DRIVE1_UUID\"}" "\"uuid\":\"$DRIVE1_UUID\"" >/dev/null

    DRIVES_LIST=$(assert_rpc "getDriveList" "SnapRaid" "getDriveList" \
        "$LIST_PARAMS") || true

    if echo "$DRIVES_LIST" | grep -q "srtestdata"; then
        _pass "getDriveList — data drive present"
    else
        _fail "getDriveList — data drive not found"
    fi

    if echo "$DRIVES_LIST" | grep -q "srtestarray1"; then
        _pass "getDriveList — arrayname populated from arrayref"
    else
        _fail "getDriveList — arrayname not populated"
    fi
fi

# ---------------------------------------------------------------------------
# Drive — setDriveEmpty
# ---------------------------------------------------------------------------
section "Drive — setDriveEmpty"

if [ -n "$DRIVE1_UUID" ]; then
    assert_rpc "setDriveEmpty — enable" "SnapRaid" "setDriveEmpty" \
        "{\"uuid\":\"$DRIVE1_UUID\",\"emptydir\":true}" '"emptydir":true' >/dev/null
    assert_rpc "setDriveEmpty — disable" "SnapRaid" "setDriveEmpty" \
        "{\"uuid\":\"$DRIVE1_UUID\",\"emptydir\":false}" '"emptydir":false' >/dev/null
fi

# ---------------------------------------------------------------------------
# Drive — negative tests
# ---------------------------------------------------------------------------
section "Drive — negative tests"

if [ -n "$ARRAY1_UUID" ]; then
    EXTRA_MNTENTREF=$(python3 -c "import uuid; print(uuid.uuid4())")

    assert_rpc_fails "setDrive — data+parity rejected" "SnapRaid" "setDrive" \
        "$(python3 -c "
import json; print(json.dumps({
    'uuid':        '$OMV_NEW_UUID',
    'arrayref':    '$ARRAY1_UUID',
    'mntentref':   '$EXTRA_MNTENTREF',
    'name':        'srtestbad',
    'content':     False,
    'data':        True,
    'parity':      True,
    'paritynum':   1,
    'paritysplit': False,
}))")"

    assert_rpc_fails "setDrive — data+paritysplit rejected" "SnapRaid" "setDrive" \
        "$(python3 -c "
import json; print(json.dumps({
    'uuid':        '$OMV_NEW_UUID',
    'arrayref':    '$ARRAY1_UUID',
    'mntentref':   '$EXTRA_MNTENTREF',
    'name':        'srtestbad2',
    'content':     False,
    'data':        True,
    'parity':      False,
    'paritynum':   1,
    'paritysplit': True,
}))")"

    if [ -n "$DRIVE1_UUID" ]; then
        # Duplicate mntentref for a new drive should fail
        assert_rpc_fails "setDrive — duplicate mntentref rejected" "SnapRaid" "setDrive" \
            "$(python3 -c "
import json; print(json.dumps({
    'uuid':        '$OMV_NEW_UUID',
    'arrayref':    '$ARRAY1_UUID',
    'mntentref':   '$FAKE_MNTENTREF1',
    'name':        'srtestdupdrv',
    'content':     True,
    'data':        True,
    'parity':      False,
    'paritynum':   1,
    'paritysplit': False,
}))")"
    fi
fi

if [ -n "$DRIVE2_UUID" ]; then
    # setDriveEmpty on a parity drive should fail
    assert_rpc_fails "setDriveEmpty — parity drive rejected" "SnapRaid" "setDriveEmpty" \
        "{\"uuid\":\"$DRIVE2_UUID\",\"emptydir\":true}"
fi

assert_rpc_fails "getDrive — unknown UUID" "SnapRaid" "getDrive" \
    '{"uuid":"00000000-0000-0000-0000-000000000000"}'

assert_rpc_fails "deleteDrive — unknown UUID" "SnapRaid" "deleteDrive" \
    '{"uuid":"00000000-0000-0000-0000-000000000000"}'

# ---------------------------------------------------------------------------
# Drive — delete
# ---------------------------------------------------------------------------
section "Drive — deleteDrive"

if [ -n "$DRIVE1_UUID" ]; then
    assert_rpc "deleteDrive (data)" "SnapRaid" "deleteDrive" \
        "{\"uuid\":\"$DRIVE1_UUID\"}" >/dev/null
    assert_rpc_fails "getDrive after delete (data)" "SnapRaid" "getDrive" \
        "{\"uuid\":\"$DRIVE1_UUID\"}"
    DRIVE1_UUID=""
fi

if [ -n "$DRIVE2_UUID" ]; then
    assert_rpc "deleteDrive (parity)" "SnapRaid" "deleteDrive" \
        "{\"uuid\":\"$DRIVE2_UUID\"}" >/dev/null
    DRIVE2_UUID=""
fi

# ---------------------------------------------------------------------------
# Informational RPCs
# ---------------------------------------------------------------------------
section "Informational RPCs — getVersion"

VERSION_RESULT=$(assert_rpc "getVersion" "SnapRaid" "getVersion" '{}' '"version"') || true
VERSION=$(echo "$VERSION_RESULT" | python3 -c \
    "import sys,json; print(json.load(sys.stdin).get('version',''))" 2>/dev/null || echo "")
if [ -n "$VERSION" ]; then
    _pass "getVersion — snapraid version: $VERSION"
else
    _fail "getVersion — version field empty or missing"
fi

section "Informational RPCs — getConfig"

# getConfig reads /etc/snapraid.conf; returns empty strings if not yet deployed
assert_rpc "getConfig — returns snapraidconf key" "SnapRaid" "getConfig" \
    '{}' '"snapraidconf"' >/dev/null

assert_rpc "getConfig — returns snapraiddiff key" "SnapRaid" "getConfig" \
    '{}' '"snapraiddiff"' >/dev/null

section "Informational RPCs — getArrayConfig"

if [ -n "$ARRAY1_UUID" ]; then
    # Returns empty string if the conf file hasn't been generated yet (pre-deploy)
    assert_rpc "getArrayConfig — returns config key" "SnapRaid" "getArrayConfig" \
        "{\"uuid\":\"$ARRAY1_UUID\"}" '"config"' >/dev/null
fi

# ---------------------------------------------------------------------------
# executeCommand — background dispatch
# ---------------------------------------------------------------------------
section "executeCommand — background dispatch"

if [ -n "$ARRAY1_UUID" ]; then
    CONF_FILE="/etc/snapraid/omv-snapraid-${ARRAY1_UUID}.conf"
    if [ -f "$CONF_FILE" ]; then
        info "Conf file found: $CONF_FILE — testing executeCommand dispatch"
        # The parent process returns a bgStatusFilename before the child
        # executes, so the RPC succeeds regardless of command outcome.
        assert_rpc "executeCommand (diff) — bg job dispatched" \
            "SnapRaid" "executeCommand" \
            "{\"uuid\":\"$ARRAY1_UUID\",\"command\":\"diff\"}" >/dev/null
    else
        info "No conf file at $CONF_FILE — skipping executeCommand dispatch test"
        info "(Run 'omv-salt deploy run snapraid' to generate conf files)"
    fi
else
    info "No test array — skipping executeCommand test"
fi

# ---------------------------------------------------------------------------
# Array — delete
# ---------------------------------------------------------------------------
section "Array — deleteArray"

if [ -n "$ARRAY2_UUID" ]; then
    assert_rpc "deleteArray (array2)" "SnapRaid" "deleteArray" \
        "{\"uuid\":\"$ARRAY2_UUID\"}" >/dev/null
    assert_rpc_fails "getArray after delete (array2)" "SnapRaid" "getArray" \
        "{\"uuid\":\"$ARRAY2_UUID\"}"
    ARRAY2_UUID=""
fi

if [ -n "$ARRAY1_UUID" ]; then
    assert_rpc "deleteArray (array1)" "SnapRaid" "deleteArray" \
        "{\"uuid\":\"$ARRAY1_UUID\"}" >/dev/null
    assert_rpc_fails "getArray after delete (array1)" "SnapRaid" "getArray" \
        "{\"uuid\":\"$ARRAY1_UUID\"}"
    ARRAY1_UUID=""
fi

# ---------------------------------------------------------------------------
# Summary
# ---------------------------------------------------------------------------
section "Summary"
TOTAL=$((PASS + FAIL))
echo >&2
echo -e "  Results: ${GREEN}${PASS} passed${NC}, ${RED}${FAIL} failed${NC} (${TOTAL} total)" >&2
if [ ${#FAILED_TESTS[@]} -gt 0 ]; then
    echo -e "\n  ${RED}Failed tests:${NC}" >&2
    for t in "${FAILED_TESTS[@]}"; do
        echo -e "    ${RED}✗${NC} $t" >&2
    done
fi
echo >&2

[ $FAIL -eq 0 ] && exit 0 || exit 1
