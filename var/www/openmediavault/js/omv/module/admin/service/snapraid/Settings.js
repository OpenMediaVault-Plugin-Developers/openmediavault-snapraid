/**
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @author    OpenMediaVault Plugin Developers <plugins@omv-extras.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
 * @copyright Copyright (c) 2013-2021 OpenMediaVault Plugin Developers
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

    getButtonItems: function() {
        var items = this.callParent(arguments);

        items.push({
            id: this.getId() + '-scheduled-diff',
            xtype: 'button',
            text: _('Scheduled diff'),
            icon: 'images/wrench.png',
            iconCls: Ext.baseCSSPrefix + 'btn-icon-16x16',
            scope: this,
            handler: Ext.Function.bind(this.onScheduledDiffButton, this)
        });

        return items;
    },

    getFormItems : function() {
        return [{
            xtype    : "fieldset",
            title    : _("Settings"),
            defaults : {
                labelSeparator : ""
            },
            items : [{
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
                xtype         : "numberfield",
                name          : "hashsize",
                fieldLabel    : _("Hash Size"),
                minValue      : 0,
                maxValue      : 65535,
                allowDecimals : false,
                allowNegative : false,
                allowBlank    : false,
                value         : 16,
                plugins    : [{
                    ptype : "fieldinfo",
                    text  : _("Hash size in Bytes")
                }]
            },{
                xtype         : "numberfield",
                name          : "autosave",
                fieldLabel    : _("Auto Save"),
                minValue      : 0,
                maxValue      : 65535,
                allowDecimals : false,
                allowNegative : false,
                allowBlank    : false,
                value         : 0,
                plugins    : [{
                    ptype : "fieldinfo",
                    text  : _("Automatically save the state when syncing after the specified amount of GiB is processed. Default value is 0, meaning disabled.")
                }]
            },{
                xtype         : "numberfield",
                name          : "percentscrub",
                fieldLabel    : _("Scrub Percentage"),
                minValue      : 0,
                maxValue      : 100,
                allowDecimals : false,
                allowNegative : false,
                allowBlank    : false,
                value         : 12,
                plugins    : [{
                    ptype : "fieldinfo",
                    text  : _("Scrub percentage used for scrub button on Drives tab.")
                }]
            },{
                xtype      : "checkbox",
                name       : "nohidden",
                fieldLabel : _("No Hidden"),
                boxLabel   : _("Excludes hidden files and directories."),
                checked    : false
            }]
        },{
            xtype    : "fieldset",
            title    : _("Diff Script Settings"),
            defaults : {
                labelSeparator : ""
            },
            items : [{
                xtype      : "checkbox",
                name       : "syslog",
                fieldLabel : _("Syslog"),
                boxLabel   : _("Write logs to syslog"),
                checked    : true
            },{
                xtype      : "checkbox",
                name       : "debug",
                fieldLabel : _("Debug"),
                checked    : false
            },{
                xtype      : "checkbox",
                name       : "sendmail",
                fieldLabel : _("Send Mail"),
                checked    : true
            },{
                xtype      : "checkbox",
                name       : "runscrub",
                fieldLabel : _("Run Scrub"),
                boxLabel   : _("Set to true if you want to scrub after a successful sync"),
                checked    : true
            },{
                xtype      : "checkbox",
                name       : "prehash",
                fieldLabel : _("Pre-Hash"),
                boxLabel   : _("Read all data two times to ensure its integrity."),
                checked    : true
            },{
                xtype         : "numberfield",
                name          : "scrubfreq",
                fieldLabel    : _("Scrub Frequency"),
                minValue      : 0,
                maxValue      : 365,
                allowDecimals : false,
                allowNegative : false,
                allowBlank    : false,
                value         : 7,
                plugins    : [{
                    ptype : "fieldinfo",
                    text  : _("Units in days.")
                }]
            },{
                xtype         : "numberfield",
                name          : "scrubpercent",
                fieldLabel    : _("Scrub Percentage"),
                minValue      : 0,
                maxValue      : 100,
                allowDecimals : false,
                allowNegative : false,
                allowBlank    : false,
                value         : 100,
                plugins    : [{
                    ptype : "fieldinfo",
                    text  : _("Scrub percentage used for diff script.")
                }]
            },{
                xtype         : "numberfield",
                name          : "updthreshold",
                fieldLabel    : _("Update Threshold"),
                minValue      : 0,
                maxValue      : 65535,
                allowDecimals : false,
                allowNegative : false,
                allowBlank    : false,
                value         : 0,
                plugins    : [{
                    ptype : "fieldinfo",
                    text  : _("Define threshold of updated files above which not to start the sync process. default = 0 to disable this check and always sync")
                }]
            },{
                xtype         : "numberfield",
                name          : "delthreshold",
                fieldLabel    : _("Delete Threshold"),
                minValue      : 0,
                maxValue      : 65535,
                allowDecimals : false,
                allowNegative : false,
                allowBlank    : false,
                value         : 0,
                plugins    : [{
                    ptype : "fieldinfo",
                    text  : _("Define threshold of deleted files above which not to start the sync process. default = 0 to disable this check and always sync")
                }]
            }]
        }];
    },

    onScheduledDiffButton: function() {
        Ext.create('OMV.module.admin.service.snapraid.ScheduledDiff', {
            uuid: OMV.UUID_UNDEFINED
        }).show();
    }
});

OMV.WorkspaceManager.registerPanel({
    id        : "settings",
    path      : "/service/snapraid",
    text      : _("Settings"),
    position  : 10,
    className : "OMV.module.admin.service.snapraid.Settings"
});
