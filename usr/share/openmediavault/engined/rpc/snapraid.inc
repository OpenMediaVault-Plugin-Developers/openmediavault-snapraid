<?php

/**
 * Copyright (C) 2010-2012 Ian Moore <imooreyahoo@gmail.com>
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

require_once "openmediavault/object.inc";
require_once "openmediavault/config.inc";
require_once "openmediavault/error.inc";
require_once "openmediavault/util.inc";
require_once "openmediavault/rpcservice.inc";
require_once "openmediavault/notify.inc";

class OMVRpcServiceSnapRaid extends OMVRpcServiceAbstract {

    private function getXPath() {
        return "//services/snapraid";
    }

    public function getName() {
        return 'SnapRaid';
    }

    public function initialize() {
        $this->registerMethod('getSettings');
        $this->registerMethod('setSettings');

        $this->registerMethod('getContentList');
        $this->registerMethod('getContent');
        $this->registerMethod('setContent');
        $this->registerMethod('deleteContent');

        $this->registerMethod('getDataList');
        $this->registerMethod('getData');
        $this->registerMethod('setData');
        $this->registerMethod('deleteData');

        $this->registerMethod('getExcludeList');
        $this->registerMethod('getExclude');
        $this->registerMethod('setExclude');
        $this->registerMethod('deleteExclude');

        $this->registerMethod('executeSync');
        $this->registerMethod('executeScrub');
        $this->registerMethod('executeCheck');
    }

    public function getSettings($params, $context) {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array( "role" => OMV_ROLE_ADMINISTRATOR ));

        // Get configuration object
        $object = $xmlConfig->get($this->getXPath());
        if (is_null($object))
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $this->getXPath());

        // Modify result data
        $object['useqparity']   = boolval($object['useqparity']);
        $object['nohidden']     = boolval($object['nohidden']);

        unset($object['contents']);
        unset($object['datas']);
        unset($object['excludes']);

        return $object;
    }

    public function setSettings($params, $context) {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array( "role" => OMV_ROLE_ADMINISTRATOR ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "parity"        : { "type" : "string" },
                "mntentref"     : { "type" : "string" },
                "useqparity"    : { "type" : "boolean" },
                "qparity"       : { "type" : "string", "optional" : true },
                "qmntentref"    : { "type" : "string", "optional" : true },
                "blocksize"     : { "type" : "integer" },
                "nohidden"      : { "type" : "boolean" }
            }
        }');

        $xpath = sprintf("//system/fstab/mntent[uuid='%s']", $params['mntentref']);
        $deviceDir = $xmlConfig->get($xpath);
        $deviceDir = $deviceDir['dir'];

        $parityRoot = "{$deviceDir}/parity.file";
        $params['parity'] = $parityRoot;

        if ( ($params['qmntentref'] != "") && $params['useqparity'] === true ) {
            if ( $params['qmntentref'] == $params['mntentref'] ) {
                throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                    gettext("Parity and Q-Parity cannot be the same."));
            }

            $xpath = sprintf("//system/fstab/mntent[uuid='%s']", $params['qmntentref']);
            $deviceDir = $xmlConfig->get($xpath);
            $deviceDir = $deviceDir['dir'];

            $qparityRoot = "{$deviceDir}/qparity.file";
            $params['qparity'] = $qparityRoot;
        } else {
            $params['qparity'] = "";
            $params['qmntentref'] = "";
        }

        // Get the existing configuration object.
        $xpath = "//services/snapraid";
        $oldObject = $xmlConfig->get($xpath);
        if(is_null($oldObject)) {
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $xpath);
        }

        // Prepare configuration data
        $object = array(
            "mntentref"     => $params['mntentref'],
            "parity"        => $params['parity'],
            "useqparity"    => array_boolval($params, 'useqparity'),
            "qmntentref"    => $params['qmntentref'],
            "qparity"       => $params['qparity'],
            "blocksize"     => intval($params['blocksize']),
            "nohidden"      => array_boolval($params, 'nohidden'),
            "contents"      => isset($oldObject['contents']) ? $oldObject['contents'] : array(),
            "datas"         => isset($oldObject['datas']) ? $oldObject['datas'] : array(),
            "excludes"      => isset($oldObject['excludes']) ? $oldObject['excludes'] : array()
        );

        // Set configuration object
        if (false === $xmlConfig->replace($this->getXPath(), $object))
            throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED, $this->getXPath());

        // Notify configuration changes
        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(OMV_NOTIFY_MODIFY, "org.openmediavault.services.snapraid", $object);
    }

    public function getContentList($params, $context) {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array( "role" => OMV_ROLE_ADMINISTRATOR ));

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
        $xpath = "//services/snapraid/contents/content";
        $objects = $xmlConfig->getList($xpath);
        if(is_null($objects)) {
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $xpath);
        }

        // Filter the result.
        return $this->applyFilter($objects, $params['start'],
            $params['limit'], $params['sortfield'], $params['sortdir']);
    }

    public function getContent($params, $context) {
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
        $xpath = sprintf("//services/snapraid/contents/content[uuid='%s']", $params['uuid']);
        $object = $xmlConfig->get($xpath);
        if(is_null($object)) {
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $xpath);
        }

        return $object;
    }

    public function setContent($params, $context) {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array( "role" => OMV_ROLE_ADMINISTRATOR ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "uuid"          : { '.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].' },
                "mntentref"     : { '.$GLOBALS['OMV_JSONSCHEMA_UUID'].' },
                "contentroot"   : { "type" : "string" }
            }
        }');

        $parity = $xmlConfig->get("//services/snapraid/mntentref");
        $useqparity = $xmlConfig->get("//services/snapraid/useqparity");
        $qparity = $xmlConfig->get("//services/snapraid/qmntentref");

        if ( $params['mntentref'] == $parity ) {
            throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                gettext("Parity is using this volume. Content cannot use the same volume as Parity or Q-Parity."));
        }

        if ( $useqparity != 0 && $params['mntentref'] == $qparity ) {
            throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                gettext("Q-Parity is using this volume. Content cannot use the same volume as Parity or Q-Parity."));
        }

        $xpath = sprintf("//system/fstab/mntent[uuid='%s']", $params['mntentref']);
        $deviceDir = $xmlConfig->get($xpath);
        $deviceDir = $deviceDir['dir'];

        $contentRoot = "{$deviceDir}/.content";
        $params['contentroot'] = $contentRoot;

        // Prepare the configuration object.
        $object = array(
            "uuid"          => ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ? OMVUtil::uuid() : $params['uuid'],
            "mntentref"     => $params['mntentref'],
            "contentroot"   => $params['contentroot']
        );

        // Set the configuration object.
        $success = FALSE;

        // Append the configuration object.
        $success = $xmlConfig->set("//services/snapraid/contents", array("content" => $object));

        if(FALSE === $success) {
            throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
        }

        // Notify configuration changes.
        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
            OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
            "org.openmediavault.services.snapraid.contents.content", $object);

        // Return the configuration object.
        return $object;
    }

    public function deleteContent($params, $context) {
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
        $xpath = sprintf("//services/snapraid/contents/content[uuid='%s']", $params['uuid']);
        $object = $this->deleteConfigObjectByPath($xpath,
            "org.openmediavault.services.snapraid.contents.content");

        // Return the deleted configuration object.
        return $object;
    }

    public function getDataList($params, $context) {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array( "role" => OMV_ROLE_ADMINISTRATOR ));

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
        $xpath = "//services/snapraid/datas/data";
        $objects = $xmlConfig->getList($xpath);
        if(is_null($objects)) {
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $xpath);
        }

        // Filter the result.
        return $this->applyFilter($objects, $params['start'],
            $params['limit'], $params['sortfield'], $params['sortdir']);
    }

    public function getData($params, $context) {
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
        $xpath = sprintf("//services/snapraid/datas/data[uuid='%s']", $params['uuid']);
        $object = $xmlConfig->get($xpath);
        if(is_null($object)) {
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $xpath);
        }

        return $object;
    }

    public function setData($params, $context) {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array( "role" => OMV_ROLE_ADMINISTRATOR ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "uuid"          : { '.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].' },
                "name"          : { "type" : "string" },
                "mntentref"     : { '.$GLOBALS['OMV_JSONSCHEMA_UUID'].' },
                "dataroot"      : { "type" : "string" }
            }
        }');

        $parity = $xmlConfig->get("//services/snapraid/mntentref");
        $useqparity = $xmlConfig->get("//services/snapraid/useqparity");
        $qparity = $xmlConfig->get("//services/snapraid/qmntentref");

        if ( $params['mntentref'] == $parity ) {
            throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                gettext("Parity is using this volume. Content cannot use the same volume as Parity or Q-Parity."));
        }

        if ( $useqparity != 0 && $params['mntentref'] == $qparity ) {
            throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                gettext("Q-Parity is using this volume. Content cannot use the same volume as Parity or Q-Parity."));
        }

        $xpath = sprintf("//system/fstab/mntent[uuid='%s']", $params['mntentref']);
        $deviceDir = $xmlConfig->get($xpath);
        $deviceDir = $deviceDir['dir'];

        $params['dataroot'] = "{$deviceDir}";

        // Prepare the configuration object.
        $object = array(
            "uuid"          => ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ? OMVUtil::uuid() : $params['uuid'],
            "name"          => $params['name'],
            "mntentref"     => $params['mntentref'],
            "dataroot"      => $params['dataroot']
        );

        // Set the configuration object.
        $success = FALSE;

        // Append the configuration object.
        $success = $xmlConfig->set("//services/snapraid/datas", array("data" => $object));

        if(FALSE === $success) {
            throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
        }

        // Notify configuration changes.
        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
            OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
            "org.openmediavault.services.snapraid.datas.data", $object);

        // Return the configuration object.
        return $object;
    }

    public function deleteData($params, $context) {
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
        $xpath = sprintf("//services/snapraid/datas/data[uuid='%s']", $params['uuid']);
        $object = $this->deleteConfigObjectByPath($xpath,
            "org.openmediavault.services.snapraid.datas.data");

        // Return the deleted configuration object.
        return $object;
    }

    public function getExcludeList($params, $context) {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array( "role" => OMV_ROLE_ADMINISTRATOR ));

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
        $xpath = "//services/snapraid/excludes/exclude";
        $objects = $xmlConfig->getList($xpath);
        if(is_null($objects)) {
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $xpath);
        }

        // Filter the result.
        return $this->applyFilter($objects, $params['start'], $params['limit'],
            $params['sortfield'], $params['sortdir']);
    }

    public function getExclude($params, $context) {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array("role" => OMV_ROLE_ADMINISTRATOR));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "uuid" : { '.$GLOBALS['OMV_JSONSCHEMA_UUID'].' }
            }
        }');

          // Get the configuration object.
        $xpath = sprintf("//services/snapraid/excludes/exclude[uuid='%s']", $params['uuid']);
        $object = $xmlConfig->get($xpath);
        if(is_null($object)) {
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $xpath);
        }

        return $object;
    }

    public function setExclude($params, $context) {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array( "role" => OMV_ROLE_ADMINISTRATOR ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "uuid"      : { '.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].' },
                "exclusion" : { "type" : "string" }
            }
        }');

        // Prepare configuration data.
        $object = array(
            "uuid"      => ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ? OMVUtil::uuid() : $params['uuid'],
            "exclusion" => $params['exclusion']
        );

        // Set the configuration data.
        $success = FALSE;
        if($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
            // Append object to configuration.
            $success = $xmlConfig->set("//services/snapraid/excludes", array("exclude" => $object));
        } else {
            // Update existing configuration object.
            $success = $xmlConfig->replace("//services/snapraid/excludes/exclude[uuid='{$params['uuid']}']", $object);
        }

        if(FALSE === $success) {
            throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
        }

        // Notify configuration changes.
        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
            OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
            "org.openmediavault.services.snapraid.excludes.exclude", $object);

        // Return the configuration object.
        return $object;
    }

    public function deleteExclude($params, $context) {
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

        // Delete the configuration object.
        $xpath = sprintf("//services/snapraid/excludes/exclude[uuid='%s']", $params['uuid']);
        return $this->deleteConfigObjectByPath($xpath,
            "org.openmediavault.services.snapraid.excludes.exclude");
    }

    public function executeSync($params, $context) {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array( "role" => OMV_ROLE_ADMINISTRATOR ));

        // Create a background process.
        $bgStatusFilename = $this->createBgProcStatus();
        $pid = $this->fork();
        if($pid > 0) { // Parent process.
            $this->initializeBgProcStatus($bgStatusFilename, $pid);
            return $bgStatusFilename;
        }

        // Child process.
        try {
            $bgOutputFilename = $this->createBgProcOutput();
            $this->updateBgProcStatus($bgStatusFilename, "outputfilename", $bgOutputFilename);

            $cmd = "/bin/su";
            $cmdArgs = array("-c", "/usr/local/bin/snapraid sync", "root");
            if(FALSE === $this->execve($cmd, $cmdArgs, $bgOutputFilename)) {
                throw new OMVException(OMVErrorMsg::E_EXEC_FAILED, $cmd." ".implode(" ", $cmdArgs), "");
            }
        } catch(Exception $e) {
            $this->finalizeBgProcStatus($bgStatusFilename, "", $e);
            exit(1);
        }
    }

    public function executeScrub($params, $context) {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array( "role" => OMV_ROLE_ADMINISTRATOR ));

        // Create a background process.
        $bgStatusFilename = $this->createBgProcStatus();
        $pid = $this->fork();
        if($pid > 0) { // Parent process.
            $this->initializeBgProcStatus($bgStatusFilename, $pid);
            return $bgStatusFilename;
        }

        // Child process.
        try {
            $bgOutputFilename = $this->createBgProcOutput();
            $this->updateBgProcStatus($bgStatusFilename, "outputfilename", $bgOutputFilename);

            $cmd = "/bin/su";
            $cmdArgs = array("-c", "/usr/local/bin/snapraid scrub", "root");
            if(FALSE === $this->execve($cmd, $cmdArgs, $bgOutputFilename)) {
                throw new OMVException(OMVErrorMsg::E_EXEC_FAILED, $cmd." ".implode(" ", $cmdArgs), "");
            }
        } catch(Exception $e) {
            $this->finalizeBgProcStatus($bgStatusFilename, "", $e);
            exit(1);
        }
    }

    public function executeCheck($params, $context) {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array( "role" => OMV_ROLE_ADMINISTRATOR ));

        // Create a background process.
        $bgStatusFilename = $this->createBgProcStatus();
        $pid = $this->fork();
        if($pid > 0) { // Parent process.
            $this->initializeBgProcStatus($bgStatusFilename, $pid);
            return $bgStatusFilename;
        }

        // Child process.
        try {
            $bgOutputFilename = $this->createBgProcOutput();
            $this->updateBgProcStatus($bgStatusFilename, "outputfilename", $bgOutputFilename);

            $cmd = "/bin/su";
            $cmdArgs = array("-c", "/usr/local/bin/snapraid check", "root");
            if(FALSE === $this->execve($cmd, $cmdArgs, $bgOutputFilename)) {
                throw new OMVException(OMVErrorMsg::E_EXEC_FAILED, $cmd." ".implode(" ", $cmdArgs), "");
            }
        } catch(Exception $e) {
            $this->finalizeBgProcStatus($bgStatusFilename, "", $e);
            exit(1);
        }
    }

}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceSnapRaid());

?>