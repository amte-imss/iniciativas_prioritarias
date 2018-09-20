<?php
/**
 * @file
 * Defines the CTools Export UI plugin for JW Player presets.
 */
$plugin = array(
  'schema' => 'jwplayer_preset',
  'access' => 'administer JW Player presets',
  'menu' => array(
    'menu item' => 'jw_player',
    'menu title' => 'JW Player presets',
    'menu prefix' => 'admin/config/media',
    'menu description' => 'Administer JW Player presets.',
  ),

  'title singular' => t('preset'),
  'title plural' => t('presets'),
  'title singular proper' => t('JW Player preset'),
  'title plural proper' => t('JW Player presets'),

  'handler' => 'jw_player_ctools_export_ui',

  'form' => array(
    'settings' => 'jw_player_ctools_export_ui_form',
    'validate' => 'jw_player_ctools_export_ui_form_validate',
  ),
);

/**
 * Implements hook_ctools_export_ui_form().
 */
function jw_player_ctools_export_ui_form(&$form, &$form_state) {
  $preset = $form_state['item'];
  $settings = $preset->settings;

   // This is a hack. CTools adds a hierarchy for export_key in form of
   // $form['info][$export_key] (see line 1007 of
   // ctools/plugins/export_ui/ctools_export_ui.class.php).
   // The FAPI machine name object doesn't seem to work well with fields present
   // in different levels of hierarchy, hence we move the fields over to the same
   // level in the hierarchy.
  unset($form['info']);

  $form['preset_name'] = array(
    '#type' => 'textfield',
    '#size' => 20,
    '#maxlength' => 255,
    '#title' => t('Preset name'),
    '#description' => t('Enter name for the preset.'),
    '#default_value' =>  $preset->preset_name,
    '#required' => TRUE,
    '#weight' => 0,
  );

  $form['info']['machine_name'] = array(
    '#title' => t('Machine name'),
    '#type' => 'machine_name',
    '#default_value' => $preset->machine_name,
    '#machine_name' => array(
      'exists' => 'jw_player_preset_load',
      'source' => array('preset_name'),
    ),
    '#weight' => 1,
    '#description' => t('Enter the machine name for the preset. It must be unique and contain only alphanumeric characters and underscores.'),
  );

  $form['description'] = array(
    '#type' => 'textarea',
    '#size' => 10,
    '#title' => t('Description'),
    '#description' => t('Summary for the preset.'),
    '#default_value' => $preset->description,
    '#weight' => 2,
  );

  $form['settings'] = array(
    '#type' => 'fieldset',
    '#title' => 'Settings',
    '#tree' => TRUE,
    '#weight' => 5,
  );

  // Disable preset source if using legacy JW Player, or if using newest
  // JW Player and self-hosting.
  if (jw_player_use_legacy() || (!jw_player_use_legacy() && variable_get('jw_player_hosting', 'self') == 'self')) {
    $disabled = TRUE;
    $desc = t('When creating JW Player presets, select if the source should be defined on Drupal, or by using definitions set within your JWPlayer.com account. <strong>Only available for sites using cloud hosting with JW Player 7 or above.</strong>');
  }
  else {
    $disabled = FALSE;
    $desc = t('When creating JW Player presets, select if the source should be defined on Drupal, or by using definitions set within your JWPlayer.com account.');
  }
  $form['settings']['preset_source'] = array(
    '#type' => 'radios',
    '#title' => t('Preset source'),
    '#disabled' => $disabled,
    '#options' => array(
      'drupal' => t('Use Drupal-defined presets'),
      'jwplayer' => t('Use presets defined on JWPlayer.com'),
    ),
    '#default_value' => isset($settings['preset_source']) ? $settings['preset_source'] : 'drupal',
    '#description' => $desc,
  );

  // Some settings are suitable for legacy versions only.
  $skin_options = array();
  if (jw_player_use_legacy()) {
    $form['settings']['primary'] = array(
      '#type' => 'radios',
      '#title' => t('Embed mode'),
      '#description' => t('Select your primary embed mode. Choosing HTML5 primary means that modern browsers that also support flash will use the HTML5 player first where possible. While this is desirable, the Flash based player supports more features and is generally more reliable.'),
      '#options' => array(
        'html5' => t('HTML5 primary, Flash failover'),
        'flash' => t('Flash primary, HTML5 failover'),
      ),
      '#default_value' => isset($settings['mode']) ? $settings['mode'] : 'html5',
      '#states' => array(
        'visible' => array(
          ':input[name="settings[preset_source]"]' => array('value' => 'drupal'),
        ),
      ),
    );

    // Legacy Skins.
    $skin_options = array(
      'bekle' => 'Bekle*',
      'modieus' => 'Modieus*',
      'glow' => 'Glow*',
      'five' => 'Five*',
      'beelden' => 'Beelden*',
      'stormtrooper' => 'Stormtrooper*',
      'vapor' => 'Vapor*',
      'roundster' => 'Roundster*'
    );
    $skin_desc = t('Select a player skin. Some skins (*) require a premium license.');
  }
  else {
    // JW Player 7 Skins if cloud-hosting. If self-hosting, jw_player_skins()
    // function retrieves all possible custom and library skins.
    if (variable_get('jw_player_hosting', 'self') == 'cloud') {
      $skin_options = array(
        'beelden' => 'Beelden',
        'bekle' => 'Bekle',
        'five' => 'Five',
        'glow' => 'Glow',
        'roundster' => 'Roundster',
        'seven' => 'Seven',
        'six' => 'Six',
        'stormtrooper' => 'Stormtrooper',
        'vapor' => 'Vapor',
      );
    }
    $skin_url = 'https://support.jwplayer.com/customer/portal/articles/1406968-using-jw-player-skins';
    $skin_desc = t('Select a player skin. <a href="@url" target="_blank">Click here</a> to see skins provided by JW Player.', array(
      '@url' => $skin_url,
    ));
  }

  // For legacy, add custom skins. For newer, add only custom skins if
  // cloud-hosting, or add custom and library skins for self-hosting.
  foreach (jw_player_skins() as $skin) {
    $skin_options[$skin->name] = drupal_ucfirst($skin->name);
  }

  $form['settings']['skin'] = array(
    '#title' => t('Skin'),
    '#description' => $skin_desc,
    '#type' => 'select',
    '#default_value' => !empty($settings['skin']) ? $settings['skin'] : FALSE,
    '#empty_option' => t('None (default skin)'),
    '#options' => $skin_options,
    '#states' => array(
      'visible' => array(
        ':input[name="settings[preset_source]"]' => array('value' => 'drupal'),
      ),
    ),
  );

  $form['settings']['stretching'] = array(
    '#title' => t('Stretching'),
    '#description' => t('How to resize the poster and video to fit the display.'),
    '#type' => 'select',
    '#default_value' => !empty($settings['stretching']) ? $settings['stretching'] : 'uniform',
    '#options' => array(
      'none' => t('None (keep original dimensions)'),
      'exactfit' => t('Exact Fit (stretch disproportionally)'),
      'uniform' => t('Uniform (stretch proportionally; black borders)'),
      'fill' => t('Fill (stretch proportionally; parts cut off)'),
    ),
    '#states' => array(
      'visible' => array(
        ':input[name="settings[preset_source]"]' => array('value' => 'drupal'),
      ),
    ),
  );

  $form['settings']['responsive'] = array(
    '#type' => 'checkbox',
    '#title' => 'Use a Responsive Design',
    '#description' => t('Enable Responsive Design using a percentage based width and an aspect ratio.'),
    '#default_value' => isset($settings['responsive']) ? $settings['responsive'] : FALSE,
    '#states' => array(
      'visible' => array(
        ':input[name="settings[preset_source]"]' => array('value' => 'drupal'),
      ),
    ),
  );

  $form['settings']['width'] = array(
    '#type' => 'textfield',
    '#size' => 10,
    '#title' => t('Width'),
    '#description' => t('Enter the width for this player.'),
    '#field_suffix' => t('pixels'),
    '#default_value' => isset($settings['width']) ? $settings['width'] : '',
    '#states' => array(
      'required' => array(
        ':input[name="settings[preset_source]"]' => array('value' => 'drupal'),
      ),
      'visible' => array(
        ':input[name="settings[preset_source]"]' => array('value' => 'drupal'),
      ),
    ),
  );

  $form['settings']['height'] = array(
    '#type' => 'textfield',
    '#size' => 10,
    '#title' => t('Height'),
    '#description' => t('Enter the height for this player.'),
    '#field_suffix' => t('pixels'),
    '#default_value' => isset($settings['height']) ? $settings['height'] : '',
    '#states' => array(
      'required' => array(
        ':input[name="settings[responsive]"]' => array('checked' => FALSE),
      ),
      'visible' => array(
        ':input[name="settings[responsive]"]' => array('checked' => FALSE),
        ':input[name="settings[preset_source]"]' => array('value' => 'drupal'),
      ),
    ),
  );

  $form['settings']['aspectratio'] = array(
    '#type' => 'textfield',
    '#size' => 10,
    '#title' => t('Aspect ratio'),
    '#description' => t('Enter the aspect ratio for this player.'),
    '#default_value' => isset($settings['aspectratio']) ? $settings['aspectratio'] : '16:9',
    '#states' => array(
      'required' => array(
        ':input[name="settings[responsive]"]' => array('checked' => TRUE),
      ),
      'visible' => array(
        ':input[name="settings[responsive]"]' => array('checked' => TRUE),
        ':input[name="settings[preset_source]"]' => array('value' => 'drupal'),
      ),
    ),
  );

  $form['settings']['autostart'] = array(
    '#title' => t('Autostart'),
    '#type' => 'checkbox',
    '#description' => t('Automatically start playing the video on page load. Can be true or false (default). Autostart does not work on mobile devices like iOS and Android.'),
    '#default_value' => isset($settings['autostart']) ? $settings['autostart'] : FALSE,
    '#states' => array(
      'visible' => array(
        ':input[name="settings[preset_source]"]' => array('value' => 'drupal'),
      ),
    ),
  );

  $form['settings']['mute'] = array(
    '#type' => 'checkbox',
    '#title' => t('Mute'),
    '#description' => t('Mute the player by default during playback. This can be useful when combined with the autostart option. Cannot control settings of external sources such as YouTube.'),
    '#default_value' => isset($settings['mute']) ? $settings['mute'] : FALSE,
    '#states' => array(
      'visible' => array(
        ':input[name="settings[preset_source]"]' => array('value' => 'drupal'),
      ),
    ),
  );

  $form['settings']['controls'] = array(
    '#title' => t('Controls'),
    '#type' => 'checkbox',
    '#description' => t('Whether to display the video controls (controlbar, icons and dock).'),
    '#default_value' => isset($settings['controls']) ? $settings['controls']: TRUE,
    '#states' => array(
      'visible' => array(
        ':input[name="settings[preset_source]"]' => array('value' => 'drupal'),
      ),
    ),
  );

  // Social media sharing.
  if (!jw_player_use_legacy()) {
    $form['settings']['sharing'] = array(
      '#type' => 'checkbox',
      '#title' => t('Sharing'),
      '#description' => t('Enable the social sharing overlay. If no sharing options are selected, the page URL link with default sharing sites will be shown.'),
      '#default_value' => isset($settings['sharing']) ? $settings['sharing'] : FALSE,
      '#states' => array(
        'visible' => array(
          ':input[name="settings[preset_source]"]' => array('value' => 'drupal'),
        ),
      ),
    );

    $form['settings']['sharing_heading'] = array(
      '#type' => 'textfield',
      '#title' => t('Sharing heading text'),
      '#description' => t('Short, instructive text to display at the top of the sharing screen.'),
      '#default_value' => !empty($settings['sharing_heading']) ? $settings['sharing_heading'] : t('Share Video'),
      '#states' => array(
        'visible' => array(
          ':input[name="settings[sharing]"]' => array('checked' => TRUE),
          ':input[name="settings[preset_source]"]' => array('value' => 'drupal'),
        ),
      ),
    );

    $sharing_sites = jw_player_sharing_sites();
    $form['settings']['sharing_sites_show'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Sharing sites'),
      '#options' => $sharing_sites,
      '#description' => t('Built-in social network options to include with the sharing overlay.'),
      '#default_value' => !empty($settings['sharing_sites_show']) ? $settings['sharing_sites_show'] : array(),
      '#states' => array(
        'visible' => array(
          ':input[name="settings[sharing]"]' => array('checked' => TRUE),
          ':input[name="settings[preset_source]"]' => array('value' => 'drupal'),
        ),
      ),
    );

    // Filter sharing sites order (tabledrag).
    $form['settings']['sharing_sites_order'] = array(
      '#type' => 'item',
      '#title' => t('Sharing sites order'),
      '#theme' => 'jw_player_sharing_sites_order',
      '#states' => array(
        'visible' => array(
          ':input[name="settings[sharing]"]' => array('checked' => TRUE),
          ':input[name="settings[preset_source]"]' => array('value' => 'drupal'),
        ),
      ),
    );

    $counter = -50;
    $sharing_sites_order_defaults = array();
    foreach ($sharing_sites as $site_id => $sharing_site) {
      $sharing_sites_order_defaults[$site_id] = $counter;
      $counter++;
    }
    foreach ($sharing_sites as $site_id => $sharing_site) {
      if (isset($settings['sharing_sites_order'][$site_id]['weight'])) {
        $default_weight = $settings['sharing_sites_order'][$site_id]['weight'];
      }
      else {
        $default_weight = $sharing_sites_order_defaults[$site_id];
      }
      $form['settings']['sharing_sites_order'][$site_id] = array(
        'content' => array(
          '#markup' => $sharing_site,
        ),
        'weight' => array(
          '#type' => 'weight',
          '#title' => t('Weight for @title', array('@title' => $sharing_site)),
          '#title_display' => 'invisible',
          '#delta' => 50,
          '#default_value' => $default_weight,
          '#array_parents' => array('settings', 'sharing_sites_order', $site_id),
        ),
        '#weight' => $default_weight,
        '#states' => array(
          'visible' => array(
            ':input[name="settings[preset_source]"]' => array('value' => 'drupal'),
          ),
        ),
      );
    }
  }

  // Add preset plugin settings.
  foreach (jw_player_preset_plugins() as $plugin => $info) {
    // Fieldset per plugin.
    $form['settings']['plugins'][$plugin] = array(
      '#type' => 'fieldset',
      '#title' => check_plain($info['name']),
      '#description' => check_plain($info['description']),
      '#tree' => TRUE,
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#states' => array(
        'visible' => array(
          ':input[name="settings[preset_source]"]' => array('value' => 'drupal'),
        ),
      ),
    );

    // Enable/disable plugin setting.
    $form['settings']['plugins'][$plugin]['enable'] = array(
      '#type' => 'checkbox',
      '#title' => t('Enable'),
      '#description' => check_plain($info['description']),
      '#default_value' => isset($settings['plugins'][$plugin]['enable']) ? $settings['plugins'][$plugin]['enable'] : FALSE,
    );

    // Add each config option specified in the plugin. Config options should be
    // in FAPI structure.
    if (is_array($info['config options']) and !empty($info['config options'])) {
      foreach ($info['config options'] as $option => $element) {
        // Note: Each config option must be a complete FAPI element, except for
        // the #title which is optional. If the #title is not provided, we use
        // the name of the config option as the title.
        if (!isset($element['#title'])) {
          $element['#title'] = drupal_ucfirst($option);
        }
        // Alter the default value if a setting has been saved previously.
        $element['#default_value'] = !empty($settings['plugins'][$plugin][$option]) ? $settings['plugins'][$plugin][$option] : $element['#default_value'];
        // Make the whole element visible only if the plugin is checked (enabled).
        $element['#states'] = array(
          'visible' => array(
            'input[name="settings[plugins][' . $plugin . '][enable]"]' => array('checked' => TRUE),
          ),
        );
        // Add the element to the FAPI structure.
        $form['settings']['plugins'][$plugin][$option] = $element;
      }
    }
  }

  // Playlist settings
  if (jw_player_use_legacy()) {
    $form['settings']['playlist_position'] = array(
      '#title' => t('Playlist position'),
      '#type' => 'select',
      '#description' => t('The position of the playlist'),
      '#default_value' => !empty($settings['playlist_position']) ? $settings['playlist_position'] : 'right',
      '#options' => array(
        'none' => t('None'),
        'bottom' => t('Bottom'),
        'top' => t('Top'),
        'right' => t('Right'),
        'left' => t('Left'),
        'over' => t('Over')
      ),
      '#states' => array(
        'visible' => array(
          ':input[name="settings[preset_source]"]' => array('value' => 'drupal'),
        ),
      ),
    );

    $form['settings']['playlist_size'] = array(
      '#title' => t('Playlist width'),
      '#type' => 'textfield',
      '#description' => t('The playlist width in pixels.'),
      '#default_value' => !empty($settings['playlist_size']) ? $settings['playlist_size'] : '180',
      '#field_suffix' => t('pixels'),
      '#size' => 10,
      '#states' => array(
        'visible' => array(
          ':input[name="settings[preset_source]"]' => array('value' => 'drupal'),
        ),
      ),
    );
  }

  $form['settings']['player_library_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Player Library URL'),
    '#description' => t('Enter the URL to the player created on JWPlayer.com that contains the settings for this preset. The global default can be set in the !admin, and overridden here.', array(
      '!admin' => l('JW Player general settings', 'admin/config/media/jw_player/settings'),
    )),
    '#default_value' => !empty($settings['player_library_url']) ? $settings['player_library_url'] : variable_get('jw_player_cloud_player_default', FALSE),
    '#states' => array(
      'required' => array(
        ':input[name="settings[preset_source]"]' => array('value' => 'jwplayer'),
      ),
      'visible' => array(
        ':input[name="settings[preset_source]"]' => array('value' => 'jwplayer'),
      ),
    ),
  );

  $form['settings']['#attached']['js'] = array(
    drupal_get_path('module', 'jw_player') . '/js/jw_player.admin.js',
  );
}

