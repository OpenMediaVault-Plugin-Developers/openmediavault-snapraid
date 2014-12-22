/**
 * @license     http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author      Volker Theile <volker.theile@openmediavault.org>
 * @author      OpenMediaVault Plugin Developers <plugins@omv-extras.org>
 * @copyright   Copyright (c) 2009-2013 Volker Theile
 * @copyright   Copyright (c) 2013-2014 OpenMediaVault Plugin Developers
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
// require("js/omv/workspace/form/Panel.js")

Ext.define("OMV.module.admin.service.snapraid.Config", {
    extend: "OMV.workspace.form.Panel",

    rpcService   : "SnapRaid",
    rpcGetMethod : "getConfig",

    hideOkButton    : true,
    hideResetButton : true,

    getButtonItems : function() {
        var me = this;
        var items = me.callParent(arguments);
        items.push({
            id       : me.getId() + "-refresh",
            xtype    : "button",
            text     : _("Refresh"),
            icon     : "images/refresh.png",
            iconCls  : Ext.baseCSSPrefix + "btn-icon-16x16",
            scope    : me,
            handler  : function() {
                me.doReload();
            }
        });
        return items;
    },

    getFormItems : function() {
		return [{
			xtype         : "fieldset",
			title         : _("Snapraid Config Files"),
			fieldDefaults : {
				labelSeparator : ""
			},
			items         : [{
				xtype      : "textarea",
				name       : "snapraidconf",
				fieldLabel : _("snapraid.conf"),
				readOnly   : true,
				height     : 250
			},{
				xtype      : "textarea",
				name       : "snapraiddiff",
				fieldLabel : _("snapraid-diff.conf"),
				readOnly   : true,
				height     : 250
			}]
		}];
	}
});

OMV.WorkspaceManager.registerPanel({
    id        : "config",
    path      : "/service/snapraid",
    text      : _("Config"),
    position  : 50,
    className : "OMV.module.admin.service.snapraid.Config"
});
