#!/bin/sh
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @author    OpenMediaVault Plugin Developers <plugins@omv-extras.org>
# @copyright Copyright (c) 2009-2013 Volker Theile
# @copyright Copyright (c) 2013-2025 openmediavault plugin developers
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

set -e

. /usr/share/openmediavault/scripts/helper-functions

SERVICE_XPATH_NAME="snapraid"
SERVICE_XPATH="/config/services/${SERVICE_XPATH_NAME}"
SERVICE_XPATH_RULE="${SERVICE_XPATH}/rules/rule"

if ! omv_config_exists "${SERVICE_XPATH}"; then
    omv_config_add_node "/config/services" "${SERVICE_XPATH_NAME}"
    omv_config_add_key "${SERVICE_XPATH}" "blocksize" "256"
    omv_config_add_key "${SERVICE_XPATH}" "hashsize" "16"
    omv_config_add_key "${SERVICE_XPATH}" "autosave" "0"
    omv_config_add_key "${SERVICE_XPATH}" "nohidden" "0"
    omv_config_add_key "${SERVICE_XPATH}" "defaultarray" ""
    omv_config_add_key "${SERVICE_XPATH}" "debug" "0"
    omv_config_add_key "${SERVICE_XPATH}" "sendmail" "1"
    omv_config_add_key "${SERVICE_XPATH}" "runscrub" "1"
    omv_config_add_key "${SERVICE_XPATH}" "scrubfreq" "7"
    omv_config_add_key "${SERVICE_XPATH}" "updthreshold" "0"
    omv_config_add_key "${SERVICE_XPATH}" "delthreshold" "0"
    omv_config_add_key "${SERVICE_XPATH}" "percentscrub" "12"
    omv_config_add_key "${SERVICE_XPATH}" "scrubpercent" "100"
    omv_config_add_node "${SERVICE_XPATH}" "arrays" ""
    omv_config_add_node "${SERVICE_XPATH}" "drives" ""
    omv_config_add_node "${SERVICE_XPATH}" "rules" ""
fi

if ! omv_config_exists "//system/crontab/job[uuid='57e13a3c-5a94-11ec-8153-3f587eab8e1f']"; then
    object="<uuid>57e13a3c-5a94-11ec-8153-3f587eab8e1f</uuid>"
    object="${object}<enable>0</enable>"
    object="${object}<execution>exactly</execution>"
    object="${object}<sendemail>0</sendemail>"
    object="${object}<comment></comment>"
    object="${object}<type>userdefined</type>"
    object="${object}<minute>30</minute>"
    object="${object}<everynminute>0</everynminute>"
    object="${object}<hour>2</hour>"
    object="${object}<everynhour>0</everynhour>"
    object="${object}<month>*</month>"
    object="${object}<dayofmonth>*</dayofmonth>"
    object="${object}<everyndayofmonth>0</everyndayofmonth>"
    object="${object}<dayofweek>7</dayofweek>"
    object="${object}<username>root</username>"
    object="${object}<command>for conf in /etc/snapraid/omv-snapraid-*.conf; do /usr/sbin/omv-snapraid-diff \${conf}; done</command>"
    omv_config_add_node_data "//system/crontab" "job" "${object}"
fi

exit 0
