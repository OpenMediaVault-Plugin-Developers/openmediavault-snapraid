# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    OpenMediaVault Plugin Developers <plugins@omv-extras.org>
# @copyright Copyright (c) 2019-2026 openmediavault plugin developers
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

{% set config = salt['omv_conf.get']('conf.service.snapraid') %}
{% set confDir = '/etc/snapraid' %}
{% set confPrefix = 'omv-snapraid-' %}
{% set confDefault = '/etc/snapraid.conf' %}

configure_snapraid_envvar_dir:
  file.directory:
    - name: "{{ confDir }}"
    - user: root
    - group: root
    - mode: 755

{% set keep_confs = [] %}
{% for array in config.arrays.array %}
{% set drives = config.drives %}
{% set confFile = confDir ~ '/' ~ confPrefix ~ array.uuid ~ '.conf' %}
{% set confLink = confDir ~ '/' ~ array.name ~ '.conf' %}

configure_snapraid_{{ array.uuid }}:
  file.managed:
    - name: "{{ confFile }}"
    - source:
      - salt://{{ tpldir }}/files/etc-snapraid_conf.j2
    - template: jinja
    - context:
        config: {{ config | json }}
        array: {{ array | json }}
        drives: {{ drives | json }}
    - user: root
    - group: root
    - mode: 644

symlink_snapraid_{{ array.uuid }}:
  file.symlink:
    - name: {{ confLink }}
    - target: {{ confFile }}

{% do keep_confs.append(confPrefix ~ array.uuid ~ '.conf') %}
{% do keep_confs.append(array.name ~ '.conf') %}
{% endfor %}

purge_stale_snapraid_confs:
  file.tidied:
    - name: "{{ confDir }}"
    - matches:
      - ".*\\.conf$"
    - exclude:
{%- for f in keep_confs %}
      - "{{ f }}"
{%- endfor %}
    - rmdirs: False
    - rmlinks: True

{% if config.defaultarray | length == 36 %}

{% set confFile = confDir ~ '/' ~ confPrefix ~ config.defaultarray ~ '.conf' %}
symlink_snapraid_defaultarray:
  file.symlink:
    - name: {{ confDefault }}
    - target: {{ confFile }}
    - force: True

{% else %}

remove_snapraid_defaultarray:
  file.absent:
    - name: {{ confDefault }}

{% endif %}

configure_snapraid-diff:
  file.managed:
    - name: "/etc/snapraid-diff.conf"
    - source:
      - salt://{{ tpldir }}/files/etc-snapraid-diff_conf.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

# Manage persistent tmpfs mounts for drives marked as empty placeholders.
# Each empty drive needs its own device number so snapraid does not refuse
# to sync when multiple drives are marked empty at the same time.
{% set empty_uuids = [] %}
{% for drive in config.drives.drive %}
{%- if drive.emptydir is defined and drive.emptydir | to_bool %}
{% set uuid_esc = drive.uuid | replace('-', '\\x2d') %}
{% set unit = 'var-lib-snapraid-empty-' ~ uuid_esc ~ '.mount' %}
{% do empty_uuids.append(drive.uuid) %}

snapraid_empty_dir_{{ drive.uuid }}:
  file.directory:
    - name: /var/lib/snapraid/empty/{{ drive.uuid }}
    - makedirs: True

snapraid_empty_unit_{{ drive.uuid }}:
  file.managed:
    - name: /etc/systemd/system/{{ unit }}
    - user: root
    - group: root
    - mode: 644
    - contents: |
        [Unit]
        Description=SnapRAID empty placeholder for {{ drive.name }}
        DefaultDependencies=no
        Before=local-fs.target umount.target
        Conflicts=umount.target

        [Mount]
        What=snapraid-empty-{{ drive.uuid }}
        Where=/var/lib/snapraid/empty/{{ drive.uuid }}
        Type=tmpfs
        Options=size=1M,mode=755

        [Install]
        WantedBy=local-fs.target

snapraid_empty_mount_{{ drive.uuid }}:
  service.running:
    - name: {{ unit }}
    - enable: True
    - require:
      - file: snapraid_empty_dir_{{ drive.uuid }}
      - file: snapraid_empty_unit_{{ drive.uuid }}

{% endif %}
{% endfor %}

snapraid_cleanup_empty_mounts:
  cmd.run:
    - name: omv-snapraid-empty-mounts {{ empty_uuids | join(' ') }}
