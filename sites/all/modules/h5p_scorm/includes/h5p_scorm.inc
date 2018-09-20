<?php

/**
 * @file
 * H5P SCORM additional features.
 */

/**
 * Parse imsmanifest.xml file.
 *
 * @param string $file_path
 *   Path to imsmanifest.xml file.
 *
 * @return bool|object
 *   Manifest data.
 */
function h5p_scorm_parse_imsmanifest($file_path) {
  $manifest = FALSE;

  if (!file_exists($file_path)) {
    return $manifest;
  }

  $contents = file_get_contents($file_path);

  $parser = xml_parser_create();
  xml_parser_set_option($parser, XML_OPTION_CASE_FOLDING, 0);
  xml_parser_set_option($parser, XML_OPTION_SKIP_WHITE, 1);

  $ok = xml_parse_into_struct($parser, $contents, $values, $tags);
  xml_parser_free($parser);

  if ($ok) {
    $manifest = new StdClass();
    $manifest->identifier = '';
    $manifest->version = '';
    $manifest->schema = '';
    $manifest->schemaversion = '';
    $manifest->root = '';
    $manifest->resource_root = '';
    $manifest->organizations = array();
    $manifest->resources = array();
    $manifest->sequencing = array();

    $default_org_id = '';
    $organization = NULL;
    $resource = NULL;
    $item = NULL;
    $item_stack = array();
    $stack = array();
    $ctag = '';

    // Iterate through values.
    foreach ($values as $e) {
      $objective = new stdClass();

      if ($e['type'] == 'open') {
        $tag = $e['tag'];
        if ($tag == 'manifest') {
          if ($ctag) {
            array_push($stack, $ctag);
          }
          $ctag = $tag;
          $manifest->identifier = isset($e['attributes']['identifier']) ?
            $e['attributes']['identifier'] : '';
          $manifest->version = isset($e['attributes']['version']) ?
            $e['attributes']['version'] : '';
          $manifest->root = isset($e['attributes']['xml:base']) ?
            $e['attributes']['xml:base'] : '';
        }
        elseif ($tag == 'metadata') {
          if ($ctag) {
            array_push($stack, $ctag);
          }
          $ctag = $tag;
        }
        elseif ($tag == 'organizations') {
          if ($ctag) {
            array_push($stack, $ctag);
          }
          $ctag = $tag;
          $default_org_id = isset($e['attributes']['default']) ?
            $e['attributes']['default'] : '';
        }
        elseif ($tag == 'organization') {
          if ($ctag) {
            array_push($stack, $ctag);
          }
          $ctag = $tag;
          $organization = new stdClass();
          $organization->id = isset($e['attributes']['identifier']) ?
            $e['attributes']['identifier'] : '';
          $organization->default = FALSE;

          // Set default attribute by organization id or if default not found.
          if ($organization->id == $default_org_id || empty($default_org_id)) {
            $organization->default = TRUE;
          }

          $organization->items = array();
          $organization->controlModeChoice = FALSE;
          $organization->controlModeFlow = FALSE;
        }
        elseif ($tag == 'item') {
          if ($ctag) {
            array_push($stack, $ctag);
          }
          if ($ctag == 'item') {
            array_push($item_stack, $item);
          }
          else {
            $ctag = $tag;
          }
          $item = new stdClass();
          $item->id = isset($e['attributes']['identifier']) ?
            $e['attributes']['identifier'] : '';
          $item->res_id = isset($e['attributes']['identifierref']) ?
            $e['attributes']['identifierref'] : '';
          $item->parms = isset($e['attributes']['parameters']) ?
            $e['attributes']['parameters'] : '';
          $item->timelimitaction = '';
          $item->lmsdata = '';
          $item->completionThreshold = FALSE;
          $item->completedByMeasure = FALSE;
          $item->minProgressMeasure = FALSE;
          $item->progressWeight = FALSE;
          $item->items = array();
          $item->objectives = array();
          $item->completionSetByContent = FALSE;
          $item->objectiveSetByContent = FALSE;
        }
        elseif ($tag == 'resources') {
          if ($ctag) {
            array_push($stack, $ctag);
          }
          $manifest->resource_root = isset($e['attributes']['xml:base']) ?
            $e['attributes']['xml:base'] : '';
        }
        elseif ($tag == 'resource') {
          if ($ctag) {
            array_push($stack, $ctag);
          }
          $resource = new stdClass();
          $resource->id = isset($e['attributes']['identifier']) ?
            $e['attributes']['identifier'] : '';
          $resource->type = isset($e['attributes']['type']) ?
            $e['attributes']['type'] : '';
          $resource->sco_type = isset($e['attributes']['adicp:scormType']) ?
            $e['attributes']['adicp:scormType'] : '';
          $resource->href = isset($e['attributes']['href']) ?
            $e['attributes']['href'] : '';
          $resource->root = isset($e['attributes']['xml:base']) ?
            $e['attributes']['xml:base'] : '';
          $resource->files = array();
          $resource->depends = array();
        }
        elseif ($tag == 'imsss:sequencing') {
          if ($ctag) {
            array_push($stack, $ctag);
          }
        }
        elseif ($tag == 'imsss:objectives') {
          if ($ctag) {
            array_push($stack, $ctag);
          }
        }
        elseif ($tag == 'imsss:primaryObjective') {
          if ($ctag) {
            array_push($stack, $ctag);
          }
          $objective = new stdClass();
          $objective->primary = TRUE;
          $objective->id = isset($e['attributes']['objectiveID']) ?
            $e['attributes']['objectiveID'] : '';
          $objective->satisfiedByMeasure = isset($e['attributes']['satisfiedByMeasure']) ?
            $e['attributes']['satisfiedByMeasure'] : FALSE;
        }
      }
      elseif ($e['type'] == 'complete') {
        $tag = $e['tag'];

        if ($tag == 'schema') {
          $manifest->schema = isset($e['value']) ? $e['value'] : '';
        }
        elseif ($tag == 'schemaversion') {
          $manifest->schemaversion = isset($e['value']) ? $e['value'] : '';
        }
        elseif ($tag == 'title') {
          if ($ctag == 'organization') {
            $organization->title = isset($e['value']) ? $e['value'] : '';
          }
          else {
            if ($ctag == 'item') {
              $item->title = isset($e['value']) ? $e['value'] : '';
            }
          }
        }
        elseif ($tag == 'file') {
          $resource->files[] = isset($e['attributes']['href']) ?
            $e['attributes']['href'] : '';
        }
        elseif ($tag == 'dependency') {
          $resource->depends[] = isset($e['attributes']['identifierref']) ?
            $e['attributes']['identifierref'] : '';
        }
        elseif ($tag == 'adlcp:timeLimitAction') {
          $item->timelimitaction = isset($e['value']) ? $e['value'] : '';
        }
        elseif ($tag == 'adlcp:dataFromLMS') {
          $item->lmsdata = isset($e['value']) ? $e['value'] : '';
        }
        elseif ($tag == 'adlcp:completionThreshold') {
          $item->completionThreshold = isset($e['value']) ? $e['value'] : 0;
          $item->completedByMeasure = isset($e['attributes']['completedByMeasure']) ?
            $e['attributes']['completedByMeasure'] : 0;
          $item->minProgressMeasure = isset($e['attributes']['minProgressMeasure']) ?
            $e['attributes']['minProgressMeasure'] : 0;
          $item->progressWeight = isset($e['attributes']['progressWeight']) ?
            $e['attributes']['progressWeight'] : 0;
        }
        elseif ($tag == 'imsss:objective') {
          $objective = new stdClass();
          $objective->primary = FALSE;
          $objective->id = isset($e['attributes']['objectiveID']) ?
            $e['attributes']['objectiveID'] : '';
          $item->objectives[] = $objective;
        }
        elseif ($tag == 'imsss:minNormalizedMeasure') {
          $objective->minNormalizedMeasure = isset($e['value']) ? $e['value'] :
            0;
        }
        elseif ($tag == 'imsss:deliveryControls' && !empty($item)) {
          $item->completionSetByContent = isset($e['attributes']['completionSetByContent']) ?
            $e['attributes']['completionSetByContent'] : FALSE;
          $item->objectiveSetByContent = isset($e['attributes']['objectiveSetByContent']) ?
            $e['attributes']['objectiveSetByContent'] : FALSE;
        }
        elseif ($tag == 'imsss:controlMode') {
          $organization->controlModeChoice = isset($e['attributes']['choice']) ?
            $e['attributes']['choice'] : FALSE;
          $organization->controlModeFlow = isset($e['attributes']['flow']) ?
            $e['attributes']['flow'] : FALSE;
        }
      }
      // Close the open object and pop the previous open object.
      elseif ($e['type'] == 'close') {

        $tag = $e['tag'];

        if ($tag == 'metadata') {
          $ctag = array_pop($stack);
        }
        elseif ($tag == 'organizations') {
          $ctag = array_pop($stack);
        }
        elseif ($tag == 'organization') {
          $ctag = array_pop($stack);
          $manifest->organizations[] = $organization;
          $organization = NULL;
        }
        elseif ($tag == 'item') {
          $ctag = array_pop($stack);
          if ($ctag == 'item') {
            $parent_item = array_pop($item_stack);
            $parent_item->items[] = $item;
            $item = $parent_item;
          }
          else {
            $organization->items[] = $item;
            $item = NULL;
          }
        }
        elseif ($tag == 'resources') {
          $ctag = array_pop($stack);
        }
        elseif ($tag == 'resource') {
          $ctag = array_pop($stack);
          $manifest->resources[] = $resource;
          $resource = NULL;
        }
        elseif ($tag == 'imsss:sequencing') {
          $ctag = array_pop($stack);
        }
        elseif ($tag == 'imsss:objectives') {
          $ctag = array_pop($stack);
        }
        elseif ($tag == 'imsss:primaryObjective') {
          $ctag = array_pop($stack);
          $item->objectives[] = $objective;
        }
      }
    }
  }

  return $manifest;
}

