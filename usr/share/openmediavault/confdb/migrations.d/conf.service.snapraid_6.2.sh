#!/bin/sh

set -e

. /usr/share/openmediavault/scripts/helper-functions

array_uuid="114a88d2-53ed-11ed-8eee-b3f2573b9c38"

count=$(omv_config_get_count "/config/services/snapraid/arrays/array")
if [ "${count}" -eq 0 ]; then
    if ! omv_config_exists "/config/services/snapraid/arrays"; then
        omv_config_add_node "/config/services/snapraid" "arrays" ""
    fi
    object="<uuid>${array_uuid}</uuid>"
    object="${object}<name>array1</name>"
    omv_config_add_node_data "/config/services/snapraid/arrays" "array" "${object}"
fi

count=$(omv_config_get_count "/config/services/snapraid/drives/drive")
index=1
parity_num=1
while [ ${index} -le ${count} ]; do
    drive="/config/services/snapraid/drives/drive[position()=${index}]"
    if ! omv_config_exists "${drive}/arrayref"; then
        omv_config_add_key "${drive}" "arrayref" "${array_uuid}"
    fi
    if ! omv_config_exists "${drive}/paritynum"; then
        parity=$(omv_config_get "${drive}/parity")
        if [ "${parity}" -eq 0 ]; then
            omv_config_add_key "${drive}" "paritynum" "1"
        else
            omv_config_add_key "${drive}" "paritynum" "${parity_num}"
            parity_num=$((parity_num+1))
        fi
    fi
    if ! omv_config_exists "${drive}/paritysplit"; then
        omv_config_add_key "${drive}" "paritysplit" "0"
    fi
    if [ ${parity_num} -gt 6 ];
        parity_num=6
    fi
    index=$((index+1))
done

exit 0
