<?php

//--- SAMPLE DATA ------------------------------------------------------

function custom_reports_custom_reports_info(&$report) {
  $items[] = array(
    'title' => t('No id (should not appear)'),
  );
  $items[] = array(
    'id' => 'sample0',
    'title' => t('Active is FALSE (should not appear)'),
    'active' => FALSE,
  );
  $items[] = array(
    'id' => 'sample1',
    'title' => t('Associative, with header'),
    'callback' => 'custom_reports_sample_associative',
  );
  $items[] = array(
    'id' => 'sample2',
    'title' => t('Associative, without header'),
    'callback' => 'custom_reports_sample_associative',
    'header' => FALSE,
  );
  $items[] = array(
    'id' => 'sample3',
    'title' => t('Non-associative'),
    'callback' => 'custom_reports_sample_non_associative',
    'associative' => FALSE,
  );
  $items[] = array(
    'id' => 'sample4',
    'title' => t('No results'),
    'callback' => 'custom_reports_sample_no_results',
    'associative' => FALSE,
  );
  $items[] = array(
    'id' => 'sample5',
    'title' => t('Storable'),
    'callback' => 'custom_reports_sample_associative',
    'store' => TRUE,
  );
  $items[] = array(
    'id' => 'sample6',
    'title' => t('With form options'),
    'callback' => 'custom_reports_sample_with_form_options',
    'form' => array(
      'start_date' => array(
        '#type' => 'date',
        '#title' => t('Start Date'),
        '#default_value' => array('month' => 7, 'day' => 1, 'year' => 2010),
        '#required' => TRUE,
      ),
      'end_date' => array(
        '#type' => 'date',
        '#title' => 'End Date',
        '#default_value' => array('month' => 6, 'day' => 30, 'year' => 2011),
      ),
    ),
  );
  return $items;
}

function custom_reports_sample_associative(&$report) {
  $rows = array(
    array('first' => 'John', 'last' => 'Doe'),
    array('first' => 'Jane', 'last' => 'Doe'),
  );
  return $rows;
}

function custom_reports_sample_non_associative(&$report) {
  $rows = array(
    array('first', 'last', 'first initial', 'middle', 'last'),
    array('John', 'Doe', 'J', 'V', 'Doe'),
    array('Jane', 'Doe', 'J', 'W', 'Doe'),
  );
  return $rows;
}

function custom_reports_sample_no_results(&$report) {
  return array();
}

function custom_reports_sample_with_form_options(&$report, $start_date, $end_date) {
  dsm(func_get_args());
}
