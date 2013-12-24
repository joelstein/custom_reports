Custom Reports Drupal 6 module
==============================

Custom Reports provides a hook for modules to generate and optionally store
their own downloadable CSV reports with a common UI. Stored reports are saved
in the file system, but are not accessible to the public.

Custom Reports depends on the Token module: http://drupal.org/project/token

Reports are available at "admin/reports/custom", and require the permission,
"access site reports".

To get started, create your own hook_custom_reports_info() function, which
should return an array of reports with the following keys:

- **id** (required): The unique identifier for this report.

- **callback**: The callback function used to generate your report results
  (defaults to id). The first argument passed is the report, followed by the
  values of any form elements you defined (see below). This function should
  return an array of results, which will be converted to CSV format.

- **title**: The title of the report, used in the UI, and in the title of the 
  downloaded CSV file (defaults to id).

- **associative**: TRUE if your report returns an associative array of results
  (where the array keys matter), or FALSE if your report returns a flat array
  (defaults to TRUE).

- **header**: TRUE if your report returns an associate array of results and the
  array keys should be used as the CSV header row. Defaults to TRUE, unless
  associative is FALSE, in which case this setting is unavailable.

- **form**: An array of additional form elements used to configure your report.
  These elements will only appear if your form is selected. Their values will
  be passed as additional arguments to your callback function, and are
  available as tokens for your filename (see below).
  
- **filename**: The name of the CSV download (include the ".csv" extension). 
  Defaults to "[date] [title].csv". If you set your own filename, you can
  include tokens which represent your custom form values.

  For example, if you included a form field called "year", then the token
  "[year]" will be replaced with that form element's value. If the form value
  is an array, it will be concatenated with hypens (-).

- **store**: TRUE if a copy of your report should be stored on the file server
  for future reference, or FALSE if your report should be downloadable only
  (defaults to FALSE).
  
  Stored reports are saved to [filepath]/custom_reports/[id]/. If you have
  additional processing which needs to take place after the report is saved
  successfully, invoked hook_custom_reports_stored(&$report).

- **active**: TRUE if your report should appear in the list, FALSE if it not
  (defaults to TRUE). Basically available so you can disable old reports,
  but still keep the name for the report history.

An example implementation, with all the available settings (note, many of 
these use the defaults, which is unnecessary):

```php
function my_module_custom_reports_info() {
  $reports[] = array(
    'id' => 'my_module_report_1',
    'callback' => 'my_module_report_1',
    'title' => t('My Module Report 1'),
    'associative' => TRUE,
    'header' => TRUE,
    'filename' => '[title] from [start_date] to [end_date].csv',
    'store' => TRUE,
    'active' => TRUE,
    'form' => array(
      'start_date' => array(
        '#type' => 'date',
        '#title' => t('Start Date'),
        '#default_value' => array('month' => 7, 'day' => 1, 'year' => 2010),
      ),
      'end_date' => array(
        '#type' => 'date',
        '#title' => 'End Date',
        '#default_value' => array('month' => 6, 'day' => 30, 'year' => 2011),
      ),
    ),
  );
  return $reports;
}
```

And here's what the callback function would look like for this report:

```php
function my_module_report_1(&$report, $start_date, $end_date) {
  // return array of results
  return array(
    array('first' => 'John', 'last' => 'Doe'),
    array('first' => 'Jane', 'last' => 'Smith'),
  );
}
```