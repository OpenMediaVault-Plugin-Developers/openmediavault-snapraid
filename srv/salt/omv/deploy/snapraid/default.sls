# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    OpenMediaVault Plugin Developers <plugins@omv-extras.org>
# @copyright Copyright (c) 2019-2024 openmediavault plugin developers
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

remove_snapraid_conf_files:
  cmd.run:
    - name: "rm -fv {{ confDir }}/*.conf"

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

{% endfor %}

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
