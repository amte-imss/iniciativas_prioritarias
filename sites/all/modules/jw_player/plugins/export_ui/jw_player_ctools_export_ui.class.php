<?php

/**
 * CTools export UI extending class. Slightly customized for JW Player.
 */
class jw_player_ctools_export_ui extends ctools_export_ui {

  /**
   * {@inheritdoc}
   */
  function list_build_row($item, &$form_state, $operations) {
    // Display the human-readable version of the preset name.
    $name = $item->preset_name;
    // Display description saved with preset.
    $description = $item->description;
    // Access the settings summary information for this preset.
    $preset_settings = jw_player_preset_settings($item->{$this->plugin['export']['key']});
    // Unset name and description from the array since they are already
    // displayed in the first two table columns.
    unset($preset_settings['name']);
    unset($preset_settings['description']);
    // Display preset settings in a line-break format.
    $settings = implode('<br />', $preset_settings);
    $schema = ctools_export_get_schema($this->plugin['schema']);

    // Note: $item->{$schema['export']['export type string']} should have
    // already been set up by export.inc, so we can use it safely.
    switch ($form_state['values']['order']) {
      case 'disabled':
        $this->sorts[$name] = empty($item->disabled) . $name;
        break;
      case 'title':
        $this->sorts[$name] = $item->{$this->plugin['export']['admin_title']};
        break;
      case 'name':
        $this->sorts[$name] = $name;
        break;
      case 'description':
        $this->sorts[$name] = $description;
        break;
      case 'settings':
        $this->sorts[$name] = $settings;
        break;
      case 'storage':
        $this->sorts[$name] = $item->{$schema['export']['export type string']} . $name;
        break;
    }

    $this->rows[$name]['data'] = array();
    $this->rows[$name]['class'] = !empty($item->disabled) ? array('ctools-export-ui-disabled') : array('ctools-export-ui-enabled');

    // If we have an admin title, make it the first row.
    if (!empty($this->plugin['export']['admin_title'])) {
      $this->rows[$name]['data'][] = array('data' => check_plain($item->{$this->plugin['export']['admin_title']}), 'class' => array('ctools-export-ui-title'));
    }
    // Add preset name, description, and settings as table columns.
    $this->rows[$name]['data'][] = array('data' => check_plain($name), 'class' => array('ctools-export-ui-name'));
    $this->rows[$name]['data'][] = array('data' => check_plain($description), 'class' => array('ctools-export-ui-description'));
    $this->rows[$name]['data'][] = array('data' => $settings, 'class' => array('ctools-export-ui-settings'));
    $this->rows[$name]['data'][] = array('data' => check_plain($item->{$schema['export']['export type string']}), 'class' => array('ctools-export-ui-storage'));

    $ops = theme('links__ctools_dropbutton', array('links' => $operations, 'attributes' => array('class' => array('links', 'inline'))));

    $this->rows[$name]['data'][] = array('data' => $ops, 'class' => array('ctools-export-ui-operations'));

    // Add an automatic mouseover of the description if one exists.
    if (!empty($this->plugin['export']['admin_description'])) {
      $this->rows[$name]['title'] = $item->{$this->plugin['export']['admin_description']};
    }
  }

  /**
   * {@inheritdoc}
   */
  function list_table_header() {
    $header = array();
    if (!empty($this->plugin['export']['admin_title'])) {
      $header[] = array('data' => t('Title'), 'class' => array('ctools-export-ui-title'));
    }

    // Include column headers for preset name, description, and settings.
    $header[] = array('data' => t('Name'), 'class' => array('ctools-export-ui-name'));
    $header[] = array('data' => t('Description'), 'class' => array('ctools-export-ui-description'));
    $header[] = array('data' => t('Settings'), 'class' => array('ctools-export-ui-settings'));
    $header[] = array('data' => t('Storage'), 'class' => array('ctools-export-ui-storage'));
    $header[] = array('data' => t('Operations'), 'class' => array('ctools-export-ui-operations'));

    return $header;
  }

}
