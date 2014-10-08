<?php
/*
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @author    OpenMediaVault Plugin Developers <plugins@omv-extras.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
 * @Copyright Copyright (c) 2010-2012 Ian Moore <imooreyahoo@gmail.com>
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
require_once "openmediavault/object.inc";
require_once "openmediavault/config.inc";
require_once "openmediavault/error.inc";
require_once "openmediavault/util.inc";
require_once "openmediavault/rpcservice.inc";
require_once "openmediavault/notify.inc";
require_once("openmediavault/system.inc");

class OMVRpcServiceSnapRaid extends OMVRpcServiceAbstract
{
    const DRIVE = 'drive';
    const RULE  = 'rule';

    public function getName()
    {
        return 'SnapRaid';
    }

    private function getPluginName()
    {
        return strtolower( $this->getName() );
    }

    private function getXPath()
    {
        return sprintf( "//services/%s", $this->getPluginName() );
    }

    private function getXPathDrives()
    {
        return sprintf( "%s/%ss", $this->getXPath(), self::DRIVE );
    }

    private function getXPathDrive()
    {
        return sprintf( "%s/%s", $this->getXPathDrives(), self::DRIVE );
    }

    private function getXPathRules()
    {
        return sprintf( "%s/%ss", $this->getXPath(), self::RULE );
    }

    private function getXPathRule()
    {
        return sprintf( "%s/%s", $this->getXPathRules(), self::RULE );
    }

    private function getXPathFstab()
    {
        return sprintf( "//system/fstab/mntent" );
    }

    private function getEventMessagePath()
    {
        return sprintf( "org.openmediavault.services.%s", $this->getPluginName() );
    }

    private function getEventMessagePathDrive()
    {
        return sprintf( "%s.%ss.%s", $this->getEventMessagePath(), self::DRIVE, self::DRIVE );
    }

    private function getEventMessagePathRule()
    {
        return sprintf( "%s.%ss.%s", $this->getEventMessagePath(), self::RULE, self::RULE  );
    }

    private function getProgram()
    {
        return "/usr/bin/snapraid";
    }

    private function getLogFile()
    {
        return "/var/log/snapraid.log";
    }

    public function initialize()
    {
        $this->registerMethod('getSettings');
        $this->registerMethod('setSettings');

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
    }

    public function getSettings($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext(
            $context,
            array( "role" => OMV_ROLE_ADMINISTRATOR )
        );

        // Get configuration object
        $object = $xmlConfig->get( $this->getXPath() );
        if (is_null($object))
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXPath()
            );
        }

        // Modify result data
        $object['nohidden'] = boolval($object['nohidden']);
        $object['autosave'] = intval($object['autosave']);
        $object['pool']     = boolval($object['pool']);

        $object['syslog']       = boolval($object['syslog']);
        $object['debug']        = boolval($object['debug']);
        $object['sendmail']     = boolval($object['sendmail']);
        $object['runscrub']     = boolval($object['runscrub']);
        $object['scrubfreq']    = intval($object['scrubfreq']);
        $object['delthreshold'] = intval($object['delthreshold']);

        unset($object['drives']);
        unset($object['rules']);

        return $object;
    }

    public function setSettings($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext(
            $context,
            array( "role" => OMV_ROLE_ADMINISTRATOR )
        );

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "blocksize"    : { "type" : "integer" },
                "autosave"     : { "type" : "integer" },
                "nohidden"     : { "type" : "boolean" },
                "pool"         : { "type" : "boolean" },
                "poolname"     : { "type" : "string", "optional" : true },
                "syslog"       : { "type" : "boolean" },
                "debug"        : { "type" : "boolean" },
                "sendmail"     : { "type" : "boolean" },
                "runscrub"     : { "type" : "boolean" },
                "scrubfreq"    : { "type" : "integer" },
                "delthreshold" : { "type" : "integer" }
            }
        }');

        // Get the existing configuration object.
        $oldObject = $xmlConfig->get( $this->getXPath() );
        if (is_null($oldObject))
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXPath()
            );
        }

        // Prepare configuration data
        $object = array(
            "blocksize"    => intval($params['blocksize']),
            "autosave"     => intval($params['autosave']),
            "nohidden"     => array_boolval($params, 'nohidden'),
            "pool"         => array_boolval($params, 'pool'),
            "poolname"     => str_replace(" ", "_", $params['poolname']),
            "syslog"       => array_boolval($params, 'syslog'),
            "debug"        => array_boolval($params, 'debug'),
            "sendmail"     => array_boolval($params, 'sendmail'),
            "runscrub"     => array_boolval($params, 'runscrub'),
            "scrubfreq"    => intval($params['scrubfreq']),
            "delthreshold" => intval($params['delthreshold']),
            "drives"       => isset($oldObject['drives']) ? $oldObject['drives'] : array(),
            "rules"        => isset($oldObject['rules']) ? $oldObject['rules'] : array()
        );

        // Set configuration object
        if (false === $xmlConfig->replace($this->getXPath(), $object))
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED,
                $this->getXPath()
            );
        }

        if ( $params['pool'] && ! file_exists("/media/pool") )
        {
            mkdir("/media/pool", 0777);
        }

        // Notify configuration changes
        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(
            OMV_NOTIFY_MODIFY,
            $this->getEventMessagePath(),
            $object
        );
    }

    public function getDriveList($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext(
            $context,
            array( "role" => OMV_ROLE_ADMINISTRATOR )
        );

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       :"object",
            "properties" : {
                "start"     : { "type" : "integer" },
                "limit"     : { '.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].' },
                "sortfield" : { '.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].' },
                "sortdir"   : { '.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].' }
              }
        }');

        // Get the configuration object.
        $objects = $xmlConfig->getList( $this->getXPathDrive() );
        if (is_null($objects))
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXPathDrive()
            );
        }

        // Get filesystem label
        $result = array();
        foreach($objects as $objectk => $objectv)
        {
            $xPath = sprintf("%s[uuid='%s']", $this->getXPathFstab(), $objectv['mntentref']);
            $fsname = $xmlConfig->get($xPath);
            $fs = new OMVFilesystem($fsname['fsname']);
            $object = array(
                "uuid"    => $objectv['uuid'],
                "name"    => $objectv['name'],
                "label"   => $fs->getLabel(),
                "content" => array_boolval($objectv, 'content'),
                "data"   => array_boolval($objectv, 'data'),
                "parity"  => array_boolval($objectv, 'parity')
            );
            $result[] = $object;
        }

        // Filter the result.
        return $this->applyFilter($result, $params['start'], $params['limit'],
            $params['sortfield'], $params['sortdir']);
    }

    public function getDrive($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array( "role" => OMV_ROLE_ADMINISTRATOR ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "uuid" : { '.$GLOBALS['OMV_JSONSCHEMA_UUID'].' }
            }
        }');

        // Get the configuration object.
        $xPath = sprintf("%s[uuid='%s']", $this->getXPathDrive(), $params['uuid']);
        $object = $xmlConfig->get($xPath);
        if (is_null($object))
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $xPath
            );
        }

        $object['content'] = boolval($object['content']);
        $object['data']    = boolval($object['data']);
        $object['parity']  = boolval($object['parity']);

        return $object;
    }

    public function setDrive($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array( "role" => OMV_ROLE_ADMINISTRATOR ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "uuid"      : { '.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].' },
                "mntentref" : { '.$GLOBALS['OMV_JSONSCHEMA_UUID'].' },
                "name"      : { "type" : "string" },
                "content"   : { "type" : "boolean" },
                "data"      : { "type" : "boolean" },
                "parity"    : { "type" : "boolean" }
            }
        }');

        $params['name'] = str_replace(" ", "_", $params['name']);

        $xPath = sprintf("%s[name='%s']", $this->getXPathDrive(), $params['name']);
        if ( $params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED'] && TRUE === $xmlConfig->exists($xPath) )
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                gettext("That name has already been used.")
            );
        }

        $xPath = sprintf("%s[mntentref='%s']", $this->getXPathDrive(), $params['mntentref']);
        if ( $params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED'] && TRUE === $xmlConfig->exists($xPath) )
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                gettext("This volume has already been added.")
            );
        }

        if ( TRUE === $params['data'] && TRUE === $params['parity'] )
        {
            throw new OMVException(
                OMVErrorMsg::E_RPC_INVALID_PARAMS,
                gettext("A data drive cannot be a parity drive.")
            );
        }

        $xPath = sprintf("%s[uuid='%s']", $this->getXPathFstab(), $params['mntentref']);
        $deviceDir = $xmlConfig->get($xPath);
        $fs = new OMVFilesystem($deviceDir['fsname']);

        $params['label'] = $fs->getLabel();
        $params['path']  = $deviceDir['dir'];

        // Prepare the configuration object.
        $object = array(
            "uuid"      => ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ? OMVUtil::uuid() : $params['uuid'],
            "mntentref" => $params['mntentref'],
            "content"   => array_boolval($params, "content"),
            "data"      => array_boolval($params, "data"),
            "parity"    => array_boolval($params, "parity"),
            "name"      => $params['name'],
            "label"     => $params['label'],
            "path"      => $params['path']
        );

        // Set the configuration object.
        $success = FALSE;
        if ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED'])
        {
            // Append object to configuration.
            $success = $xmlConfig->set($this->getXPathDrives(), array("drive" => $object));
        }
        else
        {
            // Update existing configuration object.
            $xPath = sprintf("%s[uuid='%s']", $this->getXPathDrive(), $params['uuid']);
            $success = $xmlConfig->replace($xPath, $object);
        }

        if (FALSE === $success)
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED
            );
        }

        // Notify configuration changes.
        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(
            ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ? OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
            $this->getEventMessagePathDrive(),
            $object);

        // Return the configuration object.
        return $object;
    }

    public function deleteDrive($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, array( "role" => OMV_ROLE_ADMINISTRATOR ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "uuid" : { '.$GLOBALS['OMV_JSONSCHEMA_UUID'].' }
            }
        }');

        // Delete the configuration object.
        $xPath = sprintf("%s[uuid='%s']", $this->getXPathDrive(), $params['uuid']);
        $object = $this->deleteConfigObjectByPath(
            $xPath,
            $this->getEventMessagePathDrive()
        );

        // Return the deleted configuration object.
        return $object;
    }

    public function getRuleList($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext(
            $context,
            array( "role" => OMV_ROLE_ADMINISTRATOR )
        );

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "start"     : { "type" : "integer" },
                "limit"     : { '.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].' },
                "sortfield" : { '.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].' },
                "sortdir"   : { '.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].' }
            }
        }');

        // Get configuration data.
        $objects = $xmlConfig->getList( $this->getXPathRule() );
        if (is_null($objects))
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXPathRule()
            );
        }

        // Filter the result.
        return $this->applyFilter($objects, $params['start'], $params['limit'],
            $params['sortfield'], $params['sortdir']);
    }

    public function getRule($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext(
            $context,
            array("role" => OMV_ROLE_ADMINISTRATOR)
        );

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "uuid" : { '.$GLOBALS['OMV_JSONSCHEMA_UUID'].' }
            }
        }');

          // Get the configuration object.
        $xPath = sprintf("%s[uuid='%s']", $this->getXPathRule(), $params['uuid']);
        $object = $xmlConfig->get($xPath);
        if (is_null($object))
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $xPath
            );
        }

        $object['rtype'] = intval($object['rtype']);

        return $object;
    }

    public function setRule($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext(
            $context,
            array( "role" => OMV_ROLE_ADMINISTRATOR )
        );

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "uuid"      : { '.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].' },
                "mntentref" : { '.$GLOBALS['OMV_JSONSCHEMA_UUID_NONE'].' },
                "rule"      : { "type" : "string" },
                "rtype"     : { "type" : "integer" }
            }
        }');

        if ( substr($params['rule'], -2) == "/*" )
        {
            throw new OMVException(
                OMVErrorMsg::E_MISC_FAILURE,
                "Illegal rule - cannot end with /*"
            );
        }

        $xPath = sprintf("//system/fstab/mntent[uuid='%s']", $params['mntentref']);
        $deviceDir = $xmlConfig->get($xPath);
        $params['rule-folder'] = $deviceDir['dir'];

        // Prepare configuration data.
        $object = array(
            "uuid"        => ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ? OMVUtil::uuid() : $params['uuid'],
            "mntentref"   => $params['mntentref'],
            "rule-folder" => $params['rule-folder'],
            "rule"        => $params['rule'],
            "rtype"       => intval($params['rtype'])
        );

        // Set the configuration data.
        $success = FALSE;
        if ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED'])
        {
            // Append object to configuration.
            $success = $xmlConfig->set($this->getXPathRules(), array("rule" => $object));
        }
        else
        {
            // Update existing configuration object.
            $xPath = sprintf("%s[uuid='%s']", $this->getXPathRule(), $params['uuid']);
            $success = $xmlConfig->replace($xPath, $object);
        }

        if (FALSE === $success)
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED
            );
        }

        // Notify configuration changes.
        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(
            ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ? OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
            $this->getEventMessagePathRule(),
            $object
        );

        // Return the configuration object.
        return $object;
    }

    public function deleteRule($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext(
            $context,
            array( "role" => OMV_ROLE_ADMINISTRATOR )
        );

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "uuid" : { '.$GLOBALS['OMV_JSONSCHEMA_UUID'].' }
            }
        }');

        // Delete the configuration object.
        $xPath = sprintf("%s[uuid='%s']", $this->getXPathRule(), $params['uuid']);
        return $this->deleteConfigObjectByPath(
            $xPath,
            $this->getEventMessagePathRule()
        );
    }

    public function getConfig($params, $context)
    {
        $output = "";
        $result = "";

        OMVUtil::exec("cat /etc/snapraid.conf", $result);
        $output = implode("\n", $result) . "\n";

        return $output;
    }

    public function executeCommand($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext(
            $context,
            array( "role" => OMV_ROLE_ADMINISTRATOR )
        );

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "command" : { "type" : "string" }
            }
        }');

        // Create a background process.
        $bgStatusFilename = $this->createBgProcStatus();
        $pid = $this->fork();

        if ($pid > 0)    // Parent process.
        {
            $this->initializeBgProcStatus($bgStatusFilename, $pid);
            return $bgStatusFilename;
        }

        $commands = array("sync", "scrub", "check", "diff", "status", "fix", "pool");
        if (!in_array($params['command'], $commands))
        {
            throw new OMVException(
                OMVErrorMsg::E_MISC_OPERATION_DENIED,
                sprinf("Unknown command - %s", $params['command'])
            );
        }

        $log = sprintf("[%s] %s - Manually started\r\n", date("Y-m-d H:i:s"), strtoupper($params['command']));
        file_put_contents($this->getLogFile(), $log, FILE_APPEND);

        // Child process.
        try {
            $bgOutputFilename = $this->createBgProcOutput();
            $this->updateBgProcStatus($bgStatusFilename, "outputfilename", $bgOutputFilename);

            $cmd = sprintf("%s %s 2>&1", $this->getProgram(), $params['command']);
            $this->exec($cmd, $output, $bgOutputFilename);
            $this->finalizeBgProcStatus($bgStatusFilename, $output);

            $log = sprintf("[%s] omv-snapraid-gui: INFO: %s - Finished\r\n", date("Y-m-d H:i:s"), strtoupper($params['command']));
            file_put_contents($this->getLogFile(), $log, FILE_APPEND);

            exit(0);
        } catch(Exception $e) {
            $this->finalizeBgProcStatus($bgStatusFilename, "", $e);

            $log = sprintf("[%s] omv-snapraid-gui: ERROR: %s - Error!\r\n", date("Y-m-d H:i:s"), strtoupper($params['command']));
            file_put_contents($this->getLogFile(), $log, FILE_APPEND);

            exit(1);
        }
    }
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceSnapRaid());
