#!/bin/sh

set -e

. /usr/share/openmediavault/scripts/helper-functions

xpath="/config/services/snapraid/drives/drive"

count=$(omv_config_get_count "${xpath}")
index=1
while [ ${index} -le ${count} ]; do
    drive="${xpath}[position()=${index}]"
    if ! omv_config_exists "${drive}/emptydir"; then
        omv_config_add_key "${drive}" "emptydir" "0"
    fi
    index=$((index+1))
done

exit 0
