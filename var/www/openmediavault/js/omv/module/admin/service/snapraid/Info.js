/**
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    OpenMediaVault Plugin Developers
 * @copyright Copyright (c) 2013 OpenMediaVault Plugin Developers
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
// require("js/omv/workspace/panel/Panel.js")

Ext.define("OMV.module.admin.service.snapraid.Info", {
	extend: "Ext.panel.Panel",

	initComponent: function() {
		var me = this;

		me.html = "Default username:  admin";
		me.html += "<br />";
		me.html += "Default password:  openmediavault";
		me.html += "<br /><br />";

		me.callParent(arguments);
	}
});

OMV.WorkspaceManager.registerPanel({
	id: "info",
	path: "/service/snapraid",
	text: _("Information"),
	position: 50,
	className: "OMV.module.admin.service.snapraid.Info"
});