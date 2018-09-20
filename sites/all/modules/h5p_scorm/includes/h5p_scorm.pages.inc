<?php

/**
 * @file
 * H5P SCORM menu features.
 */

/**
 * Callback for SCORM configuration.
 */
function h5p_scorm_config_callback($token) {
  if (!H5PCore::validToken('editorajax', $token)) {
    H5PCore::ajaxError(t('Invalid security token. Please reload the editor.'));
    drupal_exit();
  }

  $upload_max_size = file_upload_max_size();

  drupal_json_output(array(
    'upload_max_size' => array(
      'raw' => $upload_max_size,
      'value' => format_size($upload_max_size),
    ),
  ));

  drupal_exit();
}

/**
 * Callback for file uploads.
 */
function h5p_scorm_file_callback($token) {
  if (!H5PCore::validToken('editorajax', $token)) {
    H5PCore::ajaxError(t('Invalid security token. Please reload the editor.'));
    drupal_exit();
  }

  $files_directory = file_stream_wrapper_get_instance_by_uri('public://')->getDirectoryPath();
  if (isset($_POST['contentId']) && $_POST['contentId']) {
    $files_directory .= '/' . variable_get('h5p_default_path', 'h5p') . '/content/' . $_POST['contentId'];
  }
  else {
    $files_directory .= '/h5peditor';
  }

  $file = new H5pScormFile(_h5p_get_instance('interface'), $files_directory);

  if (!$file->isLoaded()) {
    H5PCore::ajaxError(t('File is not loaded.'));
    drupal_exit();
  }

  if ($file->validate()) {
    $file->copy();
  }

  header('Content-type: text/html; charset=utf-8');
  print $file->getResult();
  drupal_exit();
}

/**
 * Menu callback for the SCORM API fetch results.
 *
 * @param object $attempt
 *   Attempt data.
 *
 * @return array
 *   Attempt data.
 */
function h5p_scorm_scorm_api_fetch_callback($attempt) {
  $data = array();
  $cmi = !empty($attempt->cmi_data) ? unserialize($attempt->cmi_data) : array();

  if (isset($cmi['exit']) && ($cmi['exit'] == 'suspend' || $cmi['exit'] == 'logout')) {
    $data['entry'] = 'resume';
  }

  $keys = array(
    'exit',
    'location',
    'suspend_data',
    'completion_status',
    'success_status',
    'progress_measure',
    'score_min',
    'score_max',
    'score_scaled',
    'score_raw',
    'pref_audio',
    'pref_lang',
    'pref_speed',
    'pref_caption',
    'interactions',
    'objectives',
  );

  foreach ($keys as $key) {
    $data[$key] = isset($cmi[$key]) ? $cmi[$key] : '';
  }

  $data['learner_id'] = $attempt->uid;

  return $data;
}

/**
 * Menu callback for the SCORM API commit results.
 *
 * @param object $attempt
 *   Attempt data.
 */
