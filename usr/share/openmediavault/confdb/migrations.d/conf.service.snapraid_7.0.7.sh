#!/bin/sh

set -e

. /usr/share/openmediavault/scripts/helper-functions

count=$(omv_config_get_count "/config/services/snapraid/drives/drive")
index=1
while [ ${index} -le ${count} ]; do
    drive="/config/services/snapraid/drives/drive[position()=${index}]"
    parity=$(omv_config_get "${drive}/parity")
    echo ${drive}
    echo ${parity}
    if [ ${parity} -eq 0 ]; then
        omv_config_update "${drive}" "paritynum" "1"
    fi
    index=$((index+1))
done

exit 0
