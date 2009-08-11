<?php
// $Id$

/**
 * @file
 * Provides actions for the Biurnal module
 */

//TODO: Check that these actions works nicely with Drupal 6.x

function biurnal_action_info() {
  return array(
    'biurnal_set_color_scheme_action' => array(
      'description' => t('Change the color scheme of the site'),
      'type' => 'system',
      'configurable' => True,
      'hooks' => array()
    ),
  );
}

function biurnal_set_color_scheme_action($context) {
}

function biurnal_set_color_scheme_action_form($context) {
  $form = array();
  //TODO: Translate these two schemes?
  $options = array(
    'scheme_one' => 'Färgschema ett',
    'scheme_two' => 'Färgschema två',
  );
  $form['theme_selector'] = array(
    '#type'          => 'select',
    '#title'         => t('Color theme'),
    '#multiple'      => FALSE,
    '#default_value' => $context['theme_selector'],
    '#options'       => $options,
    '#required'      => TRUE,
    '#description'   => t('Choose a color theme')
  );
  return $form;
}

//TODO: This function needs to be fixed
function biurnal_set_color_scheme_action_submit($form_id, $form_values) {
  $params = array(
    'theme_selector' => $form_values['theme_selector'],
  );
  return $params;
}