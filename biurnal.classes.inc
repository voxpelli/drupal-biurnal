<?php
// $Id$

class Biurnal {
  private $images;
  private $colors;
  private $css;
  private $processes;
  private $filters;
  private $operations;

  private static $instance;

  // block directly instantiating
  private function __construct() {
    module_load_include('hsb.inc', 'biurnal');

    $this->images     = array();
    $this->colors     = array();
    $this->css        = array();
    $this->processes  = array();
    $this->filters    = array();
    $this->operations = array();

    $this->filters['mix']               = array($this, 'mix_colors');
    $this->filters['shift_hue']         = 'biurnal_shift_hue';
    $this->filters['brightness']        = 'biurnal_set_brightness';
    $this->filters['saturation']        = 'biurnal_set_saturation';
    $this->filters['brightness_and_saturation'] = 'biurnal_set_brightness_and_saturation';
    $this->filters['match_brightness']  = 'biurnal_match_brightness';
    $this->filters['match_saturation']  = 'biurnal_match_saturation';
    $this->filters['match']             = 'biurnal_match_saturation_and_brightness';

    $this->operations['clear']          = array($this, 'clear_to_color');
    $this->operations['clear_gradient'] = array($this, 'clear_to_gradient');
    $this->operations['mask']           = array($this, 'mask');
  }

  private function __clone() { } // block cloning of the object

  // we love singletons - at least in this case
  public static function get_instance() {
     if (!isset(self::$instance)) {
        self::$instance = new Biurnal();
     }
     return self::$instance;
  }

  public function init() {
    $this->biurnal_path = file_directory_path() . '/biurnal';
    if (!is_dir($this->biurnal_path)) {
      mkdir($this->biurnal_path);
    }

    $this->biurnal_backgrounds = $this->biurnal_path . '/backgrounds';
    if (!is_dir($this->biurnal_backgrounds)) {
      mkdir($this->biurnal_backgrounds);
    }
  }

  public function resolve_path($path, $base) {
    if (drupal_substr($path, 0, 1) != '/') {
      $path = '/'. $path;
    }
    $path = realpath('./' . $base . $path);
    return $path;
  }

  public function load_json($json, $base_path = NULL, $is_file = TRUE) {
    $content = $json;
    if ($is_file) {
      if (is_file($json)) {
        $content = file_get_contents($json);
      }
      else {
        return;
      }
      if (!isset($base_path)) {
        $base_path = dirname($json);
      }
    }

    $data = json_decode($content, TRUE);
    if (isset($data)) {
      foreach ($data as $key => $values) {
        switch ($key) {
          case 'colors':
            foreach ($values as $name => $color) {
              $this->set_color($name, intval($color, 16));
            }
            break;
          case 'css':
            foreach ($values as $path) {
              $resolved_path = $this->resolve_path($path, $base_path);
              if ($resolved_path) {
                $this->add_css($resolved_path);
              }
              else {
                drupal_set_message(t('Biurnal could not find the css file "%path"',
                  array(
                    '%path' => $path,
                  )), 'error');
              }
            }
            break;
          case 'images':
            foreach ($values as $dir => $images) {
              $i = 0;
              foreach ($images as $image) {
                $i++;
                $path = $this->resolve_path($image['path'], $base_path);
                $properties = isset($image['properties'])?$image['properties']:array();
                $properties['base_path'] = $base_path;
                if ($path) {
                  $this->add_image($path, $image['alter'], $dir, $properties);
                }
                else {
                  drupal_set_message(t('Biurnal could not find the image file "%path", number %current of %count in "%dir"', array(
                    '%path'    => $image['path'],
                    '%current' => $i,
                    '%count'   => count($images),
                    '%dir'     => $dir,
                  )), 'error');
                }
              }
            }
            break;
          case 'processes':
            foreach ($values as $name => $process) {
              $this->add_process($name, $process);
            }
            break;
        }
      }
    }
    else {
      drupal_set_message(t('Could not parse biurnal json data'), 'error');
    }
  }

