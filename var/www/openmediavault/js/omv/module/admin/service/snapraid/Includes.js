/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
 *
 * OpenMediaVault is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * OpenMediaVault is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenMediaVault. If not, see <http://www.gnu.org/licenses/>.
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

/**
 * @class OMV.module.admin.service.snapraid.Include
 * @derived OMV.workspace.window.Form
 */
Ext.define("OMV.module.admin.service.snapraid.Include", {
    extend: "OMV.workspace.window.Form",
    requires: [
        "OMV.workspace.window.plugin.ConfigObject"
    ],

    rpcService: "SnapRaid",
    rpcGetMethod: "getInclude",
    rpcSetMethod: "setInclude",
    plugins: [{
        ptype: "configobject"
    }],

    getFormItems: function() {
        return [{
            xtype: "textfield",
            name: "inclusion",
            fieldLabel: _("Include"),
            allowBlank: false
        }];
    }
});

/**
 * @class OMV.module.admin.service.snapraid.Includes
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.service.snapraid.Includes", {
    extend: "OMV.workspace.grid.Panel",
    requires: [
        "OMV.Rpc",
        "OMV.data.Store",
        "OMV.data.Model",
        "OMV.data.proxy.Rpc",
        "OMV.util.Format"
    ],
    uses: [
        "OMV.module.admin.service.snapraid.Include"
    ],

    hidePagingToolbar: false,
    stateful: true,
    stateId: "a982a76d-6804-4632-b31b-8b48c0ea6dde",
    columns: [{
        text: _("Include"),
        sortable: true,
        dataIndex: "inclusion",
        stateId: "inclusion"
    }],

    initComponent: function() {
        var me = this;
        Ext.apply(me, {
            store: Ext.create("OMV.data.Store", {
                autoLoad: true,
                model: OMV.data.Model.createImplicit({
                    idProperty: "uuid",
                    fields: [
                        { name: "uuid", type: "string" },
                        { name: "inclusion", type: "string" }
                    ]
                }),
                proxy: {
                    type: "rpc",
                    rpcData: {
                        service: "SnapRaid",
                        method: "getIncludeList"
                    }
                }
            })
        });
        me.callParent(arguments);
    },

    onAddButton: function() {
        var me = this;
        Ext.create("OMV.module.admin.service.snapraid.Include", {
            title: _("Add inclusion"),
            uuid: OMV.UUID_UNDEFINED,
            listeners: {
                scope: me,
                submit: function() {
                    this.doReload();
                }
            }
        }).show();
    },

    onEditButton: function() {
        var me = this;
        var record = me.getSelected();
        Ext.create("OMV.module.admin.service.snapraid.Include", {
            title: _("Edit inclusion"),
            uuid: record.get("uuid"),
            listeners: {
                scope: me,
                submit: function() {
                    this.doReload();
                }
            }
        }).show();
    },

    doDeletion: function(record) {
        var me = this;
        OMV.Rpc.request({
            scope: me,
            callback: me.onDeletion,
            rpcData: {
                service: "SnapRaid",
                method: "deleteInclude",
                params: {
                    uuid: record.get("uuid")
                }
            }
        });
    }

});

OMV.WorkspaceManager.registerPanel({
    id: "includes",
    path: "/service/snapraid",
    text: _("Includes"),
    position: 50,
    className: "OMV.module.admin.service.snapraid.Includes"
});
