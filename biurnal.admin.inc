<?php
// $Id$

/**
 * @file
 * Admin callbacks for Birunal
 */

function _biurnal_admin_index() {
  return 'Blabbo';
}

/**
 * Get the cached changes to a given task handler.
 */
function _biurnal_conf_get_conf_cache($name) {
  ctools_include('object-cache');
  $cache = ctools_object_cache_get('biurnal_conf', $name);
  if (!$cache) {
    $cache = biurnal_conf_load($name);
    if ($cache) {
      $cache->locked = ctools_object_cache_test('biurnal_conf', $name);
    }
  }

  return $cache;
}

/**
 * Store changes to a task handler in the object cache.
 */
function _biurnal_conf_set_conf_cache($name, $conf) {
  ctools_include('object-cache');
  $cache = ctools_object_cache_set('biurnal_conf', $name, $conf);
}

/**
 * Remove an item from the object cache.
 */
function _biurnal_conf_clear_conf_cache($name) {
  ctools_include('object-cache');
  ctools_object_cache_clear('biurnal_conf', $name);
}


function biurnal_conf_form_add($form_state) {
  $base    = drupal_get_path('module', 'biurnal');
  $schemes = biurnal_get_schemes();
  $options = array();

  foreach ($schemes as $key => $info) {
    $options[$key] = $info['name'];
  }

  $form['name'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Name'),
    '#default_value' => $node->title,
    '#size'          => 32,
    '#maxlength'     => 32,
    '#required'      => TRUE,
  );

  $form['scheme'] = array(
    '#type'          => 'select',
    '#title'         => t('Scheme'),
    '#options'       => $options,
    '#required'      => TRUE,
  );

  $form['submit'] = array('#type' => 'submit', '#value' => t('Add'));

  return $form;
}

function biurnal_conf_form_add_validate($form, &$form_state) {
  $name = $form_state['values']['name'];

  // Configuration name must be alphanumeric or underscores, no other punctuation.
  if (preg_match('/[^a-zA-Z0-9_]/', $name)) {
    form_error($form['name'], t('Configuration name must be alphanumeric or underscores only.'));
  }

  // Configuration name must not already exist.
  if (biurnal_conf_load($name)) {
    form_error($form['name'], t('You must use a unique name for this configuration.'));
  }

  // Scheme must exist
  if (!biurnal_get_scheme($form_state['values']['scheme'])) {
    form_error($form['scheme'], t('Invalid scheme.'));
  }
}

function biurnal_conf_form_add_submit($form, &$form_state) {
  $conf = biurnal_conf_new();
  $conf->name   = $form_state['values']['name'];
  $conf->tag    = $form_state['values']['tag'];
  $conf->scheme = $form_state['values']['scheme'];

  _biurnal_conf_set_conf_cache($conf->name, $conf);
  $form_state['redirect'] ='admin/build/biurnal/edit/' . $conf->name;
}

function biurnal_conf_form_edit($form_state, $name) {
  $conf    = _biurnal_conf_get_conf_cache($name);

  if (empty($conf)) {
    drupal_set_message(t('Invalid object name.'), 'error');
    return array();
  }

  $biurnal = Biurnal::get_instance();
  $scheme  = biurnal_get_scheme($conf->scheme);
  $base    = drupal_get_path('module', 'biurnal');

  // Add palette fields
  $names = array(
    'main'       => t('Base color'),
    'background' => t('Background color'),
    'link'       => t('Link color'),
    'top'        => t('Header top'),
    'bottom'     => t('Header bottom'),
    'text'       => t('Text color')
  );

  if (is_array($scheme['colors'])) {
    $palette = $scheme['colors'];

    $form['palette'] = array(
      '#type'        => 'fieldset',
      '#title'       => t('Color configuration'),
      '#weight'      => -5,
      '#collapsible' => FALSE,
      '#collapsed'   => FALSE,
      '#tree'        => TRUE,
      '#attributes'    => array('class' => 'form-biurnal-colors'),
    );

    foreach ($palette as $name => $value) {
      if (isset($node->palette[$theme->name][$name])) {
        $value = $node->palette[$theme->name][$name];
      }
      $form['palette'][$name] = array(
        '#type'          => 'textfield',
        '#title'         => str_replace('_', ' ', isset($names[$name]) ? $names[$name] : $name),
        '#default_value' => '#' . $value,
        '#maxlength'     => 7,
        '#size'          => 7,
        '#attributes'    => array('class' => 'form-biurnal-color'),
      );
    }

    // Add Farbtastic color picker
    drupal_add_css('misc/farbtastic/farbtastic.css', 'module', 'all', FALSE);
    drupal_add_js('misc/farbtastic/farbtastic.js');

    // Add custom CSS/JS
    drupal_add_css($base .'/biurnal.css', 'module', 'all', FALSE);
    drupal_add_js($base  .'/biurnal.js');
  }

  $bg_dir = $biurnal->biurnal_path . '/backgrounds';
  $backgrounds = array('' => t('Select a background'));
  if (is_dir($bg_dir)) {
    if ($dh = opendir($bg_dir)) {
      while (($file = readdir($dh)) !== FALSE) {
        if (drupal_substr($file, 0, 1) != '.') {
          $backgrounds[basename($file)] = basename($file);
        }
      }
      closedir($dh);
    }

    if (count($backgrounds) > 1) {
      $form['backgrounds'] = array(
        '#type'          => 'select',
        '#title'         => t('Page background'),
        '#default_value' => '',
        '#options'       => $backgrounds,
        '#description'   => t('Background for the page'),
      );
    }
  }

  $form['submit'] = array('#type' => 'submit', '#value' => t('Save'));

  return $form;
}

function biurnal_conf_form_edit_validate($form, &$form_state) {

}

function biurnal_conf_form_edit_submit($form, &$form_state) {
  
}

//STUFF BELOW HERE IS OLD

/*function biurnal_scheme_form_submit($form, &$form_state) {
  global $_biurnal_;
  $palette = $_biurnal_->get_colors_for_theme(arg(4));
  foreach ($palette as $name => $value) {
    $palette[$name]=$form_state['values']['biurnal_color_' . $name];
  }

  $_biurnal_->set_colors_for_theme(arg(4), $palette);
  module_invoke_all('biurnal_scheme_saved', arg(4), $palette);
}*/

/**
 * Theme color form.
 */
/*function theme_biurnal_scheme_form($form) {
  // Include stylesheet
  $theme = $form['theme']['#value'];
  $info  = $form['info']['#value'];
  $path  = drupal_get_path('theme', $theme) . '/';

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
}*/