  public function mix_colors($a, $b, $proportions = 0.5) {
    list($r1, $g1, $b1) = $a;
    list($r2, $g2, $b2) = $b;
    $r = ($r2 - $r1 != 0) ? intval($r1 + ($r2 - $r1) * $proportions): $r1;
    $g = ($g2 - $g1 != 0) ? intval($g1 + ($g2 - $g1) * $proportions): $g1;
    $b = ($b2 - $b1 != 0) ? intval($b1 + ($b2 - $b1) * $proportions): $b1;
    return array($r, $g, $b);
  }

  public function add_colors($a, $b, $proportions = 0.5) {
    list($r1, $g1, $b1) = $a;
    list($r2, $g2, $b2) = $b;
    $c = array($r1+$r2, $g1+$g2, $b1+$b2);

    return $c;
  }

  private function image_reference($reference, $obj) {
    $matches = array();
    if (preg_match('/^\$([^\/])\/(.+)$/', $reference, $matches)) {
      return $this->scheme_path . '/' . $matches[1] . '/' . $matches[2];
    }
    else {
      return $this->resolve_path($reference, $obj['properties']['base_path']);
    }
  }

  public function mask($obj, $image, $mask_image) {
    $path = $this->image_reference($mask_image, $obj);
    if (file_exists($path)) {
      $mask = imagecreatefrompng($path);

      if ($mask) {
        $width     = min(imagesx($mask), imagesx($image));
        $height    = min(imagesy($mask), imagesy($image));
        $composite = imagecreatetruecolor($width, $height);
        imagesavealpha($composite, TRUE);
        $trans_colour = imagecolorallocatealpha($composite, 0, 0, 0, 127);
        imagefill($composite, 0, 0, $trans_colour);

        for ($x = 0; $x < $width; $x++) {
          for ($y = 0; $y < $height; $y++) {
            //Extract red channel from mask pixel
            $mask_rgb   = imagecolorat($mask, $x, $y);
            $mask_alpha = max(127 - (($mask_rgb & 0xFF0000) >> 16) / 2, 0);

            //Extract color channels from image
            $rgb   = imagecolorat($image, $x, $y);
            $red   = ($rgb & 0xFF0000) >> 16;
            $green = ($rgb & 0x00FF00) >> 8;
            $blue  = ($rgb & 0x0000FF);

            $color = imagecolorallocatealpha($composite, $red, $green, $blue, $mask_alpha);
            imagesetpixel($composite, $x, $y, $color);
          }
        }

        imagedestroy($image);
        return $composite;
      }
    }
    return $image;
  }

  public function copy_preserve_alpha($obj, $image, $color=Null) {
    $width      = imagesx($image);
    $height     = imagesy($image);
    $background = imagecreatetruecolor($width, $height);

    $bg_color = -1;
    if (isset($color)) {
      $bg_color = imagecolorallocate($background, $color[0], $color[1], $color[2]);
    }
    else {
      $bg_color = imagecolorallocatealpha($background, 0, 0, 0, 0);
    }

    imagefilledrectangle($background, 0, 0, $width, $height, $bg_color);
    imagecopy($background, $image, 0, 0, 0, 0, $width, $height);

    for ($x = 0; $x < $width; $x++) {
      for ($y = 0; $y < $width; $y++) {
        $b_rgba = imagecolorat($background, $x, $y);
        $i_rgba = imagecolorat($image, $x, $y);

        $alpha = ($i_rgba & 0x7F000000) >> 24;
        $red   = ($b_rgba & 0xFF0000) >> 16;
        $green = ($b_rgba & 0x00FF00) >> 8;
        $blue  = ($b_rgba & 0x0000FF);

        $c = imagecolorallocatealpha($image, $red, $green, $blue, $alpha);
        imagesetpixel($image, $x, $y, $c);
      }
    }
    imagedestroy($background);

    return $image;
  }

