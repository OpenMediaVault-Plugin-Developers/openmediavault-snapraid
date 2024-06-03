#!/bin/sh

set -e

. /usr/share/openmediavault/scripts/helper-functions

SERVICE_XPATH="/config/services/snapraid"

if ! omv_config_exists "${SERVICE_XPATH}/apprise"; then
    omv_config_add_key "${SERVICE_XPATH}" "apprise" "0"
fi

if ! omv_config_exists "${SERVICE_XPATH}/apprisecmd"; then
    omv_config_add_key "${SERVICE_XPATH}" "apprisecmd" ""
fi

if ! omv_config_exists "${SERVICE_XPATH}/ntfy"; then
    omv_config_add_key "${SERVICE_XPATH}" "ntfy" "0"
fi

if ! omv_config_exists "${SERVICE_XPATH}/ntfycmd"; then
    omv_config_add_key "${SERVICE_XPATH}" "ntfycmd" ""
fi

if ! omv_config_exists "${SERVICE_XPATH}/sendsnaplog"; then
    omv_config_add_key "${SERVICE_XPATH}" "sendsnaplog" "0"
fi

if ! omv_config_exists "${SERVICE_XPATH}/sendnotifalways"; then
    omv_config_add_key "${SERVICE_XPATH}" "sendnotifalways" "0"
fi

exit 0
