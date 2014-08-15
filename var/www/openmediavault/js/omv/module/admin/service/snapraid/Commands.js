/**
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @author    OpenMediaVault Plugin Developers <plugins@omv-extras.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
 * @copyright Copyright (c) 2013-2014 OpenMediaVault Plugin Developers
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
// require("js/omv/workspace/form/Panel.js")
// require("js/omv/workspace/window/Form.js")

Ext.define("OMV.module.admin.service.snapraid.Commands", {
    extend : "OMV.workspace.form.Panel",

    autoLoadData    : false,
    hideOkButton    : true,
    hideResetButton : true,
    mode            : "local",

    getFormItems    : function() {
        var me = this;
        return [{
            xtype  : "container",
            layout : {
                type    : "table",
                columns : 2,
                tdAttrs : { style: 'padding: 7px 5px;' }
            },
            items  : [{
                xtype   : "button",
                name    : "sync",
                text    : _("Sync"),
                scope   : this,
                width   : 60,
                handler : Ext.Function.bind(me.onCommandButton, me, [ "sync" ])
            },{
                xtype   : "text",
                text    : _("Updates the redundancy information. All the modified files in the disk array are read, and the redundancy data is recomputed.")
            },{
                xtype   : "button",
                name    : "scrub",
                text    : _("Scrub"),
                scope   : this,
                width   : 60,
                handler : Ext.Function.bind(me.onCommandButton, me, [ "scrub" ])
            },{
                xtype   : "text",
                text    : _("Scrubs the array, checking for silent errors.")
            },{
                xtype   : "button",
                name    : "check",
                text    : _("Check"),
                scope   : this,
                width   : 60,
                handler : Ext.Function.bind(me.onCommandButton, me, [ "check" ])
            },{
                xtype   : "text",
                text    : _("Checks all the files and the redundancy data. All the files are hashed and compared with the snapshot saved in the previous 'sync' command.")
            },{
                xtype   : "button",
                name    : "diff",
                text    : _("Diff"),
                scope   : this,
                width   : 60,
                handler : Ext.Function.bind(me.onCommandButton, me, [ "diff" ])
            },{
                xtype   : "text",
                text    : _("Lists all the files modified from the last 'sync' command that have to recompute their redundancy data.")
            },{
                xtype   : "button",
                name    : "status",
                text    : _("Status"),
                scope   : this,
                width   : 60,
                handler : Ext.Function.bind(me.onCommandButton, me, [ "status" ])
            },{
                xtype   : "text",
                text    : _("Prints a status report of the array.")
            },{
                xtype   : "button",
                name    : "fix",
                text    : _("Fix"),
                scope   : this,
                width   : 60,
                handler : Ext.Function.bind(me.onCommandButton, me, [ "fix" ])
            },{
                xtype   : "text",
                text    : _("Restore the last backup/snapshot.")
            },{
                xtype   : "button",
                name    : "pool",
                text    : _("Pool"),
                scope   : this,
                width   : 60,
                handler : Ext.Function.bind(me.onCommandButton, me, [ "pool" ])
            },{
                xtype   : "text",
                text    : _("Update the pool.")
            }]
        }];
    },

    onCommandButton: function(cmd) {
        var me = this;
        var wnd = Ext.create("OMV.window.Execute", {
            title      : "SnapRAID " + cmd,
            rpcService : "SnapRaid",
            rpcMethod  : "executeCommand",
            rpcParams  : {
                command : cmd
            },
            hideStartButton : true,
            hideStopButton  : true,
            listeners       : {
                scope     : me,
                finish    : function(wnd, response) {
                    wnd.appendValue(_("Done..."));
                    wnd.setButtonDisabled("close", false);
                },
                exception : function(wnd, error) {
                    OMV.MessageBox.error(null, error);
                    wnd.setButtonDisabled("close", false);
                }
            }
        });
        wnd.setButtonDisabled("close", true);
        wnd.show();
        wnd.start();
    }
});

OMV.WorkspaceManager.registerPanel({
    id        : "commands",
    path      : "/service/snapraid",
    text      : _("Commands"),
    position  : 40,
    className : "OMV.module.admin.service.snapraid.Commands"
});