/**
 * Get organization (default or specific by id).
 *
 * @param object $manifest
 *   Manifest data.
 * @param bool|string $org_id
 *   Organization id or FALSE.
 *
 * @return bool|object
 *   Returns organization data or FALSE.
 */
function h5p_scorm_get_organization($manifest, $org_id = FALSE) {
  $org = FALSE;

  if (empty($manifest)) {
    return $org;
  }

  foreach ($manifest->organizations as $organization) {
    // If no org id specified and org is default, return it.
    if (!$org_id && $organization->default) {
      $org = $organization;
      break;
    }

    // Check for id match.
    if ($org_id && $organization->id == $org_id) {
      $org = $organization;
      break;
    }
  }

  return $org;
}

/**
 * Get SCORM navigation items.
 *
 * @param object $manifest
 *   Manifest data.
 * @param array $org_items
 *   Organization items.
 * @param string $path
 *   Uri path to the SCORM package.
 *
 * @return array
 *   Returns navigation list.
 */
function h5p_scorm_nav_list_items($manifest, $org_items, $path) {
  if (empty($manifest)) {
    return array();
  }

  $info = array();

  foreach ($org_items as $item) {
    // Only add items with resources.
    if ($item->res_id) {
      $files = array();

      if (!empty($item->files)) {
        $files = array_merge($files, $item->files);
      }

      $res = h5p_scorm_get_resource($manifest, $item->res_id);

      if (!empty($res->depends)) {
        foreach ($res->depends as $depend) {
          $depend_res = h5p_scorm_get_resource($manifest, $depend);

          if (!empty($depend_res->files)) {
            $files = array_merge($files, $depend_res->files);
          }
        }
      }

      if ($res && $res->href) {
        $uri = $path . DIRECTORY_SEPARATOR . $res->href . $item->parms;
        $uri = file_create_url($uri);

        $info[] = array(
          'active' => FALSE,
          'id' => $item->id,
          'lmsdata' => $item->lmsdata,
          'title' => $item->title,
          'url' => urldecode($uri),
          'files' => $files,
        );
      }
    }

    // Recurse to add inner items.
    if (count($item->items) > 0) {
      $info = array_merge($info, h5p_scorm_nav_list_items($manifest, $item->items, $path));
    }
  }

  return $info;
}

