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
// require("js/omv/workspace/panel/Panel.js")

Ext.define("OMV.module.admin.service.snapraid.Info", {
    extend : "Ext.panel.Panel",

    autoScroll    : true,
    initComponent : function() {
        var me = this;
        me.html = "<b>SnapRAID</b>" +
                  "<ul>" +
                    "<li>Version 6.3 by Andrea Mazzoleni, http://snapraid.sourceforge.net</li>" +
                    "<li><a href='http://snapraid.sourceforge.net/manual.html' target=_blank>" + _("SnapRAID user manual") + "</a></li>" +
                  "</ul>" +
                  _("Description of Buttons on Drives tab") +
                  "<ul>" +
                  "<li>" +
                    "<b>" + _("Sync") + "</b><br />" +
                    _("Updates the redundancy information. All the modified files in the disk array are read, and the redundancy data is recomputed.") +
                  "</li>" +
                  "<li>" +
                    "<b>" + _("Scrub") + "</b><br />" +
                    _("Scrubs the array, checking for silent errors.") +
                  "</li>" +
                  "<li>" +
                    "<b>" + _("Check") + "</b><br />" +
                    _("Checks all the files and the redundancy data. All the files are hashed and compared with the snapshot saved in the previous 'sync' command.") +
                  "</li>" +
                  "<li>" +
                    "<b>" + _("Diff") + "</b><br />" +
                    _("Lists all the files modified from the last 'sync' command that have to recompute their redundancy data.") +
                  "</li>" +
                  "<li>" +
                    "<b>" + _("Status") + "</b><br />" +
                    _("Prints a status report of the array.") +
                  "</li>" +
                  "<li>" +
                    "<b>" + _("Fix") + "</b><br />" +
                    _("Restore the last backup/snapshot.") +
                  "</li>" +
                  "<li>" +
                    "<b>" + _("Pool") + "</b><br />" +
                    _("Update the pool.") +
                  "</li>" +
                  "</ul>" +
                  _("The following excludes are written to the snapraid config file by default:") +
                  "<ul>" +
                    "<li>*.bak</li>" +
                    "<li>*.unrecoverable</li>" +
                    "<li>/tmp/</li>" +
                    "<li>lost+found/</li>" +
                    "<li>.content</li>" +
                    "<li>aquota.group</li>" +
                    "<li>aquota.user</li>" +
                  "</ul>";
        me.callParent(arguments);
    }
});

OMV.WorkspaceManager.registerPanel({
    id        : "info",
    path      : "/service/snapraid",
    text      : _("Information"),
    position  : 40,
    className : "OMV.module.admin.service.snapraid.Info"
});
