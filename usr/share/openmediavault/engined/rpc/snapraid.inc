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
        $this->registerMethod('getArrayConfig');

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

    public function getArrayConfig($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ['role' => OMV_ROLE_ADMINISTRATOR]);
        $config = "";
        $confFile = sprintf('/etc/snapraid/omv-snapraid-%s.conf', $params['uuid']);
        if (file_exists($confFile)) {
            $config = file_get_contents($confFile);
        }
        return ["config" => $config];
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
        if ( TRUE === $params['data'] && (TRUE === $params['parity'] || TRUE === $params['paritysplit'] )) {
            throw new \OMV\Exception(
                gettext("A data drive cannot be a parity drive.")
            );
        }
        if ( TRUE === $params['data'] ) {
            $params['parity'] = false;
            $params['paritynum'] = 1;
            $params['paritysplit'] = false;
        } elseif ( TRUE === $params['parity'] ) {
            $params['data'] = false;
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

        $confFile = sprintf('/etc/snapraid/omv-snapraid-%s.conf', $params['uuid']);
        if (! file_exists($confFile)) {
            throw new \OMV\Exception(
                gettext("A config file for this array does not exist.")
            );
        }
        $conf = sprintf('--conf %s', $confFile);

        // Child process.
        try {
            $bgOutputFilename = $this->createBgProcOutput();
            $this->updateBgProcStatus($bgStatusFilename, "outputfilename", $bgOutputFilename);

            $cmdArgs = [];
            $cmdArgs[] = $this->getProgram;

            switch ($params['command']) {
                case "scrub":
                    $cmdArgs[] = $params['command'];
                    $cmdArgs[] = sprintf('-p %s', $object->get('percentScrub'));
                    break;
                case "silent":
                    // Fix known silent errors - snapraid -e fix && snapraid scrub -p0
                    $cmdArgs[] = '-e fix';
                    $cmdArgs[] = $conf;
                    $cmdArgs[] = '&&';
                    $cmdArgs[] = $this->getProgram;
                    $cmdArgs[] = 'scrub';
                    $cmdArgs[] = '-p0';
                    break;
                case "undelete":
                    // undelete files - snapraid fix -m [-f %FILENAME%]
                    $cmdArgs[] = 'fix';
                    if (empty($params['file'])) {
                        $cmdArgs[] = '-m';
                    } else {
                        $cmdArgs[] = '-m';
                        $cmdArgs[] = sprinft('-f %s', $params['file']);
                    }
                    break;
                default:
                    $cmdArgs[] = $params['command'];
            }
            $cmdArgs[] = $conf;
            // create command
            $cmd = new \OMV\System\Process($cmdArgs);
            $cmd->setRedirect2to1();
            $cmdLine = $cmd->getCommandLine();
            // execute command
            $this->exec($cmdLine, $output, $bgOutputFilename);
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
        $cmdArgs = [];
        $cmdArgs[] = 'dpkg-query';
        $cmdArgs[] = '-W';
        $cmdArgs[] = "-f='\${Version}'";
        $cmdArgs[] = 'snapraid';
        $cmd = new \OMV\System\Process($cmdArgs);
        $cmd->execute($output, $exitStatus);
        $object['version'] = $output[0];
        return $object;
    }
}