/**
 * Get resource data.
 *
 * @param object $manifest
 *   Manifest data.
 * @param string $res_id
 *   Resource id.
 *
 * @return bool|object
 *   Resource object or FALSE.
 */
function h5p_scorm_get_resource($manifest, $res_id) {
  $res = FALSE;

  if (empty($manifest)) {
    return $res;
  }

  foreach ($manifest->resources as $resource) {
    if ($resource->id == $res_id) {
      $res = $resource;
      break;
    }
  }

  return $res;
}

/**
 * Get the starting resource.
 *
 * @param object $items
 *   Items data.
 * @param int $nid
 *   Node id.
 *
 * @return bool|object
 *   Returns item object or FALSE.
 */
function h5p_scorm_start_resource($items, $nid = NULL) {
  if (empty($items)) {
    return FALSE;
  }

  // Get last attempt.
  $attempt = h5p_scorm_get_last_attempt($GLOBALS['user']->uid, $nid);

  foreach ($items as $item) {
    if (count($item->items) > 0) {
      return h5p_scorm_start_resource($item->items, $nid);
    }

    if ($item->res_id && empty($attempt->cmi_data['page_id'])) {
      return $item;
    }
    elseif (!empty($attempt->cmi_data['page_id']) && $item->id == $attempt->cmi_data['page_id']) {
      $item->other_page = TRUE;

      return $item;
    }
  }

  return FALSE;
}

