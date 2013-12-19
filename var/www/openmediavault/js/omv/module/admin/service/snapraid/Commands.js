/**
 * Copyright (C) 2010-2012 Ian Moore <imooreyahoo@gmail.com>
 * Copyright (C)      2013 OpenMediaVault Plugin Developers
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
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")
// require("js/omv/workspace/window/plugin/ConfigObject.js")

Ext.define("OMV.module.admin.service.snapraid.Commands", {
    extend : "OMV.workspace.form.Panel",
    uses   : [
        "OMV.data.Model",
        "OMV.data.Store"
    ],

    rpcService   : "SnapRaid",
    rpcGetMethod : "getCommands",

    getFormItems : function() {
        var me = this;

        return [{
                xtype : "fieldset",
                title : _("Commands"),
                frame : false,
                items : [{
                    xtype   : "button",
                    name    : "sync",
                    text    : _("Sync"),
                    scope   : this,
                    handler : Ext.Function.bind(me.onSyncButton, me, [ me ])
                },{
		      xtype   : "text",
		      text    : _("Updates the redundancy information. All the modified files in the disk array are read, and the redundancy data is recomputed."),
		      style   : {
			   paddingLeft : "16px"
		      }
		  },{
                    border : false,
                    html   : "<br />"
                },{
                    xtype   : "button",
                    name    : "scrub",
                    text    : _("Scrub"),
                    scope   : this,
                    handler : Ext.Function.bind(me.onScrubButton, me, [ me ])
                },{
		      xtype   : "text",
		      text    : _("Scrubs the array, checking for silent errors."),
		      style   : {
			   paddingLeft : "14px"
		      }
		  },{
                    border : false,
                    html   : "<br />"
                },{
                    xtype   : "button",
                    name    : "check",
                    text    : _("Check"),
                    scope   : this,
                    handler : Ext.Function.bind(me.onCheckButton, me, [ me ])
                },{
		      xtype   : "text",
		      text    : _("Checks all the files and the redundancy data. All the files are hashed and compared with the snapshot saved in the previous 'sync' command."),
		      style   : {
			   paddingLeft : "12px"
		      }
		  },{
                    border : false,
                    html   : "<br />"
                },{
                    xtype   : "button",
                    name    : "diff",
                    text    : _("Diff"),
                    scope   : this,
                    handler : Ext.Function.bind(me.onDiffButton, me, [ me ])
                },{
		      xtype   : "text",
		      text    : _("Lists all the files modified from the last 'sync' command that have to recompute their redundancy data."),
		      style   : {
			   paddingLeft : "24px"
		      }
		  },{
                    border : false,
                    html   : "<br />"
                },{
                    xtype   : "button",
                    name    : "status",
                    text    : _("Status"),
                    scope   : this,
                    handler : Ext.Function.bind(me.onStatusButton, me, [ me ])
                },{
		      xtype   : "text",
		      text    : _("Prints a status report of the array."),
		      style   : {
			   paddingLeft : "10px"
		      }
		  },{
                    border : false,
                    html   : "<br />"
                },{
                    xtype   : "button",
                    name    : "fix",
                    text    : _("Fix"),
                    scope   : this,
                    handler : Ext.Function.bind(me.onFixButton, me, [ me ])
                },{
		      xtype   : "text",
		      text    : _("   Restore the last backup/snapshot."),
		      style   : {
			   paddingLeft : "26px"
		      }
		  },{
                    border : false,
                    html   : "<br />"
                },{
                    xtype   : "button",
                    name    : "pool",
                    text    : _("Pool"),
                    scope   : this,
                    handler : Ext.Function.bind(me.onPoolButton, me, [ me ])
                },{
		      xtype   : "text",
		      text    : _("Update the pool."),
		      style   : {
			   paddingLeft : "20px"
		      }
		  },{
                    border : false,
                    html   : "<br />"
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
