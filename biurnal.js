// $Id$

if (Drupal.jsEnabled) 
{
  jQuery(document).ready(function () 
  {
    var form = jQuery('.biurnal-color-scheme-form');
    var inputs = [];
    var hooks = [];
    var locks = [];
    var focused = null;

    // Add Farbtastic
    jQuery(form).prepend('<div class="biurnal-farbtastic-placeholder"></div>');
    var farb = jQuery.farbtastic('.biurnal-farbtastic-placeholder');
    
    // Focus the Farbtastic on a particular field.
    function focus() 
    {
      var input = this;
      // Remove old bindings
      focused && jQuery(focused).unbind('keyup', farb.updateValue)
          .parent().removeClass('item-selected');

      // Add new bindings
      focused = this;
      farb.linkTo(function (color) { callback(input, color, true, false) });
      farb.setColor(this.value);
      jQuery(focused).keyup(farb.updateValue)
        .parent().addClass('item-selected');
    }
    
    /**
     * Callback for Farbtastic when a new color is chosen.
     */
    function callback(input, color, propagate, colorscheme) 
    {
      // Set background/foreground color
      $(input).css({
        backgroundColor: color,
        color: farb.RGBToHSL(farb.unpack(color))[2] > 0.5 ? '#000' : '#fff'
      });

      // Change input value
      if (input.value && input.value != color) {
        input.value = color;

        // Update locked values
        if (propagate) {
          var i = input.i;
          for (j = i + 1; ; ++j) {
            if (!locks[j - 1] || $(locks[j - 1]).is('.unlocked')) break;
            var matched = shift_color(color, reference[input.key], reference[inputs[j].key]);
            callback(inputs[j], matched, false);
          }
          for (j = i - 1; ; --j) {
            if (!locks[j] || $(locks[j]).is('.unlocked')) break;
            var matched = shift_color(color, reference[input.key], reference[inputs[j].key]);
            callback(inputs[j], matched, false);
          }
        }
      }

    }

    // Initialize color fields
    jQuery('.form-text', form).each(function () 
    {
      // Extract palette field name
      this.key = this.id.substring(13);

      // Link to color picker temporarily to initialize.
      farb.linkTo(function () {}).setColor('#000').linkTo(this);

      // Add lock
      var i = inputs.length;
      if (inputs.length) 
      {
        var lock = jQuery('<div class="lock"></div>').toggle(
          function () 
          {
            jQuery(this).addClass('unlocked');
            jQuery(hooks[i - 1]).attr('class',
              locks[i - 2] && jQuery(locks[i - 2]).is(':not(.unlocked)') ? 'hook up' : 'hook'
            );
            jQuery(hooks[i]).attr('class',
              locks[i] && jQuery(locks[i]).is(':not(.unlocked)') ? 'hook down' : 'hook'
            );
          },
          function () 
          {
            jQuery(this).removeClass('unlocked');
            jQuery(hooks[i - 1]).attr('class',
              locks[i - 2] && jQuery(locks[i - 2]).is(':not(.unlocked)') ? 'hook both' : 'hook down'
            );
            jQuery(hooks[i]).attr('class',
              locks[i] && jQuery(locks[i]).is(':not(.unlocked)') ? 'hook both' : 'hook up'
            );
          }
        );
        jQuery(this).after(lock);
        locks.push(lock);
      }

      // Add hook
      var hook = jQuery('<div class="hook"></div>');
      jQuery(this).after(hook);
      hooks.push(hook);

      jQuery(this).parent().find('.lock').click();
      this.i = i;
      inputs.push(this);
    }).focus(focus);
  });
}