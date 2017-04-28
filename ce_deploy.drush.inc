<?php
/**
 * @file
 * ce_deploy.drush.inc
 * Some helper commands to help on deployments and / or local setups.
 */

/**
 * Implements hook_drush_command().
 */
function ce_deploy_drush_command() {
  $items['ce-revert-all'] = array(
    'description' => dt('Revert features, views, ctools exportables, secure permissions and clear caches.'),
    'aliases' => array('cera'),
  );
  $items['ce-deploy-update'] = array(
    'description' => dt('Assert required modules / features / taxonomies are enabled, permissions are in sync with code,
      and general settings not kept in features match the values in code.'),
    'aliases' => array('cedu'),
  );
  $items['ce-deploy-postinstall'] = array(
    'description' => dt('Runs any additional steps needed after a drush site-install to complete a zero-touch fresh install'),
    'aliases' => array('cepi'),
  );
  $items['ce-switch-branch-settings'] = array(
    'description' => "Switches the local.settings.php file with a different settings file from the settings-folder chosen (optional).",
    'arguments' => array(
      'branch' => 'Branch-specific settings to enable (if present).',
    ),
    'options' => array(
      'settings-folder' => 'The folder (within the sites/default path) containing the alternative settings files.',
    ),
    'aliases' => array('ce-sbs'),
  );
  return $items;
}

/**
 * Revert all components.
 */
function drush_ce_deploy_ce_revert_all() {
  drush_set_option('force', TRUE);
  drush_set_option('all', TRUE);
  cache_clear_all();
  registry_rebuild();
  menu_rebuild();
  secure_permissions_rebuild();
  drush_features_revert_all();
  views_revert_views();
  drush_set_context('command', array('command' => 'ctools-export-revert'));
  ctools_drush_export_op_command();
}

/**
 * Make sure required modules and settings match version in ce_deploy module.
 */
function drush_ce_deploy_update() {
  ce_deploy_module_enable();
//  ce_deploy_set_theme();
//  ce_deploy_taxonomies();
  ce_deploy_features();
  secure_permissions_rebuild();
//  ce_deploy_set_variables();
  cache_clear_all();
}

/**
 * Additional steps to perform after a drush site-install.
 */
function drush_ce_deploy_postinstall() {
  require_once "includes/drush.inc";
  // Some features might appear as overriden.
  ce_deploy_fix_entityreferences();
  // TODO: Add feature modules that need reverting after fixing references.
  ce_deploy_features_revert(array());
}

/**
 * Command callback for 'drush ce-switch-branch-settings' command ("drush ce-sbs").
 */
function drush_ce_deploy_ce_switch_branch_settings() {
  if ($args = func_get_args()) {
    // Assemble needed variables.
    $branch = array_shift($args);
    $alternative_settings_folder = 'branch_settings';
    $alternative_settings_folder = (drush_get_option('settings-folder') != NULL) ? drush_get_option('settings-folder') : $alternative_settings_folder;
    // Sensible to assume this, but might change per environment.
    $result_settings_file = 'local.settings.php';

    drush_print("Checking settings for branch: " . $branch);

    // Get the conf path for the current context, and the settings file to be
    // set up as 'local.settings.php'.
    $conf_path = drush_bootstrap_value('conf_path', conf_path(TRUE, TRUE));
    $alternative_conf_file = $conf_path . DIRECTORY_SEPARATOR . $alternative_settings_folder . DIRECTORY_SEPARATOR . $branch . ".settings.php";

    // If the alternative file is not found, exit with an error.
    if (!file_exists($alternative_conf_file)) {
      return drush_set_error('', dt("Could not find an alternative Drupal settings.php file at !file.",
        array('!file' => $alternative_conf_file)));
    }
    // Alternative file exists. Replace (or create) the result settings file.
    else {
      $new_conf_file = $conf_path . DIRECTORY_SEPARATOR . $result_settings_file;

      // Print message according to existence of the result file.
      if (file_exists($new_conf_file)) {
        drush_log($new_conf_file . " will be replaced with the contents of " . $alternative_conf_file, 'ok');
      }
      else {
        drush_log($new_conf_file . " will be created with the contents of " . $alternative_conf_file, 'notice');
      }

      // Try to perform the copy of the settings file.
      if (copy($alternative_conf_file, $new_conf_file)) {
        drush_log($new_conf_file . ' successfully (re)created.', 'success');
      }
      else {
        return drush_set_error('', "Could not perform the settings replacement.");
      }
    }
  }
  else {
    return drush_set_error('', 'No branch supplied.');
  }
}
