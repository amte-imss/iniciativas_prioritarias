<?php

/**
 * @file
 * Administrative part.
 */

/**
 * Settings for connect to LRS.
 */
function h5p_scorm_admin_settings($form, &$form_state) {
  $form = array();

  $form['lrs'] = array(
    '#type' => 'fieldset',
    '#title' => t('LRS settings'),
    '#description' => t('LRS settings for tincan packages'),
  );

  $form['lrs']['h5p_scorm_lrs_endpoint'] = array(
    '#type' => 'textfield',
    '#title' => t('Endpoint'),
    '#description' => t('The server endpoint. Do not include a trailing slash.'),
    '#default_value' => variable_get('h5p_scorm_lrs_endpoint', ''),
    '#required' => TRUE,
  );

  $form['lrs']['h5p_scorm_lrs_auth_user'] = array(
    '#type' => 'textfield',
    '#title' => t('User'),
    '#description' => t('The basic authenication user.'),
    '#default_value' => variable_get('h5p_scorm_lrs_auth_user', ''),
    '#required' => TRUE,
  );

  $form['lrs']['h5p_scorm_lrs_auth_password'] = array(
    '#type' => 'textfield',
    '#title' => t('Password'),
    '#description' => t('The basic authenication password.'),
    '#default_value' => variable_get('h5p_scorm_lrs_auth_password', ''),
    '#required' => TRUE,
  );

  return system_settings_form($form);
}
