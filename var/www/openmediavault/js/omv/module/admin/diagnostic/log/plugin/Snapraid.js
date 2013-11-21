/**
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
// require("js/omv/PluginManager.js")
// require("js/omv/module/admin/diagnostic/log/plugin/Plugin.js")
// require("js/omv/util/Format.js")

Ext.define("OMV.module.admin.diagnostic.log.plugin.SnapRaid", {
    extend: "OMV.module.admin.diagnostic.log.plugin.Plugin",

    id: "snapraid",
    text: _("SnapRAID"),
    stateful: true,
    stateId: "11adf193-e76a-481e-b2c0-13cb308c97c0",
    columns: [{
        text: _("Date & Time"),
        sortable: true,
        dataIndex: "date",
        stateId: "date",
        flex: 1
    },{
        text: _("Component"),
        sortable: true,
        dataIndex: "component",
        stateId: "component",
        flex: 1
    },{
        text: _("Event"),
        sortable: true,
        dataIndex: "event",
        stateId: "event",
        flex: 1
    },{
        text: _("Message"),
        sortable: true,
        dataIndex: "message",
        stateId: "message",
        flex: 1
    }],
    rpcParams: {
        id: "snapraid"
    },
    rpcFields: [
        { name: "date", type: "string" },
        { name: "component", type: "string" },
        { name: "event", type: "string" },
        { name: "message", type: "string" },
    ]
});

OMV.PluginManager.register({
    ptype: "diagnostic",
    id: "log",
    className: "OMV.module.admin.diagnostic.log.plugin.SnapRaid"
});
