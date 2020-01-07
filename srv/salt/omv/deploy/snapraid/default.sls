# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    OpenMediaVault Plugin Developers <plugins@omv-extras.org>
# @copyright Copyright (c) 2019-2020 OpenMediaVault Plugin Developers
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

configure_snapraid:
  file.managed:
    - name: "/etc/snapraid.conf"
    - source:
      - salt://{{ slspath }}/files/etc-snapraid_conf.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

configure_snapraid-diff:
  file.managed:
    - name: "/etc/snapraid-diff.conf"
    - source:
      - salt://{{ slspath }}/files/etc-snapraid-diff_conf.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644
