<?php

/**
 * @file
 *   Provides admin settings form(s)
 */

/**
 * General settings form
 */
function jw_player_settings_form($form) {
  drupal_set_title(t('JW Player general settings'));

  $versions = variable_get(
    'jw_player_available_versions',
    array(5 => 5, 6 => 6, 7 => 7)
  );
  $url = 'https://dashboard.jwplayer.com/#/players/downloads';

  $form['jw_player_version'] = array(
    '#type' => 'select',
    '#title' => t('Player version'),
    '#options' => $versions,
    '#default_value' => variable_get('jw_player_version', 6),
    '#description' => t('Select the version of JW Player you are using.'),
  );

  $form['jw_player_hosting'] = array(
    '#type' => 'radios',
    '#title' => t('Hosting type'),
    '#options' => array(
      'self' => t('Self-Hosted'),
      'cloud' => t('Cloud-Hosted'),
    ),
    '#default_value' => variable_get('jw_player_hosting', 'self'),
    '#description' => t('Choose if JW Player will be downloaded and self-hosted, or the site will use JW Player\'s cloud-hosting service.'),
    '#states' => array(
      'unchecked' => array(
        'select[name="jw_player_version"]' => array('value' => '5'),
      ),
      'visible' => array(
        array(
          array('select[name="jw_player_version"]' => array('value' => '6')),
          array('select[name="jw_player_version"]' => array('value' => '7')),
        ),
      ),
    ),
  );

  $form['jw_player_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Self-Hosted Player License Key'),
    '#description' => t('Enter your key here. You can retrieve the license key from your <a href="@url" target="_blank">JWPlayer.com dashboard</a>.', array(
      '@url' => $url,
    )),
    '#default_value' => variable_get('jw_player_key', NULL),
    '#states' => array(
      'required' => array(
        array(
          'select[name="jw_player_version"]' => array('value' => '5'),
        ),
        array(
          'select[name="jw_player_version"]' => array('value' => '6'),
          ':input[name="jw_player_hosting"]' => array('value' => 'self'),
        ),
      ),
      'visible' => array(
        array(
          'select[name="jw_player_version"]' => array('value' => '5'),
        ),
        array(
          'select[name="jw_player_version"]' => array('value' => '6'),
          ':input[name="jw_player_hosting"]' => array('value' => 'self'),
        ),
      ),
    ),
  );

  $form['jw_player_key_7'] = array(
    '#type' => 'textfield',
    '#title' => t('Self-Hosted Player License Key'),
    '#description' => t('Enter your key here. You can retrieve the license key from your <a href="@url" target="_blank">JWPlayer.com dashboard</a>.', array(
      '@url' => $url,
    )),
    '#default_value' => variable_get('jw_player_key_7', NULL),
    '#states' => array(
      'required' => array(
        'select[name="jw_player_version"]' => array('value' => '7'),
        ':input[name="jw_player_hosting"]' => array('value' => 'self'),
      ),
      'visible' => array(
        'select[name="jw_player_version"]' => array('value' => '7'),
        ':input[name="jw_player_hosting"]' => array('value' => 'self'),
      ),
    ),
  );

  $form['jw_player_cloud_player_default'] = array(
    '#type' => 'textfield',
    '#title' => t('Default Player Library URL'),
    '#description' => t('Get the URL for your cloud-hosted player from your <a href="@url" target="_blank">JWPlayer.com dashboard</a>. After choosing your player, copy and enter the URL to this field. If you are using the cloud-hosted player, the self-hosted files will not be loaded. Player settings can be overridden with <a href="@preset_url">Drupal-defined presets</a>.', array(
      '@url' => $url,
      '@preset_url' => 'admin/config/media/jw_player',
    )),
    '#default_value' => variable_get('jw_player_cloud_player_default', NULL),
    '#states' => array(
      'required' => array(
        array(
          'select[name="jw_player_version"]' => array('value' => '6'),
          ':input[name="jw_player_hosting"]' => array('value' => 'cloud'),
        ),
        array(
          'select[name="jw_player_version"]' => array('value' => '7'),
          ':input[name="jw_player_hosting"]' => array('value' => 'cloud'),
        ),
      ),
      'visible' => array(
        array(
          'select[name="jw_player_version"]' => array('value' => '6'),
          ':input[name="jw_player_hosting"]' => array('value' => 'cloud'),
        ),
        array(
          'select[name="jw_player_version"]' => array('value' => '7'),
          ':input[name="jw_player_hosting"]' => array('value' => 'cloud'),
        ),
      ),
    ),
  );

  $form['#attached']['js'] = array(
    drupal_get_path('module', 'jw_player') . '/js/jw_player.admin.js',
  );

  $form['#validate'][] = 'jw_player_settings_form_validate';

  return system_settings_form($form);
}

/**
 * General settings form validate.
 */
function jw_player_settings_form_validate($form, &$form_state) {
  $values = $form_state['values'];
  $v = (int) $values['jw_player_version'];
  $host = $values['jw_player_hosting'];
  if ($host == 'cloud') {
    if (empty($values['jw_player_cloud_player_default'])) {
      form_set_error('jw_player_cloud_player_default', t('Default Player Library URL is required when Hosting type is Cloud-Hosted.'));
    }
    else {
      preg_match(jw_player_library_url_regex(), $values['jw_player_cloud_player_default'], $matches);
      if (!isset($matches[2])) {
        form_set_error('jw_player_cloud_player_default', t('Default Player Library URL does not match format provided by JWPlayer.com.'));
      }
    }
  }
  else {
    $key = 'jw_player_key' . (($v == 7) ? '_7' : '');
    if (empty($values[$key])) {
      form_set_error($key, t('Self-Hosted Player License Key is required when Hosting type is Self-Hosted.'));
    }
    else {
      if (@base64_encode(base64_decode($values[$key])) !== $values[$key]) {
        form_set_error($key, t('Self-Hosted Player License Key must be a valid Base64 encoded value.'));
      }
    }
  }
}