/**
 * Get lrs settings (endpoint, user login and pass).
 */
function h5p_scorm_get_lrs_settings() {
  $settings = array(
    'endpoint' => variable_get('h5p_scorm_lrs_endpoint', ''),
    'auth_user' => variable_get('h5p_scorm_lrs_auth_user', ''),
    'auth_password' => variable_get('h5p_scorm_lrs_auth_password', ''),
  );

  drupal_alter('h5p_scorm_lrs_settings', $settings);

  return $settings;
}

/**
 * Parse tincan xml.
 */
function h5p_scorm_parse_tincan($file_path) {
  $tincan = FALSE;
  if (!file_exists($file_path)) {
    return $tincan;
  }

  $contents = file_get_contents($file_path);
  $parser = xml_parser_create();
  xml_parser_set_option($parser, XML_OPTION_CASE_FOLDING, 0);
  xml_parser_set_option($parser, XML_OPTION_SKIP_WHITE, 1);
  $ok = xml_parse_into_struct($parser, $contents, $values, $tags);
  xml_parser_free($parser);

  if ($ok) {
    $tincan = new StdClass();

    $array_tags = array('launch', 'description', 'name', 'activity');
    foreach ($values as $value) {
      if (in_array($value['tag'], $array_tags) && isset($value['value'])) {
        $tincan->{$value['tag']} = $value['value'];
      }
      elseif (isset($value['attributes']['id'])) {
        $tincan->{$value['tag']} = $value['attributes']['id'];
      }
    }
  }

  return $tincan;
}

/**
 * Generate uuid for tincan request.
 */
