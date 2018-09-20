(function ($) {

  Drupal.behaviors.jw_player_general_admin = {
    attach: function (context, settings) {

      $('.page-admin-config-media-jw-player-settings', context).once('general-admin', function () {
        var player_version = '#edit-jw-player-version';
        var self_host = 'input:radio[name=jw_player_hosting]';
        var self_key = '#edit-jw-player-key';
        var self_key_7 = '#edit-jw-player-key-7';
        var cloud_default = '#edit-jw-player-cloud-player-default';

        // If JW7 or newer, clear self key used for older versions.
        // If JW6, check if whether to remove cloud or self, plus remove JW7 self key.
        // If JW5 or older, only keep self key used for older versions.
        $(player_version).change(function () {
          if ($(this).val() >= 7) {
            $(self_key).val('');
          }
          else if ($(this).val() == 6) {
            if ($(self_host + ':checked').val() == 'self') {
              $(cloud_default).val('');
            }
            else {
              $(self_key).val('');
            }
            $(self_key_7).val('');
          }
          else {
            $(cloud_default).val('');
            $(self_key_7).val('');
          }
        });
        // If self-hosting selected, clear cloud value,
        // otherwise clear self-hosting values.
        if ($(self_host + ':checked').val() == 'self') {
          $(cloud_default).val('');
        }
        else {
          $(self_key).val('');
          $(self_key_7).val('');
        }
        // When hosting type changed, check if cloud or self.
        // Clear cloud value if self, and self values if cloud.
        $(self_host).change(function () {
          if ($(this).is(':checked') && $(this).val() == 'self') {
            $(cloud_default).val('');
          }
          else {
            $(self_key).val('');
            $(self_key_7).val('');
          }
        });

        // If cloud default URL is being changed, display a warning message that all
        // presets must be reviewed and updated that use JW Player as a preset source.
        // Not compatible with jQuery 1.6 and older, and Internet Explorer 8 and older.
        $(cloud_default).on('input propertychange change paste', function () {
          // Only run if current value is not empty and default value previously existed.
          if ($(this).val() != '' && $(this).prop('defaultValue') != '') {
            var parent = $(this).closest('.form-item');
            if ($(parent).find('div.warning').length == 0) {
              var msg = 'Changing this URL requires review of <a href="/admin/config/media/jw_player">all presets</a> that use JW Player as the preset source. Any preset with the same URL as the default setting may need to be updated.';
              $(parent).append('<div class="messages warning">' + msg + '</div>');
            }
          }
        });
      });
    }
  };

  Drupal.behaviors.jw_player_preset_admin = {
    attach: function (context, settings) {

      $('.page-admin-config-media-jw-player-list', context).once('preset-admin', function () {
        var resp = '#edit-settings-responsive';
        var width_field = '#edit-settings-width';
        var width_suffix = '.form-item-settings-width .field-suffix';

        // If responsive is checked, remove width value
        // and change field suffix to percentage symbol.
        if ($(resp).is(':checked')) {
          $(width_suffix).text('%');
        }

        // When responsive value is changed, clear the width field value, and
        // change field suffix to '%' when responsive and 'pixels' when fixed.
        $(resp).change(function () {
          if ($(this).is(':checked')) {
            $(width_field).val('');
            $(width_suffix).text('%');
          }
          else {
            $(width_field).val('');
            $(width_suffix).text('pixels');
          }
        });
      });

    }
  };

  Drupal.behaviors.jwplayerSharingSites = {
    attach: function (context, settings) {
      $('#edit-settings-sharing-sites-show input.form-checkbox', context).once('settings-sharing-sites', function () {
        var $checkbox = $(this);
        // Retrieve the tabledrag row belonging to this content.
        var $row = $('#' + $checkbox.attr('id').replace(/-show-/, '-order-') + '-weight', context).closest('tr');
        
        // Bind click handler to this checkbox to conditionally show and hide the
        // filter's tableDrag row.
        $checkbox.bind('click.sharingSitesUpdate', function () {
          if ($checkbox.is(':checked')) {
            $row.show();
          }
          else {
            $row.hide();
          }
          // Restripe table after toggling visibility of table row.
          Drupal.tableDrag['jw-player-sharing-sites-order'].restripeTable();
        });

        // Trigger our bound click handler to update elements to initial state.
        $checkbox.triggerHandler('click.sharingSitesUpdate');
      });
    }
  };

})(jQuery);
