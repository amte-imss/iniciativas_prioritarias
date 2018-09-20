<?php

/**
 * @file
 * jw_player.theme.inc
 *
 * JW Player theme functions.
 */

/**
 * Returns HTML for the sharing sites order form.
 *
 * Copied from the core theme_filter_admin_format_filter_order() function.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: A render element representing the form.
 *
 * @ingroup themeable
 */
function theme_jw_player_sharing_sites_order($variables) {
  $element = $variables['element'];

  // JW Player sharing sites order (tabledrag).
  $rows = array();
  foreach (element_children($element, TRUE) as $name) {
    $element[$name]['weight']['#attributes']['class'][] = 'jw-player-sharing-sites-order-weight';
    $rows[] = array(
      'data' => array(
        drupal_render($element[$name]['content']),
        drupal_render($element[$name]['weight']),
      ),
      'class' => array('draggable'),
    );
  }

  $output = drupal_render_children($element);
  $output .= theme('table', array(
    'rows' => $rows,
    'attributes' => array('id' => 'jw-player-sharing-sites-order')
  ));
  drupal_add_tabledrag('jw-player-sharing-sites-order', 'order', 'sibling', 'jw-player-sharing-sites-order-weight', NULL, NULL, TRUE);

  return $output;
}
