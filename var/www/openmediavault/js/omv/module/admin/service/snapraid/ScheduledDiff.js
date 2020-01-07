/**
 * Copyright (C) 2015-2020 OpenMediaVault Plugin Developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// require("js/omv/WorkspaceManager.js")
// require("js/omv/module/admin/system/cron/Cron.js")

Ext.define('OMV.module.admin.service.snapraid.ScheduledDiff', {
    extend: 'OMV.module.admin.system.cron.Job',

    title: _('Create scheduled diff job'),
    height: 130,
    hideResetButton: true,

    initComponent: function() {
        this.callParent(arguments);

        var enable = this.findField('enable');
        var username = this.findField('username');
        var execution = this.findField('execution');
        var command = this.findField('command');
        var comment = this.findField('comment');

        enable.hide();
        username.hide();
        execution.hide();
        command.hide();
        comment.hide();

        enable.setValue(true);
        username.setValue('root');
        execution.setValue('daily');
        comment.setValue('SnapRAID - Scheduled diff.');
        command.setValue('/usr/sbin/omv-snapraid-diff');
    }
});
