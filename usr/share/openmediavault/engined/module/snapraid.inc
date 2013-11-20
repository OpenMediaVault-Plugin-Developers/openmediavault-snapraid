<?php
/**
 * Copyright (C)      2013 OpenMediaVault Plugin Developers
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

require_once("openmediavault/module.inc");
require_once("openmediavault/config.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/initscript.inc");
require_once("openmediavault/log.inc");

class OMVModuleSnapRaid extends OMVModuleServiceAbstract implements OMVINotifyListener {

    public function getName() {
        return "snapraid";
    }

    public function applyConfig() {
        global $xmlConfig;

        $config = $xmlConfig->get("//services/snapraid");

        if (is_null($config))
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, "//services/snapraid");

        $cmd = "sudo omv-mkconf snapraid 2>&1";

        if (0 !== $this->exec($cmd, $output))
            throw new OMVException(OMVErrorMsg::E_EXEC_FAILED, $cmd, implode("\n", $output));
    }

    public function bindListeners(OMVNotifyDispatcher $dispatcher) {
        $dispatcher->addListener(
            OMV_NOTIFY_MODIFY,
            "org.openmediavault.services.snapraid",
            array($this, "setDirty"));
        $dispatcher->addListener(
            OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
            "org.openmediavault.services.snapraid.contents.content",
            array($this, "setDirty"));
        $dispatcher->addListener(
            OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
            "org.openmediavault.services.snapraid.datas.data",
            array($this, "setDirty"));
        $dispatcher->addListener(
            OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
            "org.openmediavault.services.snapraid.excludes.exclude",
            array($this, "setDirty"));
        $dispatcher->addListener(
            OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
            "org.openmediavault.services.snapraid.includes.include",
            array($this, "setDirty"));
    }
}

// Register module
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new OMVModuleSnapRaid());

// Register new logfile type
OMVLogFile::registerType("snapraid", array(
    "filename" => "snapraid.log",
    "filepath" => "/var/log/snapraid.log",
    "regex"    => "/(^(.*?)\s+(.*?)\s+(.*?))\s+(.*?)\s+(.*?):\s+(.*?)\[(.*?)\]:\s+(.*?):\s+\'(.*?)\'$/",
    "columns"  => array(
        "date"      => array(
            "index"  => 1,
            "func"   => create_function('$v', 'return strpdate($v,"Y/n/j G:i:s");')
        ),
        "component" => 7,
        "event"     => 8,
        "message"   => 9
    )
));

?>