<?php
// $Id$

/**
 * @file
 * Documentation for biurnal API.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * DUMMY EXAMPLE fetched from http://api.drupal.org/api/file/modules/aggregator/aggregator.api.php/7/source
 * Implement this hook to expose the title and a short description of your
 * processor.
 *
 * The title and the description provided are shown most importantly on
 * admin/settings/aggregator. Use as title the natural name of the processor
 * and as description a brief (40 to 80 characters) explanation of the
 * functionality.
 *
 * This hook is only called if your module implements
 * hook_aggregator_process(). If this hook is not implemented aggregator
 * will use your module's file name as title and there will be no description.
 *
 * @return
 *   An associative array defining a title and a description string.
 *
 * @see hook_aggregator_process()
 *
 * @ingroup aggregator
 */
function hook_aggregator_process_info($feed) {
  return array(
    'title' => t('Default processor'),
    'description' => t('Creates lightweight records of feed items.'),
  );
}

/**
 * @} End of "addtogroup hooks".
 */