  public function clear_to_color($obj, $image, $color = NULL) {
    $width  = imagesx($image);
    $height = imagesy($image);
    imagedestroy($image);
    $background = imagecreatetruecolor($width, $height);

    $bg_color = -1;
    if (isset($color)) {
      $bg_color = imagecolorallocate($background, $color[0], $color[1], $color[2]);
    }
    else {
      $bg_color = imagecolorallocatealpha($background, 0, 0, 0, 0);
    }

    imagefilledrectangle($background, 0, 0, $width, $height, $bg_color);

    return $background;
  }

  public function clear_to_gradient($obj, $image, $color_a, $color_b, $direction="vertical") {
    list($r1, $g1, $b1) = $color_a;
    list($r2, $g2, $b2) = $color_b;

    $width  = imagesx($image);
    $height = imagesy($image);
    imagedestroy($image);

    $background = imagecreatetruecolor($width, $height);

    for ($i=0; $i<$height; $i++) {
      $r = ( $r2 - $r1 != 0 ) ? intval( $r1 + ( $r2 - $r1 ) * ($i/$height) ): $r1;
      $g = ( $g2 - $g1 != 0 ) ? intval( $g1 + ( $g2 - $g1 ) * ($i/$height) ): $g1;
      $b = ( $b2 - $b1 != 0 ) ? intval( $b1 + ( $b2 - $b1 ) * ($i/$height) ): $b1;

      imagefilledrectangle($background, 0, $i, $width-1, $i, imagecolorallocate($background, $r, $g, $b));
    }

    return $background;
  }

  private function resolve_params($params) {
    $args = array();

    if (!isset($params)) {
      return $args;
    }

    foreach ((array)$params as $param) {
      if (is_string($param) && drupal_substr($param, 0, 1) == '$') {
        $args[] = $this->color(drupal_substr($param, 1));
      }
      elseif (is_string($param) && drupal_substr($param, 0, 2) == "0x") {
        $args[] = $this->intrgb(intval(drupal_substr($param, 2), 16));
      }
      elseif (is_string($param) && drupal_substr($param, 0, 2) == "##") {
        $args[] = $this->to_hsl_triplet(drupal_substr($param, 2));
      }
      elseif (is_string($param) && drupal_substr($param, 0, 1) == "#") {
        $args[] = $this->intrgb(intval(drupal_substr($param, 1), 16));
      }
      elseif (is_array($param)) {
        foreach ($param as $filter => $fparams) {
          $args[] = $this->run_filter($filter, $fparams);
          break;
        }
      }
      else {
        $args[] = $param;
      }
    }
    return $args;
  }

  private function run_filter($name, $params) {
    if (!isset($this->filters[$name])) {
      return NULL;
    }
    $args = $this->resolve_params($params);
    $res  = call_user_func_array($this->filters[$name], $args);
    return $res;
  }

  private function scheme_identifier() {
    if (module_exists('pathauto') && function_exists('pathauto_cleanstring')) {
      return pathauto_cleanstring(mb_convert_case($this->scheme->title, MB_CASE_LOWER));
    }
    else {
      return $this->scheme->nid;
    }
  }

  public static function theme_head() {
    global $_biurnal_;
    $vars = array();
    $_biurnal_->preprocess($vars, 'page');
    return $vars['biurnal'];
  }

