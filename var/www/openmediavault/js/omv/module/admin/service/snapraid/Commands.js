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
            xtype  : 'container',
            layout : {
                type    : 'table',
                columns : 2,
                tdAttrs : { style: 'padding: 7px 5px;' }
            },
            items : [{
                xtype   : "button",
                name    : "sync",
                text    : _("Sync"),
                scope   : this,
                width   : 60,
                handler : Ext.Function.bind(me.onSyncButton, me, [ me ])
            },{
                xtype : "text",
                text  : _("Updates the redundancy information. All the modified files in the disk array are read, and the redundancy data is recomputed.")
            },{
                xtype   : "button",
                name    : "scrub",
                text    : _("Scrub"),
                scope   : this,
                width   : 60,
                handler : Ext.Function.bind(me.onScrubButton, me, [ me ])
            },{
                xtype : "text",
                text  : _("Scrubs the array, checking for silent errors.")
            },{
                xtype   : "button",
                name    : "check",
                text    : _("Check"),
                scope   : this,
                width   : 60,
                handler : Ext.Function.bind(me.onCheckButton, me, [ me ])
            },{
                xtype : "text",
                text  : _("Checks all the files and the redundancy data. All the files are hashed and compared with the snapshot saved in the previous 'sync' command.")
            },{
                xtype   : "button",
                name    : "diff",
                text    : _("Diff"),
                scope   : this,
                width   : 60,
                handler : Ext.Function.bind(me.onDiffButton, me, [ me ])
            },{
                xtype : "text",
                text  : _("Lists all the files modified from the last 'sync' command that have to recompute their redundancy data.")
            },{
                xtype   : "button",
                name    : "status",
                text    : _("Status"),
                scope   : this,
                width   : 60,
                handler : Ext.Function.bind(me.onStatusButton, me, [ me ])
            },{
                xtype   : "text",
                text    : _("Prints a status report of the array.")
            },{
                xtype   : "button",
                name    : "fix",
                text    : _("Fix"),
                scope   : this,
                width   : 60,
                handler : Ext.Function.bind(me.onFixButton, me, [ me ])
            },{
                xtype   : "text",
                text    : _("Restore the last backup/snapshot.")
            },{
                xtype   : "button",
                name    : "pool",
                text    : _("Pool"),
                scope   : this,
                width   : 60,
                handler : Ext.Function.bind(me.onPoolButton, me, [ me ])
            },{
                xtype   : "text",
                text    : _("Update the pool.")
            }]
        }];
    },

    onSyncButton: function() {
        var me = this;
        Ext.create("OMV.window.Execute", {
            title      : _("SnapRAID sync"),
            rpcService : "SnapRaid",
            rpcMethod  : "executeSync",
            listeners  : {
                scope     : me,
                exception : function(wnd, error) {
                    OMV.MessageBox.error(null, error);
                }
            }
        }).show();
    },

    onScrubButton: function() {
        var me = this;
        Ext.create("OMV.window.Execute", {
            title      : _("SnapRAID scrub"),
            rpcService : "SnapRaid",
            rpcMethod  : "executeScrub",
            listeners  : {
                scope     : me,
                exception : function(wnd, error) {
                    OMV.MessageBox.error(null, error);
                }
            }
        }).show();
    },

    onCheckButton: function() {
        var me = this;
        Ext.create("OMV.window.Execute", {
            title      : _("SnapRAID check"),
            rpcService : "SnapRaid",
            rpcMethod  : "executeCheck",
            listeners  : {
                scope     : me,
                exception : function(wnd, error) {
                    OMV.MessageBox.error(null, error);
                }
            }
        }).show();
    },

    onDiffButton: function() {
        var me = this;
        Ext.create("OMV.window.Execute", {
            title      : _("SnapRAID diff"),
            rpcService : "SnapRaid",
            rpcMethod  : "executeDiff",
            listeners  : {
                scope     : me,
                exception : function(wnd, error) {
                    OMV.MessageBox.error(null, error);
                }
            }
        }).show();
    },

    onStatusButton: function() {
        var me = this;
        Ext.create("OMV.window.Execute", {
            title      : _("SnapRAID status"),
            rpcService : "SnapRaid",
            rpcMethod  : "executeStatus",
            listeners  : {
                scope     : me,
                exception : function(wnd, error) {
                    OMV.MessageBox.error(null, error);
                }
            }
        }).show();
    },

    onFixButton: function() {
        var me = this;
        Ext.create("OMV.window.Execute", {
            title      : _("SnapRAID fix"),
            rpcService : "SnapRaid",
            rpcMethod  : "executeFix",
            listeners  : {
                scope     : me,
                exception : function(wnd, error) {
                    OMV.MessageBox.error(null, error);
                }
            }
        }).show();
    },

    onPoolButton: function() {
        var me = this;
        Ext.create("OMV.window.Execute", {
            title      : _("SnapRAID pool"),
            rpcService : "SnapRaid",
            rpcMethod  : "executePool",
            listeners  : {
                scope     : me,
                exception : function(wnd, error) {
                    OMV.MessageBox.error(null, error);
                }
            }
        }).show();
    }

});

OMV.WorkspaceManager.registerPanel({
    id        : "commands",
    path      : "/service/snapraid",
    text      : _("Commands"),
    position  : 60,
    className : "OMV.module.admin.service.snapraid.Commands"
});
