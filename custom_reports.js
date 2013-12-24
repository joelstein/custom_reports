Drupal.behaviors.custom_reports_list_form = function(context) {

  // When the report switches
  $('#custom-reports-list-form input[name=report]').click(function() {
    selected = $('input[name=report]:checked').val();

    // Show "run as test" for storable reports
    if ($.inArray(selected, Drupal.settings.custom_reports_storable) != -1) {
      $('#edit-test-wrapper').show();
    } else {
      $('#edit-test-wrapper').hide();
    }

    // Hide all additional form elements
    $.each(Drupal.settings.custom_reports_elements, function(i, value) {
      $.each(value, function(i, value2) {
        $('#edit-' + value2.replace(/_/g, '-') + '-wrapper').hide();
      });
    });

    // Show specific additional form elements
    if (Drupal.settings.custom_reports_elements[selected] != undefined) {
      $.each(Drupal.settings.custom_reports_elements[selected], function(i, value) {
        $('#edit-' + value.replace(/_/g, '-') + '-wrapper').show();
      });
    }

  }).triggerHandler('click');

};