  public function preprocess(&$vars) {
    //Get theme name and json
    $theme_name = basename(path_to_theme());
    $this->biurnal_theme_path = path_to_theme() . '/biurnal';
    if (is_dir($this->biurnal_theme_path)) {
      $this->load_json($this->biurnal_theme_path . '/biurnal.json');
    }

    if (function_exists('path_to_subtheme')) {
      $theme_name = basename(path_to_subtheme());
      $this->biurnal_theme_path = path_to_subtheme() . '/biurnal';
      if (is_dir($this->biurnal_theme_path)) {
        $this->load_json($this->biurnal_theme_path . '/biurnal.json');
      }
    }

    //Allow the current node to override the theme in use
    if (isset($vars['node']) && $vars['node']->type == 'biurnal') {
      $this->scheme = $vars['node'];
    }
    else {
      //Get the active biurnal node
      $this->scheme = $this->get_scheme();
    }

    //Create a mock-scheme with the default values so that we never get a blank page
    if (!isset($this->scheme) || !$this->scheme) {
      $this->scheme = (object) array('nid' => 0, 'title' => 'default', 'changed' => 0);
    }
    else { //Read the colors from the biurnal node
      if (isset($this->scheme->palette[$theme_name])) {
        $palette = $this->scheme->palette[$theme_name];
        if (is_array($palette)) {
          foreach ($palette as $name => $color) {
            $this->update_color($name, intval(drupal_substr($color, 1), 16));
          }
        }
      }
    }

    //Check if the needs to be created or recreated
    $this->scheme_path = sprintf('%s/%s/%s', $this->biurnal_path, $theme_name, $this->scheme_identifier());
    if (!is_dir($this->scheme_path) || $this->scheme->changed > filemtime($this->scheme_path . '/touch.txt')) {
      //Delete old files
      if (is_dir($this->scheme_path)) {
        _biurnal_rm_recursive($this->scheme_path);
      }

      //Create new scheme files
      mkdir($this->scheme_path, 0777, TRUE);
      file_put_contents($this->scheme_path . '/touch.txt', format_date(time()));
      $this->scheme_path .= '/' . $this->scheme->changed;
      mkdir($this->scheme_path, 0777, TRUE);
      $this->process_scheme_resources();
    }
    else {
      $this->scheme_path .= '/' . $this->scheme->changed;
    }

    foreach ($this->colors as $name => $color) {
      $vars['colors'][$name] = $this->color_to_hex($color);
      $vars['colors'][$name . '_triplet'] = $color;
    }

    foreach ($this->css as $css) {
      $name = basename($css);
      $vars['biurnal'] .= '<link rel="stylesheet" href="/' . $this->scheme_path . '/' . $name . '" type="text/css" />' . "\n";
      $vars['biurnal'] .= '<!--[if lte IE 7]><link rel="stylesheet" href="/' . $this->scheme_path . '/ie.' . $name . '" type="text/css" /><![endif]-->' . "\n";
    }
  }

  private function process_scheme_resources() {
    //Create new images
    foreach ($this->images as $dir_name => $images) {
      $dir = $this->scheme_path . '/' . $dir_name;
      if (!is_dir($dir)) {
        mkdir($dir);
      }

      foreach ($images as $image) {
        list($width, $height) = getimagesize($image['path']);
        $img = imagecreatefrompng($image['path']);

        //Transparent background
        $back = imagecreatetruecolor($width, $height);
        imagesavealpha($back, TRUE);
        $trans_colour = imagecolorallocatealpha($back, 0, 0, 0, 127);
        imagefill($back, 0, 0, $trans_colour);

        $processes = NULL;
        if (isset($image['alter'])) {
          $processes = array();
          foreach ($image['alter'] as $p) {
            if (is_array($p)) {
              $processes[] = $p;
            }
            elseif (is_string($p) && isset($this->processes[$p])) {
              $processes[] = $this->processes[$p];
            }
          }

          //Run foreground and background processes
          foreach ($processes as $process) {
            if (isset($process['background'])) {
              $back = $this->apply_operations($image, $back, $process['background']);
            }
            if (isset($process['foreground'])) {
              $img = $this->apply_operations($image, $img, $process['foreground']);
            }
          }
        }

        imagecopy($back, $img, 0, 0, 0, 0, $width, $height);
        imagedestroy($img);

        //Run postpreocess operations
        if (isset($processes)) {
          foreach ($processes as $process) {
            if (isset($process['postprocess'])) {
              $back = $this->apply_operations($image, $back, $process['postprocess']);
            }
          }
        }

        $file = $dir . '/' . basename($image['path']);
        if (file_exists($file)) {
          unlink($file);
        }

        imagepng($back, $file);
        imagedestroy($back);
      }
    }

    //Process backgrounds
    if (is_dir($this->biurnal_backgrounds) && isset($this->processes['page_background_process'])) {
      $bg_dir = $this->scheme_path . '/backgrounds';
      if (!is_dir($bg_dir)) {
        mkdir($bg_dir);
      }

      $bg_process = $this->processes['page_background_process'];
      if ($dh = opendir($this->biurnal_backgrounds)) {
        while (($file = readdir($dh)) !== FALSE) {
          if (drupal_substr($file, 0, 1) != '.') {
            list($width, $height) = getimagesize($this->biurnal_backgrounds . '/' . $file);
            $img  = imagecreatefrompng($this->biurnal_backgrounds . '/' . $file);
            $back = imagecreatetruecolor($width, $height);

            if (isset($bg_process['background'])) {
              $back = $this->apply_operations($back, $bg_process['background']);
            }
            if (isset($bg_process['front'])) {
              $img = $this->apply_operations($img, $bg_process['front']);
            }

            imagecopy($back, $img, 0, 0, 0, 0, $width, $height);
            imagejpeg($back, $bg_dir . '/' . basename($file, '.png') . '.jpg', 80);
          }
        }
        closedir($dh);
      }
    }

    //Process css files
    foreach ($this->css as $css) {
      $name = basename($css);
      $content = file_get_contents($css);
      $content = preg_replace_callback('/\$(\{.*\})\$/', array($this, 'filter_callback'), $content);
      $content = preg_replace_callback('/\$([_a-z]+)/', array($this, 'color_callback'), $content);
      file_put_contents($this->scheme_path . '/ie.' . $name, $content);

      $content = preg_replace_callback('/url\(([^\)]+)\)/', array($this, 'data_url_callback'), $content);
      file_put_contents($this->scheme_path . '/' . $name, $content);
    }
  }

