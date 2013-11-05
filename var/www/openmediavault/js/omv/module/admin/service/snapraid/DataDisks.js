/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Aaron Murray <aaron@omv-extras.org>
 * @copyright Copyright (c) 2013 Aaron Murray
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
// require("js/omv/Rpc.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")
// require("js/omv/form/field/SharedFolderComboBox.js")

/**
 * @class OMV.module.admin.service.snapraid.Data
 * @derived OMV.workspace.window.Form
 */
Ext.define("OMV.module.admin.service.snapraid.Data", {
	extend: "OMV.workspace.window.Form",
	uses: [
		"OMV.form.field.SharedFolderComboBox",
		"OMV.workspace.window.plugin.ConfigObject"
	],

	rpcService: "SnapRaid",
	rpcGetMethod: "getData",
	rpcSetMethod: "setData",
	plugins: [{
		ptype: "configobject"
	}],

	/**
	 * The class constructor.
	 * @fn constructor
	 * @param uuid The UUID of the database/configuration object. Required.
	 */

	getFormItems: function () {
		return [{
            xtype         : "combo",
            name          : "mntentref",
            fieldLabel    : _("Data"),
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
                name       : "dataroot",
                fieldLabel : _("Data root"),
                allowNone  : true,
                readOnly   : true
        }];
	}
});

/**
 * @class OMV.module.admin.service.snapraid.DataList
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.service.snapraid.DataList", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.Rpc",
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc"
	],
	uses: [
		"OMV.module.admin.service.snapraid.Data"
	],

	hidePagingToolbar: false,
    hideEditButton: true,
	stateful: true,
	stateId: "9879057b-b2c0-4c48-a4c1-8c9b4fb54d7b",
	columns: [{
		text: _("Volume"),
		sortable: true,
		dataIndex: "dataroot",
		stateId: "dataroot"
	}],

	initComponent: function () {
		var me = this;
		Ext.apply(me, {
			store: Ext.create("OMV.data.Store", {
				autoLoad: true,
				model: OMV.data.Model.createImplicit({
					idProperty: "uuid",
					fields: [
						{ name: "uuid", type: "string" },
						{ name: "dataroot", type: "string" }
					]
				}),
				proxy: {
					type: "rpc",
					rpcData: {
						service: "SnapRaid",
						method: "getDataList"
					}
				}
			})
		});
		me.callParent(arguments);
	},

	onAddButton: function () {
		var me = this;
		Ext.create("OMV.module.admin.service.snapraid.Data", {
			title: _("Add data volume"),
			uuid: OMV.UUID_UNDEFINED,
			listeners: {
				scope: me,
				submit: function () {
					this.doReload();
				}
			}
		}).show();
	},

	doDeletion: function (record) {
		var me = this;
		OMV.Rpc.request({
			scope: me,
			callback: me.onDeletion,
			rpcData: {
				service: "SnapRaid",
				method: "deleteData",
				params: {
					uuid: record.get("uuid")
				}
			}
		});
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "datalist",
	path: "/service/snapraid",
	text: _("Data"),
	position: 30,
	className: "OMV.module.admin.service.snapraid.DataList"
});