function h5p_scorm_tincan_get_uuid() {
  $random_string = openssl_random_pseudo_bytes(16);
  $time_low = bin2hex(substr($random_string, 0, 4));
  $time_mid = bin2hex(substr($random_string, 4, 2));
  $time_hi_and_version = bin2hex(substr($random_string, 6, 2));
  $clock_seq_hi_and_reserved = bin2hex(substr($random_string, 8, 2));
  $node = bin2hex(substr($random_string, 10, 6));
  $time_hi_and_version = hexdec($time_hi_and_version);
  $time_hi_and_version = $time_hi_and_version >> 4;
  $time_hi_and_version = $time_hi_and_version | 0x4000;
  $clock_seq_hi_and_reserved = hexdec($clock_seq_hi_and_reserved);
  $clock_seq_hi_and_reserved = $clock_seq_hi_and_reserved >> 2;
  $clock_seq_hi_and_reserved = $clock_seq_hi_and_reserved | 0x8000;

  return sprintf(
    '%08s-%04s-%04x-%04x-%012s',
    $time_low,
    $time_mid,
    $time_hi_and_version,
    $clock_seq_hi_and_reserved,
    $node
  );
}

/**
 * Get tincan configurations for course.
 */
function h5p_scorm_get_tincan_configurations() {
  // Get LRS settings.
  $settings = h5p_scorm_get_lrs_settings();

  return array(
    'endpoint' => $settings['endpoint'] . '/',
    'auth' => 'Basic ' . base64_encode($settings['auth_user'] . ':' . $settings['auth_password']),
  );
}

/**
 * Create an 'actor' for the LRS.
 *
 * @param mixed $account
 *   Optional. A Drupal user object or a user id.
 *
 * @return array
 *   An array representing an 'actor'.
 */
function h5p_scorm_get_actor($account = NULL) {
  if (is_null($account)) {
    global $user;
    $account = user_load($user->uid);
  }
  else {
    if (is_int($account)) {
      $account = user_load($account);
    }
  }

  return array(
    'name' => array(
      format_username($account),
    ),
    'mbox' => array(
      $account->mail ? 'mailto:' . $account->mail : 'mailto:' . variable_get('site_mail'),
    ),
    'objectType' => 'Agent',
  );
}

/**
 * Create the new attempt.
 *
 * @param int $nid
 *   Node nid.
 *
 * @return int
 *   Attempt id.
 */
function h5p_scorm_attempt_create($nid) {
  if (empty($nid)) {
    return NULL;
  }

  global $user;

  $query = db_select('h5p_scorm_attempt', 'a');
  $query->fields('a', array(
    'delta',
    'completion_status',
    'success_status',
    'score',
    'cmi_data',
    'activity_objective',
  ));
  $query->condition('a.uid', $user->uid);
  $query->condition('a.nid', $nid);
  $query->orderBy('a.updated', 'DESC');
  $old_data = $query->execute()->fetchAssoc();

  // Prepare objectives list.
  if (empty($old_data['cmi_data']) && $node = node_load($nid)) {
    $objectives = array();
    $params = drupal_json_decode($node->json_content);

    if ($manifest = h5p_scorm_parse_imsmanifest($params['scorm']['extracted'] . DIRECTORY_SEPARATOR . 'imsmanifest.xml')) {
      // Get default organization.
      $org = h5p_scorm_get_organization($manifest);

      if (!empty($org->items[0]->objectives)) {
        foreach ($org->items[0]->objectives as $objective) {
          if (isset($objective->id)) {
            $objectives[] = array(
              'id' => $objective->id,
            );
          }
        }
      }
    }

    $old_data['cmi_data'] = serialize(array(
      'objectives' => $objectives,
    ));
  }

  $attempt_id = db_insert('h5p_scorm_attempt')
    ->fields(array(
      'uid' => $user->uid,
      'nid' => $nid,
      'delta' => !empty($old_data['delta']) ? $old_data['delta'] + 1 : 1,
      'completion_status' => !empty($old_data['completion_status']) ? $old_data['completion_status'] : '',
      'success_status' => !empty($old_data['success_status']) ? $old_data['success_status'] : '',
      'score' => !empty($old_data['score']) ? $old_data['score'] : NULL,
      'cmi_data' => !empty($old_data['cmi_data']) ? $old_data['cmi_data'] : NULL,
      'activity_report' => NULL,
      'activity_objective' => !empty($old_data['activity_objective']) ? $old_data['activity_objective'] : NULL,
      'started' => REQUEST_TIME,
      'updated' => REQUEST_TIME,
    ))
    ->execute();

  return $attempt_id;
}

