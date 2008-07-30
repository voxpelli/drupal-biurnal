<?php
// $Id$

/**
 * Implementation of hook_form_alter().
 */
function biurnal_form_alter($form_id, &$form) 
{
  global $_biurnal_;
  
  // Insert the color changer into the theme settings page.
  // TODO: Last condition in the following if disables color changer when private files are used this should be solved in a different way. See issue #92059.
  if ($form_id == 'system_theme_settings' && 
    $_biurnal_->theme_is_biurnal(arg(4)) &&
    function_exists('gd_info') && 
    variable_get('file_downloads', FILE_DOWNLOADS_PUBLIC) == FILE_DOWNLOADS_PUBLIC) 
  {
    $form['color'] = array(
      '#type' => 'fieldset',
      '#title' => t('Biurnal color scheme'),
      '#weight' => -1,
      '#attributes' => array('id' => 'biurnal_color_scheme_form'),
      '#theme' => 'biurnal_scheme_form',
    );
    $form['color'] += biurnal_scheme_form(arg(4));
    $form['#submit']['biurnal_scheme_form_submit'] = array();
  }
}

function biurnal_scheme_form($theme) 
{
  global $_biurnal_;
  $base = drupal_get_path('module', 'biurnal');

  // Add Farbtastic color picker
  drupal_add_css('misc/farbtastic/farbtastic.css', 'module', 'all', FALSE);
  drupal_add_js('misc/farbtastic/farbtastic.js');

  // Add custom CSS/JS
  drupal_add_css($base .'/biurnal.css', 'module', 'all', FALSE);
  drupal_add_js($base .'/biurnal.js');

  // Add palette fields
  $names = array(
    'main' => t('Base color'),
    'background' => t('Background color'),
    'link' => t('Link color'),
    'top' => t('Header top'),
    'bottom' => t('Header bottom'),
    'text' => t('Text color')
  );
  $form['biurnal']['#tree'] = true;
  
  $palette = $_biurnal_->get_colors_for_theme(arg(4));
  foreach ($palette as $name => $value) {
    $form['palette']['biurnal_color_'.$name] = array(
      '#type' => 'textfield',
      '#title' => isset($names[$name])?$names[$name]:$name,
      '#default_value' => $value,
      '#size' => 8,
    );
  }
  
  $bg_dir = $_biurnal_->biurnal_path . '/backgrounds';
  $backgrounds = array(''=>t('Select a background'));
  if (is_dir($bg_dir)) 
  {
    if ($dh = opendir($bg_dir)) 
    {
      while (($file = readdir($dh)) !== false) 
      {
        if(substr($file,0,1)!='.')
          $backgrounds[basename($file)]=basename($file);
      }
      closedir($dh);
    }
    
    $form['backgrounds'] = array(
      '#type' => 'select', 
      '#title' => t('Page background'), 
      '#default_value' => '',
      '#options' => $backgrounds,
      '#description' => t('Background for the page'),
    );
  }
  
  $form['theme'] = array('#type' => 'value', '#value' => arg(4));
  $form['info'] = array('#type' => 'value', '#value' => $info);

  return $form;
}

function biurnal_scheme_form_submit($form_id, $values) 
{
  global $_biurnal_;
  $palette = $_biurnal_->get_colors_for_theme(arg(4));
  foreach($palette as $name => $value)
    $palette[$name]=$values['biurnal_color_'.$name];
    
  $_biurnal_->set_colors_for_theme(arg(4),$palette);
  module_invoke_all('biurnal_scheme_saved',arg(4),$palette);
}

/**
 * Theme color form.
 */
function theme_biurnal_scheme_form($form) 
{
  // Include stylesheet
  $theme = $form['theme']['#value'];
  $info = $form['info']['#value'];
  $path = drupal_get_path('theme', $theme) .'/';

  // Wrapper
  $output .= '<div class="biurnal-form clear-block">';
  
  // Palette
  $output .= '<div id="palette" class="clear-block">';
  foreach (element_children($form['palette']) as $name) {
    $output .= drupal_render($form['palette'][$name]);
  }
  $output .= '</div>';

  // Preview
  $output .= drupal_render($form);
  
  // Close wrapper
  $output .= '</div>';

  return $output;
}