  private function data_url_callback($matches) {
    $path = $this->scheme_path . '/' . trim($matches[1]);
    if (file_exists($path) && filesize($path) < 800) {
      return 'url(data:image/png;base64,' . base64_encode(file_get_contents($path)) . ')';
    }
    return $matches[0];
  }

  private function filter_callback($matches) {
    $filter_def = json_decode($matches[1], TRUE);
    if ($filter_def) {
      foreach ($filter_def as $filter => $fparams) {
        $val = $this->run_filter($filter, $fparams);
        if (is_array($val)) {
          return $this->color_to_hex($val);
        }
        else {
          return $val;
        }
      }
    }
  }

  private function color_callback($matches) {
    return $this->get_hex_color($matches[1]);
  }

  private function apply_operations($obj, $img, $operations) {
    foreach ($operations as $operation => $params) {
      if (isset($this->operations[$operation])) {
        $args = $this->resolve_params($params);
        array_unshift($args, $obj, $img);
        $img = call_user_func_array($this->operations[$operation], $args);
      }
    }
    return $img;
  }

  public function intrgb($int) {
    return array(0xFF & ($int >> 0x10), 0xFF & ($int >> 0x8), 0xFF & $int);
  }

  public function get_colors() {
    return $this->colors;
  }

  public function theme_is_biurnal($name) {
    $file = drupal_get_path('theme', $name) . '/biurnal/biurnal.json';
    return is_file($file);
  }

  public function set_scheme($nid) {
    variable_set('biurnal_scheme', $nid);
  }

  public function get_colors_for_theme($name) {
    $palette=array();

    $file = drupal_get_path('theme', $name) . '/biurnal/biurnal.json';
    $content = file_get_contents($file);
    $data = json_decode($content, TRUE);
    if (isset($data) && isset($data['colors'])) {
      foreach ($data['colors'] as $name => $color) {
        $palette[$name] = '#' . $color;
      }
      return $palette;
    }
    else {
      drupal_set_message(t('Could not parse biurnal json data'), 'error');
    }
  }

  public function stop_preview() {
    unset($_SESSION['biurnal_preview']);
  }

  public function preview_scheme($nid = NULL) {
    if (isset($nid)) {
      $_SESSION['biurnal_preview'] = $nid;
      return TRUE;
    }
    else {
      if (isset($_SESSION['biurnal_preview'])) {
        $nid = $_SESSION['biurnal_preview'];
      }

      if (!empty($nid)) {
        return $nid;
      }
      else {
        return FALSE;
      }
    }
  }

