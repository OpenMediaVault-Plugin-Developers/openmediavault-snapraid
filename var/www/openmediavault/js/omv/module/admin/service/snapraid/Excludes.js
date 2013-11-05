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
 * @class OMV.module.admin.service.snapraid.Exclude
 * @derived OMV.workspace.window.Form
 */
Ext.define("OMV.module.admin.service.snapraid.Exclude", {
	extend: "OMV.workspace.window.Form",
	requires: [
		"OMV.workspace.window.plugin.ConfigObject"
	],

	rpcService: "SnapRaid",
	rpcGetMethod: "getExclude",
	rpcSetMethod: "setExclude",
	plugins: [{
		ptype: "configobject"
	}],

	getFormItems: function() {
		return [{
			xtype: "textfield",
			name: "exclude",
			fieldLabel: _("Exclude"),
			allowBlank: false
		}];
	}
});

/**
 * @class OMV.module.admin.service.snapraid.Excludes
 * @derived OMV.workspace.grid.Panel
 */
Ext.define("OMV.module.admin.service.snapraid.Excludes", {
	extend: "OMV.workspace.grid.Panel",
	requires: [
		"OMV.Rpc",
		"OMV.data.Store",
		"OMV.data.Model",
		"OMV.data.proxy.Rpc",
		"OMV.util.Format"
	],
	uses: [
		"OMV.module.admin.service.snapraid.Exclude"
	],
	
	hidePagingToolbar: false,
	stateful: true,
	stateId: "a982a76d-6804-4632-b31b-8b48c0ea6dde",
	columns: [{
		text: _("Exclude"),
		sortable: true,
		dataIndex: "exclude",
		stateId: "exclude"
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
						{ name: "exclude", type: "string" }
					]
				}),
				proxy: {
					type: "rpc",
					rpcData: {
						service: "SnapRaid",
						method: "getExcludeList"
					}
				}
			})
		});
		me.callParent(arguments);
	},

	onAddButton: function() {
		var me = this;
		Ext.create("OMV.module.admin.service.snapraid.Exclude", {
			title: _("Add exclusion"),
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
		Ext.create("OMV.module.admin.service.snapraid.Exclude", {
			title: _("Edit exclusion"),
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
				method: "deleteExclude",
				params: {
					uuid: record.get("uuid")
				}
			}
		});
	}

});

OMV.WorkspaceManager.registerPanel({
	id: "excludes",
	path: "/service/snapraid",
	text: _("Excludes"),
	position: 40,
	className: "OMV.module.admin.service.snapraid.Excludes"
});