<?php
/**
 * @file
 * Handles album settings form.
 */

/**
 * Node album settings form.
 */
function _photos_form($node) {
  global $user;
/* drupal_add_js(
<<<EOT
$(function() {
  select_hide($("input[@name='album[page_display]'][@checked]").val(), '#photos-form-page');
  $("input[@name='album[page_display]']").click(function() {
    select_hide($(this).val(), '#photos-form-page');
  });
  select_hide($("input[@name='album[teaser_display]'][@checked]").val(), '#photos-form-teaser');
  $("input[@name='album[teaser_display]']").click(function() {
    select_hide($(this).val(), '#photos-form-teaser');
  });
})
EOT
, 'inline');
*/
  if ($user->uid <> 1) {
    $t = photos_user_count();
    $msg = t('You may create %a albums, now you have created %b albums, you can still create %aa albums.', array('%a' => $t['total'], '%b' => $t['create'], '%aa' => $t['remain']));
    if (isset($t['rest'])) {
      drupal_set_message($msg);
      return drupal_goto('node/add');
    }
    $form['help'] = array('#value' => $msg, '#weight' => -10);
  }
  $type = node_type_get_type($node);
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => check_plain($type->title_label),
    '#required' => TRUE,
    '#default_value' => !empty($node->title) ? $node->title : NULL,
    '#weight' => -5
  );
  $form['photos'] = array(
    '#type' => 'fieldset',
    '#title' => t('Album settings'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#weight' => -3
  );
  $opt = array(t('Do not display'), t('Display cover'), t('Display thumbnails'));
  if (variable_get('photos_display_user', 1)) {
    $form['photos']['global']['album'] = array(
      '#type' => 'fieldset',
      '#title' => t('Global Settings'),
      '#collapsible' => TRUE,
      '#tree' => TRUE
    );
    $form['photos']['global']['album']['viewpager'] = array(
      '#type' => 'select',
      '#default_value' => !empty($node->album['viewpager']) ? $node->album['viewpager'] : variable_get('photos_display_viewpager', 10),
      '#title' => t('How many images show in each page?'),
      '#required' => TRUE,
      '#options' => drupal_map_assoc(array(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, 25, 30, 35, 40, 45, 50))
    );
    $form['photos']['global']['album']['imageorder'] = array(
      '#type' => 'select',
      '#title' => t('Image display order'),
      '#required' => TRUE,
      '#default_value' => !empty($node->album['imageorder']) ? $node->album['imageorder'] : variable_get('photos_display_imageorder', 'timestamp|desc'),
      '#options' => _photos_order_label()
    );
    $size = photos_upload_info(0);
    $image_styles = image_style_options(FALSE);
    $list_imagesize = variable_get('photos_display_list_imagesize', 'medium');
    $view_imagesize = variable_get('photos_display_view_imagesize', 'large');
    $full_imagesize = variable_get('photos_display_full_imagesize', 'large');
    $teaser_imagesize = variable_get('photos_display_teaser_imagesize', 'thumbnail');

    if (isset($node->album['list_imagesize'])) {
      $style_name = $node->album['list_imagesize'];
      // Necessary when upgrading from D6 to D7.
      if (isset($image_styles[$style_name])) {
        $list_imagesize = $style_name;
      }
    }

    if (isset($node->album['view_imagesize'])) {
      $style_name = $node->album['view_imagesize'];
      // Necessary when upgrading from D6 to D7.
      if (isset($image_styles[$style_name])) {
        $view_imagesize = $style_name;
      }
    }

    if (isset($node->album['full_imagesize'])) {
      $style_name = $node->album['full_imagesize'];
      // Necessary when upgrading from D6 to D7.
      if (isset($image_styles[$style_name])) {
        $full_imagesize = $style_name;
      }
    }

    if (isset($node->album['teaser_imagesize'])) {
      $style_name = $node->album['teaser_imagesize'];
      // Necessary when upgrading from D6 to D7.
      if (isset($image_styles[$style_name])) {
        $teaser_imagesize = $style_name;
      }
    }

    $form['photos']['global']['album']['list_imagesize'] = array(
      '#type' => 'select',
      '#title' => t('Image display size(by list)'),
      '#required' => TRUE,
      '#default_value' => $list_imagesize,
      '#description' => t('Displayed in the list(e.g: photos/album/[nid]) of image size.'),
      '#options' => _photos_select_size()
    );
    $form['photos']['global']['album']['view_imagesize'] = array(
      '#type' => 'select',
      '#title' => t('Image display size(by page)'),
      '#required' => TRUE,
      '#default_value' => $view_imagesize,
      '#description' => t('Displayed in the page(e.g: photos/image/[fid]) of image size.'),
      '#options' => _photos_select_size()
    );
    if (variable_get('photos_slide', 0)) {
      $form['photos']['global']['album']['slide'] = array(
        '#type' => 'textfield',
        '#default_value' => !empty($node->album['slide']) ? $node->album['slide'] : variable_get('photos_display_slide', '640x480'),
        '#title' => t('Slide width and height'),
        '#required' => TRUE,
        '#description' => t('The width and height of flash, e.g: 640x480.'),
      );
      $form['photos']['global']['album']['slide_music'] = array(
        '#type' => 'textfield',
        '#default_value' => !empty($node->album['slide_music']) ? $node->album['slide_music'] : '',
        '#title' => t('Slide Music'),
        '#description' => t('e.g: http://mysite/music.mp3'),
      );
      $opt[] = t('Slide show');
    }
  }

  if (variable_get('photos_display_page_user', 1)) {
    $form['photos']['page']['album'] = array(
      '#type' => 'fieldset',
      '#title' => t('Page Settings'),
      '#collapsible' => TRUE,
      '#tree' => TRUE,
      '#prefix' => '<div id="photos-form-page">',
      '#suffix' => '</div>'
    );
    $form['photos']['page']['album']['page_display'] = array(
      '#type' => 'radios',
      '#default_value' => !empty($node->album['page_display']) ? $node->album['page_display'] : variable_get('photos_display_page_display', 1),
      '#title' => t('Display setting'),
      '#required' => TRUE,
      '#options' => $opt
    );
    $form['photos']['page']['album']['full_viewnum'] = array(
      '#type' => 'select',
      '#default_value' => !empty($node->album['full_viewnum']) ? $node->album['full_viewnum'] : variable_get('photos_display_full_viewnum', 10),
      '#title' => t('Display quantity'),
      '#required' => TRUE,
      '#options' => drupal_map_assoc(array(5, 10, 15, 20, 25, 30, 35, 40, 45, 50)),
      '#prefix' => '<div class="photos-form-count">'
    );
    $form['photos']['page']['album']['full_imagesize'] = array(
      '#type' => 'select',
      '#title' => t('Image display size'),
      '#required' => TRUE,
      '#default_value' => $full_imagesize,
      '#options' => _photos_select_size(),
      '#suffix' => '</div>'
    );
    $form['photos']['page']['album']['page_slide'] = array(
      '#type' => 'textfield',
      '#default_value' => !empty($node->album['page_slide']) ? $node->album['page_slide'] : variable_get('photos_display_page_slide', '640x480'),
      '#title' => t('Slide width and height'),
      '#required' => TRUE,
      '#description' => t('The width and height of flash, e.g: 640x480.'),
      '#prefix' => '<div class="photos-form-slide">',
      '#suffix' => '</div>'
    );
  }

  if (variable_get('photos_display_teaser_user', 1)) {
    $form['photos']['teaser']['album'] = array(
      '#type' => 'fieldset',
      '#title' => t('Teaser Settings'),
      '#collapsible' => TRUE,
      '#tree' => TRUE,
      '#prefix' => '<div id="photos-form-teaser">',
      '#suffix' => '</div>'
    );
    $form['photos']['teaser']['album']['teaser_display'] = array(
      '#type' => 'radios',
      '#default_value' => !empty($node->album['teaser_display']) ? $node->album['teaser_display'] : variable_get('photos_display_teaser_display', 1),
      '#title' => t('Display setting'),
      '#required' => TRUE,
      '#options' => $opt
    );
    $form['photos']['teaser']['album']['teaser_viewnum'] = array(
      '#type' => 'select',
      '#default_value' => !empty($node->album['teaser_viewnum']) ? $node->album['teaser_viewnum'] : variable_get('photos_display_teaser_viewnum', 5),
      '#title' => t('Display quantity'),
      '#required' => TRUE,
      '#options' => drupal_map_assoc(array(1, 2, 3, 4, 5, 6, 7, 8, 9, 10)),
      '#prefix' => '<div class="photos-form-count">'
    );
    $form['photos']['teaser']['album']['teaser_imagesize'] = array(
      '#type' => 'select',
      '#title' => t('Image display size'),
      '#required' => TRUE,
      '#default_value' => $teaser_imagesize,
      '#options' => _photos_select_size(),
      '#suffix' => '</div>'
    );
    $form['photos']['teaser']['album']['teaser_slide'] = array(
      '#type' => 'textfield',
      '#default_value' => !empty($node->album['teaser_slide']) ? $node->album['teaser_slide'] : variable_get('photos_display_teaser_slide', '640x480'),
      '#title' => t('Slide width and height'),
      '#required' => TRUE,
      '#description' => t('The width and height of flash, e.g: 640x480.'),
      '#prefix' => '<div class="photos-form-slide">',
      '#suffix' => '</div>'
    );
  }

  $pid = !empty($node->album['pid']) ? $node->album['pid'] : NULL;
  $form['photos']['page']['album']['pid'] = array(
    '#type' => 'value',
    '#value' => $pid
  );

  return $form;
}