/**
 * Validator for jw_player_ctools_export_ui_form().
 */
function jw_player_ctools_export_ui_form_validate($form, &$form_state) {
  $settings = $form_state['values']['settings'];
  // If sharing is empty, remove selected options from sharing sites
  // field, and reset sharing heading text.
  if (array_key_exists('sharing', $settings) && $settings['sharing'] == 0) {
    $sharing = array();
    foreach ($settings['sharing_sites_show'] as $key => $value) {
      $sharing[$key] = 0;
    }
    form_set_value($form['settings']['sharing_sites_show'], $sharing, $form_state);
    form_set_value($form['settings']['sharing_heading'], t(''), $form_state);
  }

  // Use preset form if legacy or a version that is configured with local presets.
  if ($settings['preset_source'] == 'drupal') {
    if ($settings['responsive']) {
      // Width Percent Validation.
      if (empty($settings['width']) || !is_numeric($settings['width'])) {
        form_set_error('settings][width', t('Width field is required and must be a numeric value.'));
      }
      else if ($settings['width'] <= 0 || $settings['width'] > 100) {
        form_set_error('settings][width', t('Width field must be greater than 0% and no more than 100%.'));
      }
      // Aspect Ratio Validation.
      if (empty($settings['aspectratio'])) {
        form_set_error('settings][aspectratio', t('Aspect Ratio field is required if Responsive Design is enabled.'));
      }
      else if (!preg_match('/^([0-9]*):([0-9]*)$/', $settings['aspectratio'], $matches) ||
        (!is_numeric($matches[1]) || !is_numeric($matches[2]))) {
        form_set_error('settings][aspectratio', t('Aspect Ratio field must be of the format of two numbers separated by a colon. For example, <em>16:9</em>.'));
      }
    }
    else {
      // Width Validation.
      if (empty($settings['width']) || !is_numeric($settings['width'])) {
        form_set_error('settings][width', t('Width field is required and must be a numeric value.'));
      }
      else if ($settings['width'] <= 0) {
        form_set_error('settings][width', t('Width field must be greater than 0.'));
      }
      // Height Validation.
      if (empty($settings['height']) || !is_numeric($settings['height'])) {
        form_set_error('settings][height', t('Height field is required and must be a numeric value.'));
      }
      else if ($settings['height'] <= 0) {
        form_set_error('settings][height', t('Height field must be greater than 0.'));
      }
    }
  }
  else {
    preg_match(jw_player_library_url_regex(), $settings['player_library_url'], $matches);
    if (!isset($matches[2])) {
      form_set_error('settings][player_library_url', t('Player Library URL does not match format provided by JWPlayer.com.'));
    }
  }
}

/**
 * Submit function for jw_player_ctools_export_ui_form().
 */
function jw_player_ctools_export_ui_form_submit($form, &$form_state) {
  $settings = &$form_state['values']['settings'];
  // Remove unused values, so exports only have necessary items.
  if ($settings['responsive']) {
    unset($settings['height']);
  }
  else {
    unset($settings['aspectratio']);
  }
  if (!$settings['sharing']) {
    unset($settings['sharing_heading']);
    unset($settings['sharing_sites_show']);
    unset($settings['sharing_sites_order']);
  }
  if ($settings['preset_source'] == 'jwplayer') {
    foreach ($settings as $key => $value) {
      if ($key != 'preset_source' && $key != 'player_library_url') {
        unset($settings[$key]);
      }
    }
  }
  else {
    unset($settings['player_library_url']);
  }
}