function h5p_scorm_scorm_api_commit_callback($attempt) {
  $cmi_data_raw = file_get_contents('php://input');
  $cmi_data = drupal_json_decode($cmi_data_raw);

  $new_data = array();
  $new_data['activity_report'] = !empty($attempt->activity_report) ? unserialize($attempt->activity_report) : array();

  // Prepare activity objective.
  $activity_objective = !empty($attempt->activity_objective) ? unserialize($attempt->activity_objective) : array();

  foreach (h5p_scorm_get_packege_pages($attempt->nid) as $page_id => $page_title) {
    if (!isset($activity_objective[$page_id])) {
      $activity_objective[$page_id] = array(
        'cmi.completion_status' => '',
        'cmi.success_status' => '',
      );
    }
  }

  // Build activity report.
  if (array_key_exists('activity_report', $cmi_data)) {
    if (!empty($cmi_data['page_id'])) {
      // Build activity objective.
      foreach ($cmi_data['activity_report'] as $item) {
        if (in_array($item['property'], array(
          'cmi.score.min',
          'cmi.score.max',
          'cmi.core.score.min',
          'cmi.core.score.max',
          'cmi.completion_status',
          'cmi.success_status',
        ))
        ) {
          $activity_objective[$cmi_data['page_id']][$item['property']] = $item['value'];
        }
        elseif (in_array($item['property'], array(
          'cmi.score.raw',
          'cmi.score.scaled',
          'cmi.core.score.raw',
          'cmi.core.score.scaled',
        ))
        ) {
          // Don't update score to lower value.
          if (!isset($activity_objective[$cmi_data['page_id']][$item['property']])
            || $activity_objective[$cmi_data['page_id']][$item['property']] < $item['value']
          ) {
            $activity_objective[$cmi_data['page_id']][$item['property']] = $item['value'];
          }
        }
      }

      if (!isset($activity_objective[$cmi_data['page_id']]['cmi.completion_status'])
        || empty($activity_objective[$cmi_data['page_id']]['cmi.completion_status'])
      ) {
        $activity_objective[$cmi_data['page_id']]['cmi.completion_status'] = H5P_SCORM_COMPLETION_STATUS_INCOMPLETE;
        $cmi_data['activity_report'][] = array(
          'property' => 'cmi.completion_status',
          'value' => H5P_SCORM_COMPLETION_STATUS_INCOMPLETE,
        );
      }
    }

    array_push($new_data['activity_report'], array(
      REQUEST_TIME => $cmi_data['activity_report'],
    ));
    unset($cmi_data['activity_report']);
  }

  $completion = array(
    'completed' => 0,
    'incomplete' => 0,
  );

  $success = array(
    'passed' => 0,
    'failed' => 0,
  );

  $scores = 0;
  $quizzes = 0;

  // Calculate values.
  foreach ($activity_objective as $objective) {
    if (empty($objective)) {
      continue;
    }

    if (isset($objective['cmi.completion_status']) && $objective['cmi.completion_status'] == H5P_SCORM_COMPLETION_STATUS_COMPLETED) {
      $completion['completed']++;
    }

    if (isset($objective['cmi.success_status'])) {
      if ($objective['cmi.success_status'] == H5P_SCORM_SUCCESS_STATUS_PASSED) {
        $success['passed']++;
      }
      elseif ($objective['cmi.success_status'] == H5P_SCORM_SUCCESS_STATUS_FAILED) {
        $success['failed']++;
      }
    }

    if (isset($objective['cmi.score.raw'])) {
      $scores += $objective['cmi.score.raw'];
      $quizzes++;
    }

    if (isset($objective['cmi.core.score.raw'])) {
      $scores += $objective['cmi.core.score.raw'];
      $quizzes++;
    }
  }

  if ($completion['completed'] == count($activity_objective)) {
    $new_data['completion_status'] = H5P_SCORM_COMPLETION_STATUS_COMPLETED;
  }
  else {
    $new_data['completion_status'] = H5P_SCORM_COMPLETION_STATUS_INCOMPLETE;
  }

  if ($success['failed'] > 0) {
    $new_data['success_status'] = H5P_SCORM_SUCCESS_STATUS_FAILED;
  }
  elseif ($success['passed'] == count($activity_objective)) {
    $new_data['success_status'] = H5P_SCORM_SUCCESS_STATUS_PASSED;
    // Set complete for passed courses.
    $new_data['completion_status'] = H5P_SCORM_COMPLETION_STATUS_COMPLETED;

    if (!empty($cmi_data['page_id'])) {
      $activity_objective[$cmi_data['page_id']]['cmi.completion_status'] = H5P_SCORM_COMPLETION_STATUS_COMPLETED;
    }
  }

  if ($quizzes > 0) {
    $new_data['score'] = $scores / $quizzes;
  }

  // Custom logs.
  if ($new_data['completion_status'] != $attempt->completion_status) {
    array_push($new_data['activity_report'], array(
      REQUEST_TIME => array(
        array(
          'property' => 'h5p.completion_status',
          'value' => $new_data['completion_status'],
        ),
      ),
    ));
  }
  if ($new_data['success_status'] != $attempt->success_status) {
    array_push($new_data['activity_report'], array(
      REQUEST_TIME => array(
        array(
          'property' => 'h5p.success_status',
          'value' => $new_data['success_status'],
        ),
      ),
    ));
  }

  $new_data['total_time'] = REQUEST_TIME - $attempt->started;
  $new_data['activity_report'] = serialize($new_data['activity_report']);
  $new_data['activity_objective'] = serialize($activity_objective);
  $new_data['cmi_data'] = serialize($cmi_data);
  $new_data['updated'] = REQUEST_TIME;

  db_merge('h5p_scorm_attempt')
    ->key(array(
      'aid' => $attempt->aid,
    ))
    ->fields($new_data)
    ->execute();

  print 'store complete - ' . strlen($cmi_data_raw) . ' bytes received';

  drupal_exit();
}
