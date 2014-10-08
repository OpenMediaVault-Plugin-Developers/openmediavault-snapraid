<?php
/*
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @author    OpenMediaVault Plugin Developers <plugins@omv-extras.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
 * @copyright Copyright (c) 2013-2014 OpenMediaVault Plugin Developers
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

class OMVModuleSnapRaid extends OMVModuleServiceAbstract implements OMVINotifyListener
{
    public function getName()
    {
        return "snapraid";
    }

    private function getEventMessagePath()
    {
        return sprintf("org.openmediavault.services.%s", $this->getName());
    }

    private function getEventMessagePathDrive()
    {
        return sprintf("%s.drives.drive", $this->getEventMessagePath());
    }

    private function getEventMessagePathRule()
    {
        return sprintf("%s.rules.rule", $this->getEventMessagePath());
    }

    public function applyConfig()
    {
        $cmd = "omv-mkconf snapraid 2>&1";

        if (0 !== $this->exec($cmd, $output))
        {
            throw new OMVException(
                OMVErrorMsg::E_EXEC_FAILED,
                $cmd,
                implode("\n", $output)
            );
        }
    }

    public function bindListeners(OMVNotifyDispatcher $dispatcher)
    {
        $dispatcher->addListener(
            OMV_NOTIFY_MODIFY,
            $this->getEventMessagePath(),
            array($this, "setDirty")
        );
        $dispatcher->addListener(
            OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
            $this->getEventMessagePathDrive(),
            array($this, "setDirty")
        );
        $dispatcher->addListener(
            OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
            $this->getEventMessagePathRule(),
            array($this, "setDirty")
        );
    }
}

// Register module
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new OMVModuleSnapRaid());

// Register new logfile type
OMVLogFile::registerType("snapraid", array(
    "filename" => "snapraid.log",
    "filepath" => "/var/log/snapraid.log",
    "regex"    => "/^\[((.*?)\s+(.*?))\]\s+(.*?):\s+(.*?):\s+(.*?)$/",
    "columns"  => array(
        "date"      => 1,
        "component" => 4,
        "level"     => 5,
        "message"   => 6
    )
));
