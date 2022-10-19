#!/bin/sh

set -e

. /usr/share/openmediavault/scripts/helper-functions

SERVICE_XPATH_NAME="snapraid"
SERVICE_XPATH="/config/services/${SERVICE_XPATH_NAME}"
SERVICE_XPATH_RULE="${SERVICE_XPATH}/rules/rule"

count=$(omv_config_get_count "${SERVICE_XPATH_RULE}");
index=1;
while [ ${index} -le ${count} ]; do
    if omv_config_exists "${SERVICE_XPATH_RULE}[position()=${index}]/rulefolder"; then
        omv_config_delete "${SERVICE_XPATH_RULE}[position()=${index}]/rulefolder"
    fi
    if omv_config_exists "${SERVICE_XPATH_RULE}[position()=${index}]/mntentref"; then
        omv_config_delete "${SERVICE_XPATH_RULE}[position()=${index}]/mntentref"
    fi
    index=$(( ${index} + 1 ))
done;

exit 0
