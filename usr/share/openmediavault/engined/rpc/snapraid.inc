<?php
/*
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @author    OpenMediaVault Plugin Developers <plugins@omv-extras.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
 * @copyright Copyright (c) 2013-2022 OpenMediaVault Plugin Developers
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

require_once("openmediavault/functions.inc");

class OMVRpcServiceSnapRaid extends \OMV\Rpc\ServiceAbstract
{
    private $getProgram = "/usr/bin/snapraid";
    private $getLogFile = "/var/log/snapraid.log";

    public function getName()
    {
        return 'SnapRaid';
    }

    public function initialize()
    {
        $this->registerMethod('getSettings');
        $this->registerMethod('setSettings');

        $this->registerMethod('getArrayList');
        $this->registerMethod('getArray');
        $this->registerMethod('setArray');
        $this->registerMethod('deleteArray');
        $this->registerMethod('enumerateArrays');

        $this->registerMethod('getDriveList');
        $this->registerMethod('getDrive');
        $this->registerMethod('setDrive');
        $this->registerMethod('deleteDrive');

        $this->registerMethod('getRuleList');
        $this->registerMethod('getRule');
        $this->registerMethod('setRule');
        $this->registerMethod('deleteRule');

        $this->registerMethod('getConfig');

        $this->registerMethod('executeCommand');
        $this->registerMethod('getVersion');
    }

    public function getSettings($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        // Get the configuration object.
        $db = \OMV\Config\Database::getInstance();
        $object = $db->get("conf.service.snapraid");
        // Remove useless properties from the object.
        $object->remove("drives");
        $object->remove("rules");
        return $object->getAssoc();
    }

    public function setSettings($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.snapraid.setsettings");
        // Get the existing configuration object.
        $db = \OMV\Config\Database::getInstance();
        $object = $db->get("conf.service.snapraid");
        $object->setAssoc($params);
        $db->set($object);
        // Remove useless properties from the object.
        $object->remove("arrays");
        $object->remove("drives");
        $object->remove("rules");
        // Return the configuration object.
        return $object->getAssoc();
    }

    public function getArrayList($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ['role' => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, 'rpc.common.getlist');
        // Get the configuration object.
        $db = \OMV\Config\Database::getInstance();
        $objects = $db->get('conf.service.snapraid.array');
        $drives = $db->get('conf.service.snapraid.drive');
        // Add additional share informations.
        $objectsAssoc = [];
        foreach ($objects as $objectk => &$objectv) {
            // get number of drives in array
            $driveCount = 0;
            foreach ($drives as $drivek => $drivev) {
                if ($drivev->get('arrayref') == $objectv->get('uuid')) {
                    $driveCount++;
                }
            }
            // Add the new property 'drives'.
            $objectv->add('drives', 'integer', $driveCount);
            $objectsAssoc[] = $objectv->getAssoc();
        }
        // Filter the result.
        return $this->applyFilter($objectsAssoc, $params['start'], $params['limit'],
            $params['sortfield'], $params['sortdir']);
    }

    public function getArray($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.common.objectuuid");
        // Get the configuration object.
        $db = \OMV\Config\Database::getInstance();
        return $db->getAssoc("conf.service.snapraid.array", $params['uuid']);
    }

    public function setArray($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.snapraid.setarray");
        // Prepare the configuration object.
        $object = new \OMV\Config\ConfigObject("conf.service.snapraid.array");
        // Remove spaces from name
        $params['name'] = str_replace(" ", "_", $params['name']);
        // Check that data drive is not also a parity drive
        if ( TRUE === $params['data'] && TRUE === $params['parity'] ) {
            throw new \OMV\Exception(
                gettext("A data drive cannot be a parity drive.")
            );
        }
        $db = \OMV\Config\Database::getInstance();
        $object->setAssoc($params);
        // Set the configuration object.
        $isNew = $object->isNew();
        $db = \OMV\Config\Database::getInstance();
        if (TRUE === $isNew) {
            // Check uniqueness - Shared folder
            $db->assertIsUnique($object, "name");
        }
        $db->set($object);
        // Return the configuration object.
        return $object->getAssoc();
    }

    public function deleteArray($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.common.objectuuid");
        // Delete the configuration object.
        $db = \OMV\Config\Database::getInstance();
        $object = $db->get("conf.service.snapraid.drive", $params['uuid']);
        $db->delete($object);
        // Return the deleted configuration object.
        return $object->getAssoc();
    }

    public function enumerateArrays($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ['role' => OMV_ROLE_ADMINISTRATOR]);
        // Get configuration data.
        $db = \OMV\Config\Database::getInstance();
        $objects = $db->get("conf.service.snapraid.array");
        $arrays = [];
        foreach ($objects as $objectk => &$objectv) {
            $arrayuuid = $objectv->get("uuid");
            $arrayname = $objectv->get("name");
            $arrays[] = [
                'uuid' => $arrayuuid,
                'name' => $arrayname
            ];
        }
        // Filter the result.
        return ($arrays);
    }

    public function getDriveList($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.common.getlist");
        // Get the configuration object.
        $db = \OMV\Config\Database::getInstance();
        $objects = $db->get("conf.service.snapraid.drive");
        // Add additional share informations.
        $objectsAssoc = [];
        foreach ($objects as $objectk => &$objectv) {
            // Add the new property 'sharedfoldername'.
            $objectv->add("arrayname", "string", gettext("n/a"));
            // Get the shared folder configuration object.
            $array = $db->get("conf.service.snapraid.array", $objectv->get("arrayref"));
            // Update the 'arrayname' property.
            $objectv->set("arrayname", $array->get("name"));
            $objectsAssoc[] = $objectv->getAssoc();
        }
        // Filter the result.
        return $this->applyFilter($objectsAssoc, $params['start'], $params['limit'],
            $params['sortfield'], $params['sortdir']);
    }

    public function getDrive($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.common.objectuuid");
        // Get the configuration object.
        $db = \OMV\Config\Database::getInstance();
        return $db->getAssoc("conf.service.snapraid.drive", $params['uuid']);
    }

    public function setDrive($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.snapraid.setdrive");
        // Prepare the configuration object.
        $object = new \OMV\Config\ConfigObject("conf.service.snapraid.drive");
        // Remove spaces from name
        $params['name'] = str_replace(" ", "_", $params['name']);
        // Check that data drive is not also a parity drive
        if ( TRUE === $params['data'] && TRUE === $params['parity'] ) {
            throw new \OMV\Exception(
                gettext("A data drive cannot be a parity drive.")
            );
        }
        $db = \OMV\Config\Database::getInstance();
        if ($db->exists("conf.system.filesystem.mountpoint", [
            "operator" => "stringEquals",
            "arg0" => "uuid",
            "arg1" => $params["mntentref"]
        ])) {
            $meObject = $db->get("conf.system.filesystem.mountpoint", $params["mntentref"]);
            // Get the filesystem backend.
            $fsbMngr = \OMV\System\Filesystem\Backend\Manager::getInstance();
            $fsbMngr->assertBackendExistsByType($meObject->get("type"));
            $fsb = $fsbMngr->getBackendByType($meObject->get("type"));
            // Add some mount point information:
            $params['path'] = $meObject->get("dir");
            // Get the filesystem implementation.
            $fs = $fsb->getImpl($meObject->get("fsname"));
            if (!is_null($fs) && $fs->exists()) {
                $params['label'] = $fs->getLabel();
            }
        }
        $object->setAssoc($params);
        // Set the configuration object.
        $isNew = $object->isNew();
        $db = \OMV\Config\Database::getInstance();
        if (TRUE === $isNew) {
            // Check uniqueness - Shared folder
            $db->assertIsUnique($object, "mntentref");
            $db->assertIsUnique($object, "name");
        }
        $db->set($object);
        // Return the configuration object.
        return $object->getAssoc();
    }

    public function deleteDrive($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.common.objectuuid");
        // Delete the configuration object.
        $db = \OMV\Config\Database::getInstance();
        $object = $db->get("conf.service.snapraid.drive", $params['uuid']);
        $db->delete($object);
        // Return the deleted configuration object.
        return $object->getAssoc();
    }

    public function getRuleList($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.common.getlist");
        // Get configuration data.
        $db = \OMV\Config\Database::getInstance();
        $objects = $db->getAssoc("conf.service.snapraid.rule");
        // Filter the result.
        return $this->applyFilter($objects, $params['start'], $params['limit'],
            $params['sortfield'], $params['sortdir']);
    }

    public function getRule($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.common.objectuuid");
        // Get the configuration object.
        $db = \OMV\Config\Database::getInstance();
        return $db->getAssoc("conf.service.snapraid.rule", $params['uuid']);
    }

    public function setRule($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.snapraid.setrule");
        // Prepare the configuration object.
        $object = new \OMV\Config\ConfigObject("conf.service.snapraid.rule");
        // Check for illegal rule
        if ( substr($params['rule1'], -2) == "/*" ) {
            throw new \OMV\Exception(gettext("Illegal rule - cannot end with /*"));
        }
        $db = \OMV\Config\Database::getInstance();
        $object->setAssoc($params);
        // Set the configuration object.
        $db->set($object);
        // Return the configuration object.
        return $object->getAssoc();
    }

    public function deleteRule($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.common.objectuuid");
        // Delete the configuration object.
        $db = \OMV\Config\Database::getInstance();
        $object = $db->get("conf.service.snapraid.rule", $params['uuid']);
        $db->delete($object);
        // Return the deleted configuration object.
        return $object->getAssoc();
    }

    public function getConfig($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);

        $snapraidConf = "";
        $snapraidDiff = "";
        $result = "";

        exec("export LANG=C; cat /etc/snapraid.conf", $result);
        $snapraidConf = implode("\n", $result) . "\n";

        $result = "";

        exec("export LANG=C; cat /etc/snapraid-diff.conf", $result);
        $snapraidDiff = implode("\n", $result) . "\n";

        return [
            "snapraidconf" => $snapraidConf,
            "snapraiddiff" => $snapraidDiff
        ];
    }

    public function executeCommand($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.snapraid.executecommand");
        // Get the configuration object.
        $db = \OMV\Config\Database::getInstance();
        $object = $db->get("conf.service.snapraid");

        // Create a background process.
        $bgStatusFilename = $this->createBgProcStatus();
        $pid = $this->fork();

        if ($pid > 0) { // Parent process.
            $this->initializeBgProcStatus($bgStatusFilename, $pid);
            return $bgStatusFilename;
        }
        $log = sprintf("[%s] omv-snapraid-gui: INFO: %s - Manually started\r\n",
            date("Y-m-d H:i:s"), strtoupper($params['command'])
        );
        file_put_contents($this->getLogFile, $log, FILE_APPEND);

        // Child process.
        try {
            $bgOutputFilename = $this->createBgProcOutput();
            $this->updateBgProcStatus($bgStatusFilename, "outputfilename", $bgOutputFilename);

            switch ($params['command']) {
                case "scrub":
                    $cmd = sprintf("export LANG=C; %s %s -p %s 2>&1", $this->getProgram,
                        $params['command'], $object->get('percentscrub')
                    );
                    break;
                case "silent":
                    // Fix known silent errors - snapraid -e fix && snapraid scrub -p0
                    $cmd = sprintf("export LANG=C; %s -e fix && %s scrub -p0 2>&1",
                        $this->getProgram, $this->getProgram
                    );
                    break;
                case "undelete":
                    // undelete files - snapraid fix -m [-f %FILENAME%]
                    if (empty($params['file'])) {
                        $cmd = sprintf("export LANG=C; %s fix -m 2>&1",
                            $this->getProgram
                        );
                    } else {
                        $cmd = sprintf("export LANG=C; %s fix -m -f %s 2>&1",
                            $this->getProgram, $params['file']
                        );
                    }
                    break;
                default:
                    $cmd = sprintf("export LANG=C; %s %s 2>&1",
                        $this->getProgram, $params['command']
                    );
            }
            $this->exec($cmd, $output, $bgOutputFilename);
            $this->finalizeBgProcStatus($bgStatusFilename, $output);
            $log = sprintf("[%s] omv-snapraid-gui: INFO: %s - Finished\r\n",
                date("Y-m-d H:i:s"), strtoupper($params['command'])
            );
            file_put_contents($this->getLogFile, $log, FILE_APPEND);
            exit(0);
        } catch(\Exception $e) {
            $this->finalizeBgProcStatus($bgStatusFilename, "", $e);
            $log = sprintf("[%s] omv-snapraid-gui: ERROR: %s - Error!\r\n",
                date("Y-m-d H:i:s"), strtoupper($params['command'])
            );
            file_put_contents($this->getLogFile, $log, FILE_APPEND);
            exit(1);
        }
    }

    function getVersion($params, $context)
    {
        $output = [];
        $cmd = "export LANG=C; dpkg-query -W -f='\${Version}' snapraid";
        exec($cmd, $output);
        $object['version'] = $output[0];
        return $object;
    }
}
