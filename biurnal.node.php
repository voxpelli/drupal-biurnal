<?php
// $Id$

function biurnal_node_info() {
  return array(
    'biurnal' => array(
      'name' => t('Biurnal configuration node'),
      'module' => 'biurnal',
      'description' => t("This is a configuration node for biurnal"),
    )
  );
}

function biurnal_access($op, $node) {
  global $user;

  if ($op == 'create') {
    return user_access('create biurnal node');
  }

  if ($op == 'update' || $op == 'delete') {
    if (user_access('edit all biurnal nodes') || (user_access('edit own biurnal node') && ($user->uid == $node->uid))) {
      return TRUE;
    }
  }
}

function biurnal_form(&$node) {
  global $_biurnal_;
  
  $type = node_get_types('type', $node);
  $base = drupal_get_path('module', 'biurnal');

  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => check_plain($type->title_label),
    '#required' => TRUE,
    '#default_value' => $node->title,
    '#weight' => -5
  );
  
  // Add Farbtastic color picker
  drupal_add_css('misc/farbtastic/farbtastic.css', 'module', 'all', FALSE);
  drupal_add_js('misc/farbtastic/farbtastic.js');
  
  // Add custom CSS/JS
  drupal_add_css($base .'/biurnal.css', 'module', 'all', FALSE);
  drupal_add_js($base .'/biurnal.js');
  
  $themes = list_themes();
  
  foreach($themes as $theme)
  {
    if ($theme->status && $_biurnal_->theme_is_biurnal($theme->name) &&
      function_exists('gd_info') && 
      variable_get('file_downloads', FILE_DOWNLOADS_PUBLIC) == FILE_DOWNLOADS_PUBLIC) 
    {
      $form['color'] = array(
        '#type' => 'fieldset',
        '#title' => t('Color scheme for %name', array('%name'=>$theme->name)),
        '#weight' => -1,
        '#attributes' => array('class' => 'biurnal_color_scheme_form'),
        '#theme' => 'biurnal_scheme_form',
      );
      $form['color'] += biurnal_scheme_form($node, $theme);
    }
  }

  return $form;
}

function biurnal_update(&$node) {
  biurnal_insert($node,True);
}

function biurnal_insert(&$node, $is_update=False) {
  global $_biurnal_;
  
  $themes = list_themes();
  $scheme = array();
  foreach ($themes as $theme) {
    if ($theme->status && $_biurnal_->theme_is_biurnal($theme->name)) {
      $palette = $_biurnal_->get_colors_for_theme($theme->name);
      foreach( $palette as $name => $value ) {
        $field_name = 'biurnal_color_'. $theme->name .'_'. $name;
        $scheme[$theme->name][$name] = $node->$field_name;
      }
    }
  }
  
  $configuration = serialize($scheme);
  if (!$is_update) {
    db_query("INSERT INTO {biurnal_configuration} (nid, configuration) VALUES (%d, '%s')", $node->nid, $configuration);
  }
  else {
    db_query("UPDATE {biurnal_configuration} AS bc SET bc.configuration = '%s' WHERE bc.nid = %d", $configuration, $node->nid);
  }
}

function biurnal_load(&$node) {
  global $_biurnal_;
  
  $bc = db_fetch_object(db_query('SELECT configuration FROM {biurnal_configuration} WHERE nid = %d', $node->nid));
  
  $biurnal = new stdClass();
  $biurnal->palette = unserialize($bc->configuration);

  if($_biurnal_->preview_scheme() == $node->nid) {
    $node->body = l(t('Stop previewing scheme'), 'biurnal/stop_preview');
  }
  else {
    $node->body = l(t('Preview scheme'), 'biurnal/preview/'. $node->nid);
  }
  
  return $biurnal;
}

function biurnal_scheme_form(&$node, $theme) {
  global $_biurnal_;

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
  
  $palette = $_biurnal_->get_colors_for_theme($theme->name, $node);
  foreach ($palette as $name => $value) {
    if(isset($node->palette[$theme->name][$name])) {
      $value = $node->palette[$theme->name][$name];
    }
    
    $form['palette']['biurnal_color_'. $theme->name .'_'. $name] = array(
      '#type' => 'textfield',
      '#title' => str_replace('_', ' ', isset($names[$name])?$names[$name]:$name),
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
    
    $form['backgrounds_'. $theme->name] = array(
      '#type' => 'select', 
      '#title' => t('Page background'), 
      '#default_value' => '',
      '#options' => $backgrounds,
      '#description' => t('Background for the page'),
    );
  }
  
  $form['theme'] = array('#type' => 'value', '#value' => $theme->name);
  $form['info'] = array('#type' => 'value', '#value' => $info);

  return $form;
}

/**
 * Theme color form.
 */
function theme_biurnal_scheme_form($form) {
  // Include stylesheet
  $theme = $form['theme']['#value'];
  $info = $form['info']['#value'];
  $path = drupal_get_path('theme', $theme) .'/';

  // Wrapper
  $output .= '<div class="biurnal-form clear-block">';
  
  // Palette
  $output .= '<div class="palette">';
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