#!/bin/sh

set -e

. /usr/share/openmediavault/scripts/helper-functions

syslog="/config/services/snapraid/syslog"

if omv_config_exists "${syslog}"; then
    omv_config_delete "${syslog}"
fi

exit 0
