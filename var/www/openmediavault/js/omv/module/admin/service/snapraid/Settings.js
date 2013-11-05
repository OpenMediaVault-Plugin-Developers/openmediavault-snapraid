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
// require("js/omv/form/field/SharedFolderComboBox.js")

Ext.define("OMV.module.admin.service.snapraid.Settings", {
    extend : "OMV.workspace.form.Panel",
    uses   : [
        "OMV.data.Model",
        "OMV.data.Store"
    ],

    rpcService   : "SnapRaid",
    rpcGetMethod : "getSettings",
    rpcSetMethod : "setSettings",

    getFormItems : function() {
        var me = this;

        return [{
            xtype    : "fieldset",
            title    : _("Settings"),
            defaults : {
                labelSeparator : ""
            },
            items : [{
                xtype         : "combo",
                name          : "mntentref",
                fieldLabel    : _("Parity"),
                emptyText     : _("Select a volume ..."),
                allowBlank    : false,
                allowNone     : false,
                editable      : false,
                triggerAction : "all",
                displayField  : "description",
                valueField    : "uuid",
                store         : Ext.create("OMV.data.Store", {
                    autoLoad : true,
                    model    : OMV.data.Model.createImplicit({
                        idProperty : "uuid",
                        fields     : [
                            { name : "uuid", type : "string" },
                            { name : "devicefile", type : "string" },
                            { name : "description", type : "string" }
                        ]
                    }),
                    proxy : {
                        type : "rpc",
                        rpcData : {
                            service : "ShareMgmt",
                            method  : "getCandidates"
                        },
                        appendSortParams : false
                    },
                    sorters : [{
                        direction : "ASC",
                        property  : "devicefile"
                    }]
                }),
                plugins    : [{
                    ptype : "fieldinfo",
                    text  : _("Volume to use for Parity storage. It must NOT be on a data disk")
                }]
            },{
                xtype      : "textfield",
                name       : "parity",
                fieldLabel : _("Parity root"),
                allowNone  : true,
                readOnly   : true
            },{
                xtype         : "combo",
                name          : "qmntentref",
                fieldLabel    : _("Q-Parity"),
                emptyText     : _("Select a volume ..."),
                allowBlank    : true,
                allowNone     : true,
                editable      : false,
                triggerAction : "all",
                displayField  : "description",
                valueField    : "uuid",
                store         : Ext.create("OMV.data.Store", {
                    autoLoad : true,
                    model    : OMV.data.Model.createImplicit({
                        idProperty : "uuid",
                        fields     : [
                            { name : "uuid", type : "string" },
                            { name : "devicefile", type : "string" },
                            { name : "description", type : "string" }
                        ]
                    }),
                    proxy : {
                        type : "rpc",
                        rpcData : {
                            service : "ShareMgmt",
                            method  : "getCandidates"
                        },
                        appendSortParams : false
                    },
                    sorters : [{
                        direction : "ASC",
                        property  : "devicefile"
                    }]
                }),
                plugins    : [{
                    ptype : "fieldinfo",
                    text  : _("Volume to use for Q-Parity storage (equivalent of RAID 6). It must NOT be on a data disk or the parity disk. (OPTIONAL)")
                }]
            },{
                xtype      : "textfield",
                name       : "qparity",
                fieldLabel : _("QParity root"),
                allowNone  : true,
                readOnly   : true
            },{
                xtype         : "numberfield",
                name          : "blocksize",
                fieldLabel    : _("Block Size"),
                minValue      : 0,
                maxValue      : 65535,
                allowDecimals : false,
                allowNegative : false,
                allowBlank    : false,
                value         : 256,
                plugins    : [{
                    ptype : "fieldinfo",
                    text  : _("Block size in KiB")
                }]
            },{
                xtype      : "checkbox",
                name       : "nohidden",
                fieldLabel : _("No Hidden"),
                checked    : false,
                plugins    : [{
                    ptype : "fieldinfo",
                    text  : _("Excludes hidden files and directories")
                }]
            },{
                xtype   : "button",
                name    : "sync",
                text    : _("Sync"),
                scope   : this,
                handler : function() {
                    OMV.MessageBox.show({
                        title   : _("Sync?"),
                        msg     : _("Are you sure you want to sync?"),
                        buttons : Ext.Msg.YESNO,
                        fn      : function(answer) {
                            if (answer !== "yes")
                               return;

                            OMV.Rpc.request({
                                scope       : me,
                                relayErrors : false,
                                rpcData     : {
                                    service : "SnapRaid",
                                    method  : "doSync"
                                }
                            });
                        },
                        scope : me,
                        icon  : Ext.Msg.QUESTION
                    });
                }
            },{
                xtype   : "button",
                name    : "scrub",
                text    : _("Scrub"),
                scope   : this,
                handler : function() {
                    OMV.MessageBox.show({
                        title   : _("Scrub?"),
                        msg     : _("Are you sure you want to scrub?"),
                        buttons : Ext.Msg.YESNO,
                        fn      : function(answer) {
                            if (answer !== "yes")
                               return;

                            OMV.Rpc.request({
                                scope       : me,
                                relayErrors : false,
                                rpcData     : {
                                    service : "SnapRaid",
                                    method  : "doScrub"
                                }
                            });
                        },
                        scope : me,
                        icon  : Ext.Msg.QUESTION
                    });
                }
            }]
        }];
    }
});

OMV.WorkspaceManager.registerPanel({
    id        : "settings",
    path      : "/service/snapraid",
    text      : _("Settings"),
    position  : 10,
    className : "OMV.module.admin.service.snapraid.Settings"
});