/**
 * Get pages list by node NID.
 *
 * @param int $nid
 *   Node NID.
 *
 * @return array
 *   Package pages list.
 */
function h5p_scorm_get_packege_pages($nid) {
  $pages = array();

  if (empty($nid)) {
    return $pages;
  }

  $query = db_select('h5p_nodes', 'n');
  $query->fields('n', array('json_content'));
  $query->condition('n.nid', $nid);

  if ($data = $query->execute()->fetchField()) {
    if (($data = drupal_json_decode($data)) && isset($data['scorm']['pages'])) {
      foreach ($data['scorm']['pages'] as $page) {
        $pages[$page['id']] = $page['title'];
      }
    }
  }

  return $pages;
}

/**
 * Get last attempt.
 */
function h5p_scorm_get_last_attempt($uid, $nid) {
  if (empty($nid)) {
    return FALSE;
  }

  $result = &drupal_static(__FUNCTION__ . '_' . implode('_', array($uid, $nid)));
  if ($result !== NULL) {
    return $result;
  }

  $query = db_select('h5p_scorm_attempt', 'a');
  $query->fields('a');
  $query->condition('a.nid', $nid);
  $query->condition('a.uid', $uid);
  $query->orderBy('a.aid', 'DESC');
  $query->range(0, 1);

  if ($result = $query->execute()->fetchAll()) {
    $result = reset($result);
    $result->cmi_data = unserialize($result->cmi_data);
  }

  return $result;
}

/**
 * Get scorm pages by node NID.
 *
 * @param int $nid
 *   Node NID.
 *
 * @return array
 *   Returns all pages.
 */
function h5p_scorm_pages_by_nid($nid) {
  $pages = &drupal_static(__FUNCTION__ . '_' . $nid);

  if ($pages !== NULL) {
    return $pages;
  }

  $query = db_select('h5p_nodes', 'n');
  $query->fields('n', array('json_content'));
  $query->condition('n.nid', $nid);

  if ($json_content = $query->execute()->fetchField()) {
    $json_content = drupal_json_decode($json_content);

    if (isset($json_content['scorm'], $json_content['scorm']['pages'])) {
      foreach ($json_content['scorm']['pages'] as $page) {
        $pages[$page['id']] = $page['title'];
      }
    }
  }

  return $pages;
}

/**
 * Get init page.
 *
 * @param string $url
 *   Path to extracted package directory.
 *
 * @return string
 *   Path including the init file.
 */
function _h5p_scorm_get_page_init_url($url) {
  if ($manifest = h5p_scorm_parse_imsmanifest($url . DIRECTORY_SEPARATOR . 'imsmanifest.xml')) {
    // Get default organization.
    $org = h5p_scorm_get_organization($manifest);

    // Find the starting resource.
    if ($item = h5p_scorm_start_resource($org->items)) {
      $res = h5p_scorm_get_resource($manifest, $item->res_id);

      if ($res && $res->href) {
        $url .= DIRECTORY_SEPARATOR . $res->href . $item->parms;
      }
    }
  }
  elseif ($tincan = h5p_scorm_parse_tincan($url . DIRECTORY_SEPARATOR . 'tincan.xml')) {
    $url .= DIRECTORY_SEPARATOR . $tincan->launch;
  }

  return base_path() . $url;
}

/**
 * Get list of SCORM pages.
 *
 * @param string $url
 *   Path to extracted package directory.
 *
 * @return array
 *   List of pages.
 */
function _h5p_scorm_get_pages($url) {
  $pages = array();

  if (!$manifest = h5p_scorm_parse_imsmanifest($url . DIRECTORY_SEPARATOR . 'imsmanifest.xml')) {
    return $pages;
  }

  $org = h5p_scorm_get_organization($manifest);

  if ($elements = h5p_scorm_nav_list_items($manifest, $org->items, $url)) {
    foreach ($elements as $element) {
      $pages[] = $element;
    }
  }

  return $pages;
}
