#!/bin/sh

set -e

. /usr/share/openmediavault/scripts/helper-functions

uuid='57e13a3c-5a94-11ec-8153-3f587eab8e1f'
path="/config/system/crontab/job[uuid='${uuid}']/command"

cmd='for conf in /etc/snapraid/omv-snapraid-*.conf; do /usr/sbin/omv-snapraid-diff ${conf}; done'

if omv_config_exists "${path}"; then
  omv_config_update "${path}" "${cmd}"
  omv_module_set_dirty cron
fi

exit 0
