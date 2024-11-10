#!/bin/sh

set -e

. /usr/share/openmediavault/scripts/helper-functions

xpath="/config/services/snapraid"

if ! omv_config_exists "${xpath}/defaultarray"; then
    omv_config_add_key "${xpath}" "defaultarray" ""
fi

exit 0
