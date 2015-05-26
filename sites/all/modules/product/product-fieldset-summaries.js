(function ($) {
    Drupal.behaviors.productFieldsetSummaries = {
        attach: function (context) {
            $('fieldset.product-form-author', context).drupalSetSummary(function (context) {
                return $('div.form-item-name input', context).val();
            });

            $('fieldset.product-form-author', context).drupalSetSummary(function (context) {
                var name = $('div.form-item-name input', context).val() || Drupal.settings.anonymous,
                    date = $('div.form-item-date input', context).val();

                return date ?
                    Drupal.t('By @name on @date', {'@name': name, '@date': date}) :
                    Drupal.t('By @name', {'@name': name});
            });

            $('fieldset.product-form-options', context).drupalSetSummary(function (context) {
                var vals = [];
                var statusId = $('div.form-item-status input:checked', context).attr('id');
                var status = $('div.form-item-status label[for=' + statusId + ']', context).text();

                vals.push(Drupal.checkPlain($.trim(status)));
                return vals.join(', ');
            });
        }
    };
})(jQuery);