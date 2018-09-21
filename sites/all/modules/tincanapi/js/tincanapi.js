/**
 * @file
 * Handles creating statements using JS.
 */

(function ($) {

  Drupal.tincanapi = {
    track: function(data, callback) {
      if (!Drupal.settings.tincanapi) {
        return;
      }

      data.token = Drupal.settings.tincanapi.token;

      $.ajax({
        type: 'POST',
        url: Drupal.settings.basePath + 'ajax/tincanapi/track',
        data: data,
        complete: callback
      });
    }
  };

  /**
   * When an ctools popup is displayed, the currentPage is adapted.
   * But when the ctools popup is close, the page still thinks it
   * is on that popup page. But following the opening and closing 
   * of the popups with a history stack, the proper currentPage
   * is restored.
   */

  var history = [];
  Drupal.behaviors.tincanapi = {
    attach: function (context, settings) {
      if (Drupal.settings.tincanapi) {
        if (history.length == 0 || history[history.length - 1] != Drupal.settings.tincanapi.currentPage) {
          history.push(Drupal.settings.tincanapi.currentPage);
        }
      }
    }
  };

  $(document).bind("CToolsDetachBehaviors", function() {
    if (Drupal.settings.tincanapi && history[history.length - 1] == Drupal.settings.tincanapi.currentPage) {
      history.pop();
      Drupal.settings.tincanapi.currentPage = history[history.length - 1];
    }
  });

  /**
   * Update the parent, when the currentPage changes because a new
   * l-page-container is loaded.
   */

  var loadCallback = function() {
    if (Drupal.settings.tincanapi)  {
      history[0] = Drupal.settings.tincanapi.currentPage;
    }
  };

  if ($.fn.on) {
    $(document).on('load', '.l-page-container', loadCallback);
  } else {
    $('.l-page-container').live('load', loadCallback);
  }

})(jQuery);
