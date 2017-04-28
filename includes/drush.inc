<?php
/**
 * @file
 * Helper functions to be used just by drush commands.
 */

/**
 * Update foreign keys where necessary.
 *
 * Features containing entityreference base tables referenced need to be right
 * on the database, otherwise features containing field info for those fields
 * will appear as overriden after a fresh-install. This code is the last patch
 * of https://www.drupal.org/node/1969018, but added here to be applied after
 * drush si manually, so that we don't need a patch for entityreference.
 *
 * @see https://www.drupal.org/node/1969018
 * @see https://www.drupal.org/node/1416506
 */
function ce_deploy_fix_entityreferences() {
  // Load the base table configuration from the cache.
  $base_tables = variable_get('entityreference:base-tables', array());

  foreach (field_info_fields() as $field_name => $field) {
    if ($field['type'] != 'entityreference') {
      // Not an entity reference field.
      continue;
    }

    // Read in the field config.
    $field_name = field_read_field($field['field_name']);

    // Create a foreign key to the target entity type base type, if available.
    $entity_type = $field_name['settings']['target_type'];
    if (isset($base_tables[$entity_type])) {
      list($base_table, $id_column) = $base_tables[$entity_type];
      $schema['foreign keys'][$base_table] = array(
        'table' => $base_table,
        'columns' => array('target_id' => $id_column),
      );
    }

    // Update foreign keys to actually contain the proper entity foreign keys
    // if they differ.
    if ($field_name['foreign keys'] != $schema['foreign keys']) {
      $field_name['foreign keys'] = $schema['foreign keys'];
      field_update_field($field_name);
    }
  }
}
