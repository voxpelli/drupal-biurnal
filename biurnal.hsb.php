<?php
// $Id$

/**
 * @file
 * Help functions for HSB/RGB handling
 *
 * Partly translated to PHP by Hugo Wetterberg <hugo.wetterberg@goodold.se>
 * from DannyB's StarBasic code posted on http://www.oooforum.org/forum/viewtopic.phtml?t=4945
 */

define('BIURNAL_HUE', 0);
define('BIURNAL_SATURATION', 1);
define('BIURNAL_BRIGHTNESS', 2);

// $this->filters['shift_hue'] = 'biurnal_shift_hue';
function biurnal_shift_hue($color, $degrees) {
  $hsb = biurnal_rgb_to_hsb($color);
  $hsb[BIURNAL_HUE] += ($degrees/360.0);
  return biurnal_hsb_to_rgb($hsb);
}

// $this->filters['brightness'] = 'biurnal_set_brightness';
function biurnal_set_brightness($color, $brightness) {
  $hsb = biurnal_rgb_to_hsb($color);
  $hsb[BIURNAL_BRIGHTNESS] = min(abs($brightness), 1.0);
  return biurnal_hsb_to_rgb($hsb);
}

// $this->filters['match_brightness'] = 'biurnal_match_brightness';
function biurnal_match_brightness($color_a, $color_b) {
  $hsb_a = biurnal_rgb_to_hsb($color_a);
  $hsb_b = biurnal_rgb_to_hsb($color_b);
  $hsb_a[BIURNAL_BRIGHTNESS] = $hsb_b[BIURNAL_BRIGHTNESS];
  return biurnal_hsb_to_rgb($hsb_a);
}

// $this->filters['saturation'] = 'biurnal_set_saturation';
function biurnal_set_saturation($color, $saturation) {
  $hsb = biurnal_rgb_to_hsb($color);
  $hsb[BIURNAL_SATURATION] = min(abs($saturation), 1.0);
  return biurnal_hsb_to_rgb($hsb);
}

//$this->filters['brightness_and_saturation'] = 'biurnal_set_brightness_and_saturation';
function biurnal_set_brightness_and_saturation($color, $brightness, $saturation) {
  $hsb = biurnal_rgb_to_hsb($color);
  $hsb[BIURNAL_BRIGHTNESS] = min(abs($brightness), 1.0);
  $hsb[BIURNAL_SATURATION] = min(abs($saturation), 1.0);
  return biurnal_hsb_to_rgb($hsb);
}

// $this->filters['match_saturation'] = 'biurnal_match_saturation';
function biurnal_match_saturation($color_a, $color_b) {
  $hsb_a = biurnal_rgb_to_hsb($color_a);
  $hsb_b = biurnal_rgb_to_hsb($color_b);
  $hsb_a[BIURNAL_SATURATION] = $hsb_b[BIURNAL_SATURATION];
  return biurnal_hsb_to_rgb($hsb_a);
}

// $this->filters['match'] = 'biurnal_match_saturation_and_brightness';
function biurnal_match_saturation_and_brightness($color_a, $color_b) {
  $hsb_a = biurnal_rgb_to_hsb($color_a);
  $hsb_b = biurnal_rgb_to_hsb($color_b);
  $hsb_a[BIURNAL_BRIGHTNESS] = $hsb_b[BIURNAL_BRIGHTNESS];
  $hsb_a[BIURNAL_SATURATION] = $hsb_b[BIURNAL_SATURATION];
  return biurnal_hsb_to_rgb($hsb_a);
}

/**
 * Convert RGB triplet to HSB triplet
 *
 * @param array $color RGB triplet
 * @return array
 * @author Hugo Wetterberg
 */
function biurnal_rgb_to_hsb($color) {
  list($n_red, $n_green, $n_blue) = $color;
  $n_min = min($n_red, $n_green, $n_blue);
  $n_max = max($n_red, $n_green, $n_blue);

  if ($n_min == $n_max) {
    // Grayscale
    $n_hue = 0.0;
    $n_saturation = 0.0;
    $n_brightness = $n_max;
  }
  else {
    if ($n_red == $n_min) {
      $d = $n_green - $n_blue;
      $h = 3.0;
    }
    elseif ($n_green == $n_min) {
      $d = $n_blue - $n_red;
      $h = 5.0;
    }
    else {
      $d = $n_red - $n_green;
      $h = 1.0;
    }

    $n_hue = ($h - ($d / ($n_max - $n_min))) / 6.0;
    $n_saturation = ($n_max - $n_min) / $n_max;
    $n_brightness = $n_max / 255.0;
  }

  return array($n_hue, $n_saturation, $n_brightness);
}

/**
 * Convert HSB triplet to RGB triplet
 *
 * @param array $color HSB triplet
 * @return array
 * @author Hugo Wetterberg
 */
function biurnal_hsb_to_rgb($color) {
  list($n_hue, $n_saturation, $n_brightness) = $color;

  // Scale the brightness from a range of 0.0 thru 1.0
  // to a range of 0.0 thru 255.0
  // Then truncate to integer.
  // Store it into a local variable, so we don't affect
  // the value back in the caller.
  $n_brightness2 = round(min($n_brightness * 256.0, 255.0));

  if ($n_saturation == 0.0) {
    // Grayscale because there is no saturation
    $n_red   = $n_brightness2;
    $n_green = $n_brightness2;
    $n_blue  = $n_brightness2;
  }
  else {
    // Make hue angle be within a single rotation.
    // If the hue is > 1.0 or < 0.0, then it has
    // "gone around the color wheel" too many times.
    // For example, a value of 1.2 means that it has
    // gone around the wheel 1.2 times, which is really
    // the same ending angle as 0.2 trips around the wheel.
    // Scale it back into the 0.0 to 1.0 range.
    $n_hue2 = $n_hue;
    if ($n_hue2 > 1.0) {
      $n_hue2 = $n_hue2 - round($n_hue2);
    }
    elseif ( $n_hue2 < 0.0 ) {
      $n_hue2 = abs($n_hue2);
      if ($n_hue2 > 1.0) {
        $n_hue2 = $n_hue2 - round($n_hue2);
      }
      $n_hue2 = 1.0 - $n_hue2;
    }

    // Rescale hue to a range of 0.0 to 6.0.
    $n_hue2 = $n_hue2 * 6.0;

    // Separate hue into int and fractional parts
    $i_hue = round($n_hue2);
    $f_hue = $n_hue2 - $i_hue;

    // If Hue is even
    if ($i_hue % 2 == 0) {
      $f_hue = 1.0 - $f_hue;
    }

    $m = $n_brightness2 * (1.0 - $n_saturation);
    $n = $n_brightness2 * (1.0 - ($n_saturation * $f_hue));

    switch ($i_hue) {
      case 1:
        $n_red   = $n;
        $n_green = $n_brightness2;
        $n_blue  = $m;
        break;
      case 2:
        $n_red   = $m;
        $n_green = $n_brightness2;
        $n_blue  = $n;
        break;
      case 3:
        $n_red   = $m;
        $n_green = $n;
        $n_blue  = $n_brightness2;
        break;
      case 4:
        $n_red   = $n;
        $n_green = $m;
        $n_blue  = $n_brightness2;
        break;
      case 5:
        $n_red   = $n_brightness2;
        $n_green = $m;
        $n_blue  = $n;
        break;
      default:
        $n_red   = $n_brightness2;
        $n_green = $n;
        $n_blue  = $m;
        break;
    }
  }

  return array($n_red, $n_green, $n_blue);
}