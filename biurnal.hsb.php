<?php
// $Id$

define('BIURNAL_HUE',0);
define('BIURNAL_SATURATION',1);
define('BIURNAL_BRIGHTNESS',2);

// $this->filters['shift_hue'] = 'biurnal_shift_hue';
function biurnal_shift_hue($color, $degrees)
{
  $hsb = biurnal_RGBtoHSB($color);
  $hsb[BIURNAL_HUE] += ($degrees/360.0);

  return biurnal_HSBtoRGB($hsb);
}

// $this->filters['brightness'] = 'biurnal_set_brightness';
function biurnal_set_brightness($color, $brightness)
{
  $hsb = biurnal_RGBtoHSB($color);
  $hsb[BIURNAL_BRIGHTNESS] = min(abs($brightness),1.0);

  return biurnal_HSBtoRGB($hsb);
}

// $this->filters['match_brightness'] = 'biurnal_match_brightness';
function biurnal_match_brightness($colorA, $colorB)
{
  $hsbA = biurnal_RGBtoHSB($colorA);
  $hsbB = biurnal_RGBtoHSB($colorB);
  
  $hsbA[BIURNAL_BRIGHTNESS] = $hsbB[BIURNAL_BRIGHTNESS];
  return biurnal_HSBtoRGB($hsbA);
}

// $this->filters['saturation'] = 'biurnal_set_saturation';
function biurnal_set_saturation($color, $saturation)
{
  $hsb = biurnal_RGBtoHSB($color);
  $hsb[BIURNAL_SATURATION] = min(abs($saturation),1.0);
  return biurnal_HSBtoRGB($hsb);
}

//$this->filters['brightness_and_saturation'] = 'biurnal_set_brightness_and_saturation';
function biurnal_set_brightness_and_saturation($color, $brightness, $saturation)
{
  $hsb = biurnal_RGBtoHSB($color);
  $hsb[BIURNAL_BRIGHTNESS] = min(abs($brightness),1.0);
  $hsb[BIURNAL_SATURATION] = min(abs($saturation),1.0);
  return biurnal_HSBtoRGB($hsb);
}

// $this->filters['match_saturation'] = 'biurnal_match_saturation';
function biurnal_match_saturation($colorA, $colorB)
{
  $hsbA = biurnal_RGBtoHSB($colorA);
  $hsbB = biurnal_RGBtoHSB($colorB);
  
  $hsbA[BIURNAL_SATURATION] = $hsbB[BIURNAL_SATURATION];
  return biurnal_HSBtoRGB($hsbA);
}

// $this->filters['match'] = 'biurnal_match_saturation_and_brightness';
function biurnal_match_saturation_and_brightness($colorA, $colorB)
{
  $hsbA = biurnal_RGBtoHSB($colorA);
  $hsbB = biurnal_RGBtoHSB($colorB);
  
  $hsbA[BIURNAL_BRIGHTNESS] = $hsbB[BIURNAL_BRIGHTNESS];
  $hsbA[BIURNAL_SATURATION] = $hsbB[BIURNAL_SATURATION];
  return biurnal_HSBtoRGB($hsbA);
}

/*
Translated to PHP by Hugo Wetterberg <hugo.wetterberg@goodold.se> 
from DannyB's StarBasic code posted on http://www.oooforum.org/forum/viewtopic.phtml?t=4945
*/

/**
 * Convert RGB triplet to HSB triplet
 *
 * @param array $color RGB triplet
 * @return array
 * @author Hugo Wetterberg
 **/
function biurnal_RGBtoHSB( $color ) {
  list( $nRed, $nGreen, $nBlue ) = $color;
  $nMin = min( $nRed, $nGreen, $nBlue );
  $nMax = max( $nRed, $nGreen, $nBlue );

  if ($nMin == $nMax) {
    // Grayscale 
    $nHue = 0.0;
    $nSaturation = 0.0;
    $nBrightness = $nMax;
  }
  else {
    if ($nRed == $nMin) {
      $d = $nGreen - $nBlue;
      $h = 3.0; 
    }
    else if ($nGreen == $nMin) {
      $d = $nBlue - $nRed;
      $h = 5.0;
    }
    else {
      $d = $nRed - $nGreen;
      $h = 1.0;
    }
   
    $nHue = ( $h - ( $d / ($nMax - $nMin) ) ) / 6.0;
    $nSaturation = ($nMax - $nMin) / $nMax;
    $nBrightness = $nMax / 255.0;
  }
  
  return array($nHue, $nSaturation, $nBrightness);
}

/**
 * Convert HSB triplet to RGB triplet
 *
 * @param array $color HSB triplet
 * @return array
 * @author Hugo Wetterberg
 **/
function biurnal_HSBtoRGB( $color ) {
  list( $nHue, $nSaturation, $nBrightness ) = $color;
  
  // Scale the brightness from a range of 0.0 thru 1.0 
  // to a range of 0.0 thru 255.0 
  // Then truncate to integer. 
  // Store it into a local variable, so we don't affect 
  // the value back in the caller. 
  $nBrightness2 = round( min( $nBrightness * 256.0, 255.0 ));
    
  if ($nSaturation == 0.0) { 
    // Grayscale because there is no saturation 
    $nRed = $nBrightness2;
    $nGreen = $nBrightness2; 
    $nBlue = $nBrightness2;
  }
  else { 
    // Make hue angle be within a single rotation. 
    // If the hue is > 1.0 or < 0.0, then it has 
    // "gone around the color wheel" too many times. 
    // For example, a value of 1.2 means that it has 
    // gone around the wheel 1.2 times, which is really 
    // the same ending angle as 0.2 trips around the wheel. 
    // Scale it back into the 0.0 to 1.0 range. 
    $nHue2 = $nHue;
    if ($nHue2 > 1.0) {
      $nHue2 = $nHue2 - round($nHue2); 
    }
    else if ( $nHue2 < 0.0 ) {
      $nHue2 = abs( $nHue2 );
      if ($nHue2 > 1.0) {
        $nHue2 = $nHue2 - round($nHue2);
      }
      $nHue2 = 1.0 - $nHue2;
    }
       
    // Rescale hue to a range of 0.0 to 6.0. 
    $nHue2 = $nHue2 * 6.0;
       
    // Separate hue into int and fractional parts 
    $iHue = round($nHue2);
    $fHue = $nHue2 - $iHue;
       
    // If Hue is even 
    if ($iHue % 2 == 0) {
      $fHue = 1.0 - $fHue;
    } 
       
    $m = $nBrightness2 * (1.0 - $nSaturation);
    $n = $nBrightness2 * (1.0 - ($nSaturation * $fHue));
     
    switch($iHue) {
      case 1:
        $nRed = $n;
        $nGreen = $nBrightness2;
        $nBlue = $m;
        break;
      case 2:
        $nRed = $m;
        $nGreen = $nBrightness2;
        $nBlue = $n;
        break;
      case 3:
        $nRed = $m;
        $nGreen = $n;
        $nBlue = $nBrightness2;
        break;
      case 4:
        $nRed = $n;
        $nGreen = $m;
        $nBlue = $nBrightness2;
        break;
      case 5:
        $nRed = $nBrightness2;
        $nGreen = $m;
        $nBlue = $n;
        break;
      default: 
        $nRed = $nBrightness2;
        $nGreen = $n;
        $nBlue = $m;
        break;
    }
  }
  
  return array($nRed, $nGreen, $nBlue);
}