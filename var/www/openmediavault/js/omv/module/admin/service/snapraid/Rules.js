/**
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @author    OpenMediaVault Plugin Developers <plugins@omv-extras.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
 * @copyright Copyright (c) 2013-2019 OpenMediaVault Plugin Developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
// require("js/omv/WorkspaceManager.js")
// require("js/omv/workspace/grid/Panel.js")
// require("js/omv/workspace/window/Form.js")
// require("js/omv/workspace/window/plugin/ConfigObject.js")
// require("js/omv/util/Format.js")
// require("js/omv/Rpc.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")
// require("js/omvextras/window/RootFolderBrowser.js")

Ext.define("OMV.module.admin.service.snapraid.Rule", {
    extend   : "OMV.workspace.window.Form",
    requires : [
        "OMV.workspace.window.plugin.ConfigObject",
        "OmvExtras.window.RootFolderBrowser"
    ],

    rpcService   : "SnapRaid",
    rpcGetMethod : "getRule",
    rpcSetMethod : "setRule",
    plugins      : [{
        ptype : "configobject"
    }],

    getFormItems : function() {
        var me = this;
        return [{
            xtype         : "combo",
            name          : "mntentref",
            fieldLabel    : _("Volume"),
            emptyText     : _("Select a volume ..."),
            editable      : false,
            triggerAction : "all",
            displayField  : "description",
            valueField    : "uuid",
            allowNone     : true,
            allowBlank    : true,
            store         : Ext.create("OMV.data.Store", {
                autoLoad : true,
                model    : OMV.data.Model.createImplicit({
                    idProperty  : "uuid",
                    fields      : [
                        { name : "uuid", type : "string" },
                        { name : "devicefile", type : "string" },
                        { name : "description", type : "string" }
                    ]
                }),
                proxy : {
                    type    : "rpc",
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
            })
        },{
            xtype          : "trigger",
            name           : "rule1",
            fieldLabel     : _("Rule"),
            allowBlank     : false,
            triggers       : {
                folder : {
                    cls     : Ext.baseCSSPrefix + "form-folder-trigger",
                    handler : "onTriggerClick"
                }
            },
            onTriggerClick : function() {
                Ext.create("OmvExtras.window.RootFolderBrowser", {
                    listeners : {
                        scope  : this,
                        select : function(wnd, node, path) {
                            // Set the selected path.
                            this.setValue(path);
                        }
                    }
                }).show();
            }
        },{
            xtype      : "combo",
            name       : "rtype",
            fieldLabel : _("Rule Type"),
            mode       : "local",
            store      : new Ext.data.SimpleStore({
                fields  : [ "value", "text" ],
                data    : [
                    [ 0, _("Exclusion") ],
                    [ 1, _("Inclusion") ]
                ]
            }),
            displayField  : "text",
            valueField    : "value",
            allowBlank    : false,
            editable      : false,
            triggerAction : "all",
            value         : 0,
            plugins    : [{
                ptype : "fieldinfo",
                text  : _("Specify whether the rule is an exclusion or inclusion.")
            }]
        }
        ];
    }
});

Ext.define("OMV.module.admin.service.snapraid.Rules", {
    extend   : "OMV.workspace.grid.Panel",
    requires : [
        "OMV.Rpc",
        "OMV.data.Store",
        "OMV.data.Model",
        "OMV.data.proxy.Rpc",
        "OMV.util.Format"
    ],
    uses     : [
        "OMV.module.admin.service.snapraid.Rule"
    ],

    hidePagingToolbar : false,
    stateful          : true,
    stateId           : "a982a76d-6804-4632-b31b-8b48c0ea6dde",
    columns           : [{
        xtype     : "textcolumn",
        text      : _("Rule"),
        sortable  : true,
        dataIndex : "rule1",
        stateId   : "rule1"
    },{
        xtype     : "textcolumn",
        text      : _("Rule Type"),
        sortable  : true,
        dataIndex : "rtype",
        stateId   : "rtype",
        renderer  : function (value) {
            if (value == 1)
                return _("Inclusion");
            else
                return _("Exclusion");
        }
    }],

    initComponent : function() {
        var me = this;
        Ext.apply(me, {
            store : Ext.create("OMV.data.Store", {
                autoLoad : true,
                model    : OMV.data.Model.createImplicit({
                    idProperty  : "uuid",
                    fields      : [
                        { name : "uuid", type : "string" },
                        { name : "rule1", type : "string" },
                        { name : "rtype", type : "integer" }
                    ]
                }),
                proxy : {
                    type    : "rpc",
                    rpcData : {
                        service : "SnapRaid",
                        method  : "getRuleList"
                    }
                }
            })
        });
        me.callParent(arguments);
    },

    onAddButton : function() {
        var me = this;
        Ext.create("OMV.module.admin.service.snapraid.Rule", {
            title     : _("Add rule"),
            uuid      : OMV.UUID_UNDEFINED,
            listeners : {
                scope  : me,
                submit : function() {
                    this.doReload();
                }
            }
        }).show();
    },

    onEditButton : function() {
        var me = this;
        var record = me.getSelected();
        Ext.create("OMV.module.admin.service.snapraid.Rule", {
            title     : _("Edit rule"),
            uuid      : record.get("uuid"),
            listeners : {
                scope  : me,
                submit : function() {
                    this.doReload();
                }
            }
        }).show();
    },

    doDeletion : function(record) {
        var me = this;
        OMV.Rpc.request({
            scope    : me,
            callback : me.onDeletion,
            rpcData  : {
                service : "SnapRaid",
                method  : "deleteRule",
                params  : {
                    uuid : record.get("uuid")
                }
            }
        });
    }

});

OMV.WorkspaceManager.registerPanel({
    id        : "excludes",
    path      : "/service/snapraid",
    text      : _("Rules"),
    position  : 30,
    className : "OMV.module.admin.service.snapraid.Rules"
});
