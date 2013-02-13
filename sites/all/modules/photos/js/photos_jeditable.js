(function ($) {
Drupal.behaviors.photosJeditable = {
  attach: function(context) {
    var atext = [Drupal.t('Save'), Drupal.t('Cancel'), Drupal.t('Being updated...'), Drupal.t('Click to edit')];
    $('.jQueryeditable_edit_filename, .jQueryeditable_edit_des').hover(function(){
      $(this).addClass('photot_ajax_hover');
    },function(){
      $(this).removeClass('photot_ajax_hover');
    });

    // @todo need fid. Create alternative way to edit images. Need token etc.
    $('.jQueryeditable_edit_filename').editable(Drupal.settings.basePath + 'photos/image/update', {
      type : 'textarea',
      submit : atext[0],
      cancel : atext[1],
      indicator : atext[2],
      tooltip : atext[3],
      loadtype : 'POST',
      loadtext   : 'Loading...',
      submitdata : {token : Drupal.settings.photos.image_edit_token},
      callback : function(value, settings) {
        // @todo add option for title selector i.e. #page-title $('#page-title').text(value);
        // @note test on album page (make sure album title does not change).
      }
    }, function(){
      return false;
    });
    $('.jQueryeditable_edit_des').editable(Drupal.settings.basePath + 'photos/image/update', {
      height : 140,
      type : 'text',
      submit : atext[0],
      cancel : atext[1],
      indicator : atext[2],
      tooltip : atext[3],
      type : 'textarea',
      loadtype: 'POST',
      loadtext   : 'Loading...',
      submitdata : {token : Drupal.settings.photos.image_edit_token}
    }, function(){
      return false;
    });

    $('.jQueryeditable_edit_delete').click(function(){
      d = $(this).attr('alt');
      $(this).addClass('photos_ajax_del_img');
      $.get($(this).attr('href'), {go: 1}, function(v){
        if(v !== 0){
          $('#' + d).remove();
        }
        else{
          alert(Drupal.t('Delete failure'));
        }
      });
      return false;
    });
  }
};
})(jQuery);
