<?php
// $Id$

function biurnal_action_info()
{
  return array(
    'biurnal_set_color_scheme_action' => array(
      'description' => t('Change the color scheme of the site'),
      'type' => 'system',
      'configurable' => True,
      'hooks' => array()
    ),
  );
}

function biurnal_set_color_scheme_action($context)
{
  
}

function biurnal_set_color_scheme_action_form($context)
{
  $form = array();
  $options = array('scheme_one'=>'Färgschema ett','scheme_two'=>'Färgschema två');
  $form['theme_selector'] = array(
    '#type' => 'select',
    '#title' => t("Color theme"),
    '#multiple' => False,
    '#default_value' => $context['theme_selector'],
    '#options' => $options,
    '#required' => TRUE,
    '#description' => t('Choose a color theme')
  );
  return $form;
}

function biurnal_set_color_scheme_action_submit($form_id,$form_values)
{
  $params = array(
    'theme_selector' => $form_values['theme_selector'],
  );
  return $params;
}