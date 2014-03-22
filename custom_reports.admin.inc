<?php

// Uncomment the following line to include some sample reports:
// include 'custom_reports.sample.inc';

/**
 * Lists available reports.
 */
function custom_reports_list_form(&$form_state) {
  $options = $storable = $elements = array();
  
  foreach (custom_reports_get_reports() as $report) {
    if (!$report['active']) {
      continue;
    }
    // Add report to list
    $options[$report['id']] = $report['title'];
    // Add link to report history if stored
    if ($report['store']) {
      $options[$report['id']] .= ' - ' . l('history', "admin/reports/custom/history/{$report['id']}");
    }
    // Keep track of storable reports, so we can toggle the "run as test" checkbox
    if ($report['store']) {
      $storable[] = $report['id'];
    }
    // Add report's custom form elements (using a relevant namespace)
    if (!empty($report['form'])) {
      foreach ($report['form'] as $key => $element) {
        $form["{$report['id']}_{$key}"] = $element;
        $elements[$report['id']][] = "{$report['id']}_{$key}";
      }
    }
  }
  
  $form['report'] = array(
    '#type' => 'radios',
    '#title' => t('Report'),
    '#options' => $options,
    '#required' => TRUE,
    '#weight' => -1,
  );
  $form['test'] = array(
    '#type' => 'checkbox',
    '#title' => t('Run this report as a test'),
    '#description' => t('When checked, the report will not be saved to the file system.'),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Generate'),
  );
  
  // Add javascript to toggle certain form fields
  drupal_add_js(drupal_get_path('module', 'custom_reports') . '/custom_reports.js');
  drupal_add_js(array('custom_reports_storable' => $storable, 'custom_reports_elements' => $elements), 'setting');
  
  return $form;
}

/**
 * Generates chosen report.
 */
function custom_reports_list_form_submit($form, &$form_state) {
  $reports = custom_reports_get_reports();
  $report = $reports[$form_state['values']['report']];
  
  // Make sure callback exists
  if (!function_exists($report['callback'])) {
    form_set_error('report', t('The callback for the !title report does not exist.', array('!title' => $report['title'])));
    return;
  }
  
  // Prepare callback args
  $args[0] =& $report;
  foreach ($report['form'] as $key => $element) {
    $args[] = $report['args'][$key] = $form_state['values']["{$report['id']}_{$key}"];
  }
  
  // Generate results from report's callback function
  $report['results'] = call_user_func_array($report['callback'], $args);
  
  // Make sure report generated results
  if (empty($report['results'])) {
    $message = 'The !title report generated no results.';
    $args['!title'] = $report['title'];
    if ($report['store']) {
      $message .= ' Please check the <a href="@link">report history</a>.';
      $args['@link'] = url("admin/reports/custom/history/{$report['id']}");
    }
    form_set_error('report', t($message, $args));
    return;
  }
  
  // Convert results into CSV
  $report['csv'] = custom_reports_array_to_csv($report['results'], $report['header']);
  
  // Replace tokens in filename
  $report['filename'] = token_replace($report['filename'], 'custom_reports', $report);
  
  // If report is storaable, and not running as a test
  if ($report['store'] and !$form_state['values']['test']) {
    
    // Get/create "custom_reports" directory
    $directory = file_create_path('custom_reports');
    file_check_directory($directory, FILE_CREATE_DIRECTORY);
    
    // Protect stored reports from public access using .htaccess file
    $htaccess_file = $directory . '/.htaccess';
    $htaccess_lines = "order allow,deny\ndeny from all\n";
    if (!is_file($htaccess_file) or strpos(file_get_contents($htaccess_file), $htaccess_lines) === FALSE) {
      file_save_data($htaccess_lines, $htaccess_file);
      chmod($htaccess_file, 0644);
    }
    
    // Get/create specific report directory
    $directory .= '/' . $report['id'];
    file_check_directory($directory, FILE_CREATE_DIRECTORY);
    
    // Save file and verify it was saved before proceeding
    $path = file_save_data($report['csv'], $directory . '/' . $report['filename'], FILE_EXISTS_RENAME);
    if (!$path) {
      form_set_error('report', t('The report could not be saved to !path.', array('!path' => $directory . '/' . $report['filename'])));
      return;
    }
    
    // Let modules perform post-save processing of report
    module_invoke_all('custom_reports_stored', $report);
  }
  
  // Download!
  custom_reports_download_csv($report['csv'], $report['filename']);
}

/**
 * Returns list of available reports.
 */
function custom_reports_get_reports() {
  $reports = array();
  // Invoke hook_custom_reports_info()
  foreach (module_invoke_all('custom_reports_info') as $report) {
    // // Id must be defined
    if (empty($report['id'])) {
      continue;
    }
    // Callback defaults to id
    if (!isset($report['callback'])) {
      $report['callback'] = $report['id'];
    }
    // Title defaults to id
    if (!isset($report['title'])) {
      $report['title'] = $report['id'];
    }
    // Associative defaults to TRUE
    if (!isset($report['associative'])) {
      $report['associative'] = TRUE;
    }
    // Header rows are only available if associative is TRUE
    if (!$report['associative']) {
      $report['header'] = FALSE;
    }
    // Header defaults to TRUE (except if associative)
    if (!isset($report['header'])) {
      $report['header'] = TRUE;
    }
    // Filename defaults to something like "YYYY-MM-DD Report Title.csv"
    if (!isset($report['filename'])) {
      $report['filename'] = '[date] [title].csv';
    }
    // Set default date format.
    if (!isset($report['date_format'])) {
      $report['date_format'] = 'Y-m-d';
    }
    // Store defaults to FALSE
    if (!isset($report['store'])) {
      $report['store'] = FALSE;
    }
    // Form defaults to an empty array
    if (!isset($report['form'])) {
      $report['form'] = array();
    }
    // Active defaults to TRUE
    if (!isset($report['active'])) {
      $report['active'] = TRUE;
    }
    $report['args'] = array();
    $reports[$report['id']] = $report;
  }
  return $reports;
}

/**
 * Displays list of previously generated reports.
 * Downloadable via system file download only.
 */
function custom_reports_history($report = NULL) {
  $reports = custom_reports_get_reports();
  
  // Determine path
  $directories[] = 'custom_reports';
  if ($report) {
    $directories[] = $report;
  }
  $path = file_create_path(join('/', $directories));
  
  // Get files in path
  $files = file_scan_directory($path, '.*');
  if (empty($files)) {
    return t('No history is available for this report.');
  }
  krsort($files);
  
  // Table header
  $header = array(t('Report'), t('Filename'));
  
  // Table rows
  foreach ($files as $file) {
    $directory = basename(dirname($file->filename));
    $directory_name = isset($reports[$directory]) ? $reports[$directory]['title'] : $directory;
    $rows[] = array(
      l($directory_name, "admin/reports/custom/history/{$directory}"),
      l($file->basename, "system/files/{$file->filename}"),
    );
  }
  
  return theme('table', $header, $rows);
}