  public function get_scheme() {
    $nid = $this->preview_scheme();

    if (empty($nid) && module_exists('views')) {
      $view = views_get_view('biurnal_active_scheme');

      if ($view) {
        $res = views_build_view('items', $view, NULL, FALSE, 1, 0, 0, array());

        if ($res['items'] && count($res['items'])) {
          $nid = $res['items'][0]->nid;
        }
      }
    }

    if (empty($nid)) {
      $nid = variable_get('biurnal_scheme', 0);
    }

    if ($nid) {
      $node = node_load($nid);
      return $node;
    }
    else {
      return FALSE;
    }
  }

  public function color($name = 'main') {
    if (isset($this->colors) && isset($this->colors[$name])) {
      return $this->colors[$name];
    }
    return array(0, 0, 0);
  }

  public function add_image($path, $alter = NULL, $dir = 'img', $properties = array()) {
    $dir = basename($dir);
    $this->images[$dir][] = array('path' => $path, 'alter' => $alter, 'properties' => $properties);
  }

  public function add_process($name, $process) {
    $this->processes[$name] = $process;
  }

  public function set_color($name, $color) {
    if (is_int($color))
      $color = $this->intrgb($color);
    $this->colors[$name]=$color;
  }

  public function update_color($name, $color) {
    if (isset($this->colors[$name]))
      $this->set_color($name, $color);
  }

  public function get_hex_color($name) {
    $color = $this->color($name);
    return sprintf('#%02x%02x%02x', $color[0], $color[1], $color[2]);
  }

  public function color_to_hex($color) {
    return sprintf('#%02x%02x%02x', $color[0], $color[1], $color[2]);
  }

  public function add_css($path) {
    $this->css[] = $path;
  }

  private function to_hsl_triplet($str) {
    $args = split(',', $str);
    return array(intval($args[0]), intval($args[1]), intval($args[2]));
  }

  /**
   * Convert a HSL triplet into RGB
   */
  private function hsl2rgb($hsl) {
    $h  = $hsl[0];
    $s  = $hsl[1];
    $l  = $hsl[2];
    $m2 = ($l <= 0.5) ? $l * ($s + 1) : $l + $s - $l*$s;
    $m1 = $l * 2 - $m2;
    return array(
      $this->hue2rgb($m1, $m2, $h + 0.33333),
      $this->hue2rgb($m1, $m2, $h),
      $this->hue2rgb($m1, $m2, $h - 0.33333),
    );
  }

  /**
   * Helper function for hsl2rgb().
   */
  private function hue2rgb($m1, $m2, $h) {
    $h = ($h < 0) ? $h + 1 : (($h > 1) ? $h - 1 : $h);
    if ($h * 6 < 1) return $m1 + ($m2 - $m1) * $h * 6;
    if ($h * 2 < 1) return $m2;
    if ($h * 3 < 2) return $m1 + ($m2 - $m1) * (0.66666 - $h) * 6;
    return $m1;
  }

  /**
   * Convert an RGB triplet to HSL.
   */
  private function rgb2hsl($rgb) {
    $r = $rgb[0];
    $g = $rgb[1];
    $b = $rgb[2];
    $min = min($r, min($g, $b));
    $max = max($r, max($g, $b));
    $delta = $max - $min;
    $l = ($min + $max) / 2;
    $s = 0;
    if ($l > 0 && $l < 1) {
      $s = $delta / ($l < 0.5 ? (2 * $l) : (2 - 2 * $l));
    }
    $h = 0;
    if ($delta > 0) {
      if ($max == $r && $max != $g) $h += ($g - $b) / $delta;
      if ($max == $g && $max != $b) $h += (2 + ($b - $r) / $delta);
      if ($max == $b && $max != $r) $h += (4 + ($r - $g) / $delta);
      $h /= 6;
    }
    return array($h, $s, $l);
  }
}