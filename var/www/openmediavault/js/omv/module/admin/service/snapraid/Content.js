/**
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @author    OpenMediaVault Plugin Developers <plugins@omv-extras.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
 * @copyright Copyright (c) 2013-2014 OpenMediaVault Plugin Developers
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
// require("js/omv/Rpc.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")
// require("js/omv/form/field/SharedFolderComboBox.js")

/**
 * @class OMV.module.admin.service.snapraid.Content
 * @derived OMV.workspace.window.Form
 */
Ext.define("OMV.module.admin.service.snapraid.Content", {
    extend : "OMV.workspace.window.Form",
    uses   : [
        "OMV.form.field.SharedFolderComboBox",
        "OMV.workspace.window.plugin.ConfigObject"
    ],

    rpcService   : "SnapRaid",
    rpcGetMethod : "getContent",
    rpcSetMethod : "setContent",
    plugins      : [{
        ptype : "configobject"
    }],

    getFormItems: function () {
        return [{
            xtype         : "combo",
            name          : "mntentref",
            fieldLabel    : _("Volume"),
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
            })
        },{
            xtype      : "textfield",
            name       : "contentroot",
            fieldLabel : _("Content root"),
            allowNone  : true,
            readOnly   : true,
            hidden     : true
        }];
    }
});

/**
 * @class OMV.module.admin.service.snapraid.ContentList
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.service.snapraid.ContentList", {
    extend   : "OMV.workspace.grid.Panel",
    requires : [
        "OMV.Rpc",
        "OMV.data.Store",
        "OMV.data.Model",
        "OMV.data.proxy.Rpc"
    ],
    uses     : [
        "OMV.module.admin.service.snapraid.Content"
    ],

    hidePagingToolbar : false,
    stateful          : true,
    stateId           : "9879057b-b2c0-4c48-a4c1-8c9b4fb54d7b",
    columns           : [{
        text      : _("Label"),
        sortable  : true,
        dataIndex : "label",
        stateId   : "label"
    },{
        text      : _("Path"),
        sortable  : true,
        dataIndex : "contentroot",
        stateId   : "contentroot"
    }],

    initComponent : function () {
        var me = this;
        Ext.apply(me, {
            store : Ext.create("OMV.data.Store", {
                autoLoad : true,
                model    : OMV.data.Model.createImplicit({
                    idProperty  : "uuid",
                    fields      : [
                        { name : "uuid", type : "string" },
                        { name : "label", type : "string" },
                        { name : "contentroot", type : "string" }
                    ]
                }),
                proxy : {
                    type    : "rpc",
                    rpcData : {
                        service : "SnapRaid",
                        method  : "getContentList"
                    }
                }
            })
        });
        me.callParent(arguments);
    },

    onAddButton : function () {
        var me = this;
        Ext.create("OMV.module.admin.service.snapraid.Content", {
            title     : _("Add content path"),
            uuid      : OMV.UUID_UNDEFINED,
            listeners : {
                scope  : me,
                submit : function () {
                    this.doReload();
                }
            }
        }).show();
    },

    onEditButton : function() {
        var me = this;
        var record = me.getSelected();
        Ext.create("OMV.module.admin.service.snapraid.Content", {
            title     : _("Edit content path"),
            uuid      : record.get("uuid"),
            listeners : {
                scope  : me,
                submit : function() {
                    this.doReload();
                }
            }
        }).show();
    },

    doDeletion : function (record) {
        var me = this;
        OMV.Rpc.request({
            scope    : me,
            callback : me.onDeletion,
            rpcData  : {
                service : "SnapRaid",
                method  : "deleteContent",
                params  : {
                    uuid : record.get("uuid")
                }
            }
        });
    }
});

OMV.WorkspaceManager.registerPanel({
    id        : "contents",
    path      : "/service/snapraid",
    text      : _("Content"),
    position  : 20,
    className : "OMV.module.admin.service.snapraid.ContentList"
});
