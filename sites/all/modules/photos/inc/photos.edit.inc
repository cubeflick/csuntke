<?php
/**
 * @file
 * Handles uploading and editing images.
 */

/**
 * Photos upload and edit page.
 */
function photos_edit_upload() {
  global $user;
  $v = '';
  $header = array(
    array('data' => t('Album title'), 'field' => 'n.title'),
    array('data' => t('Image count'), 'field' => 'p.count'),
    array('data' => t('Link')),
    array('data' => t('Create time'), 'field' => 'n.nid', 'sort' => 'desc'),
  );
  $query = db_select('node', 'n')->extend('TableSort')->extend('PagerDefault');
  $query->join('photos_album', 'p', 'p.pid = n.nid');
  $query->fields('n')
    ->fields('p', array('count'));
  $query->condition('n.uid', $user->uid);
  $query->orderByHeader($header);
  $query->limit(20);
  $result = $query->execute();

  foreach ($result as $node) {
    $slideshow = '';
    if (module_exists('dfgallery')) {
      // @todo add check and support for other slideshow options.
      $slideshow = l(t('Slideshow'), 'photos/Slide/' . $node->nid);
    }
    $table[] = array(
      l($node->title, 'node/' . $node->nid),
      $node->count,
      array(
        'data' => l(t('Upload'), 'node/' . $node->nid . '/photos') . l(t('View'), 'photos/album/' . $node->nid) . $slideshow,
        'class' => 'photos_album_edit_table_link'
      ),
      format_date($node->created, 'small')
    );
  }
  $edit = array('submit' => TRUE);
  $upload_form = drupal_get_form('photos_upload_form', 0, $edit);
  $v = drupal_render($upload_form);
  if (empty($table)) {
    $table[] = array(array('data' => t('Please create an album.'), 'colspan' => 4));
  }
  else {
    $v .= t('Or select an album') . ': ';
  }
  $v .= theme('table', array('header' => $header, 'rows' => $table));
  $v .= theme('pager');

  return $v;
}

/**
 * Upload form.
 */
//图片上传
function photos_upload_form($form, &$form_state, $node = FALSE, $edit = array()) {
  global $user;
  $form['new'] = array(
    '#title' => t('Image upload'),
    '#weight' => -4,
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#description' => t('Allow the type:') . ' jpg gif png jpeg ' . ((variable_get('photos_upzip', 0) && $_SESSION['photos_swfu_switch']) ? ' zip' : NULL),
  );
  if (isset($node->type) && $node->type == 'photos' || photos_get_count('user_album', $user->uid)) {
    for ($i = 0; $i < variable_get('photos_num', 5); ++$i) {
      $form['new']['images_' . $i] = array(
        '#type' => 'file'
      );
      $form['new']['title_' . $i] = array(
        '#type' => 'textfield',
        '#title' => t('Image title'),
      );
      $form['new']['des_' . $i] = array(
        '#type' => 'textarea',
        '#title' => t('Image description'),
        '#cols' => 40,
        '#rows' => 3,
      );
    }
  }
  else {
    drupal_set_message(t('You must first !create an album to upload images.', array('!create' => l(t('create'), 'node/add/photos', array('query' => drupal_get_destination())))));
      return $form;
  }
  if (isset($node->type) && $node->type == 'photos') {
    $form['new']['pid'] = array(
      '#type' => 'value',
      '#value' => $node->nid
    );
  }
  else {
    $form['new']['pid'] = array(
      '#title' => t('Upload to album'),
      '#type' => 'select',
      '#options' => _photos_useralbum_option($user->uid),
      '#default_value' => isset($_GET['pid']) ? $_GET['pid'] : NULL,
      '#required' => TRUE,
      '#prefix' => '<div id="photos-pid">',
      '#suffix' => '</div>',
      '#weight' => -5,
    );
  }
  if (isset($node->nid)) {
    $form['new']['nid'] = array(
      '#type' => 'value',
      '#value' => $node->nid
    );
  }
  if ($edit['submit']) {
    $form['new']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Confirm upload'),
      '#weight' => 10,
      '#submit' => array('photos_upload_form_submit'),
    );
    $form['#action'] = url($_GET['q'], array('query' => drupal_get_destination()));
    $form['#attributes']['enctype'] = 'multipart/form-data';
  }

  return $form;
}

/**
 * Submit upload form.
 */
function photos_upload_form_submit($form, &$form_state) {
  global $user;
  // $limits = _upload_file_limits($user); // @todo set limits for maximum upload size etc.
  $validators = array(
    'file_validate_is_image' => array(),
    // 'file_validate_image_resolution' => array($limits['resolution']),
    // 'file_validate_size' => array($limits['file_size'], $limits['user_size'])
  );
  $photos_num = variable_get('photos_num', 5);
  for ($i = 0; $i < $photos_num; ++$i) {
    _photos_rename();
    if ($_FILES['files']['name']['images_' . $i]) {
      $value = new stdClass();
      $value->pid = $form_state['values']['pid'];
      $value->nid = isset($form_state['values']['nid']) ? $form_state['values']['nid'] : $form_state['values']['pid'];
      $value->des = $form_state['values']['des_' . $i];
      $value->title = $form_state['values']['title_' . $i];
      $ext = drupal_substr($_FILES['files']['name']['images_' . $i], -3);
      if ($ext != 'zip' && $ext != 'ZIP') {
        if ($file = file_save_upload('images_' . $i, $validators, photos_check_path())) {
          $msg[] = photos_image_date((object)array_merge((array)$file, (array)$value), 1);
        }
      }
      else {
        if (!variable_get('photos_upzip', 0)) {
          return form_set_error('error', t('Please set Album photos to open zip upload'));
        }
        $directory = photos_check_path();
        file_prepare_directory($directory);
        $zip = file_destination($directory . '/' . trim(basename($_FILES['files']['name']['images_' . $i])), FILE_EXISTS_RENAME);
        if (move_uploaded_file($_FILES['files']['tmp_name']['images_' . $i], $zip)) {
          if (!$msg = _photos_unzip($zip, $value)) {
            $msg = t('Failed to upload');
          }
        }
      }
    }
  }
  if (!empty($msg) && is_array($msg)) {
    return drupal_set_message(t('%count image(s) uploaded successfully.', array('%count' => count($msg))));
  }
  else {
    if (isset($msg)) {
      return drupal_set_message($msg);
    }
    else {
      return FALSE;
    }
  }
}

/**
 * Image management. (图片管理)
 */
function photos_edit_page($node) {
  $output = '';
  if ($node->type == 'photos') {
    $edit_form = drupal_get_form('photos_upload_form', $node, array('submit' => TRUE));
    $output = '<div id="photos_upload_new">' . drupal_render($edit_form) . '</div>';
    $output .= _photos_edit_page_album($node);
    $page_title = t('Organize Albums: @title', array('@title' => $node->title));
    drupal_set_title($page_title);
  }
  else {
    $output .= _photos_edit_page_node($node);
    $page_title = t('Organize Sub-Album: @title', array('@title' => $node->title));
    drupal_set_title($page_title);
  }

  return $output;
}

/**
 * Sub album image management page.
 */
function _photos_edit_page_node($node) {
  $output = '';
  if ($node->subalbum['count']) {
    $column = isset($_GET['field']) ? $_GET['field'] : '';
    $sort = isset($_GET['sort']) ? $_GET['sort'] : '';
    $term = _photos_order_value($column, $sort, 10);
    $query = db_select('file_managed', 'f')
      ->extend('PagerDefault');
    $query->join('photos_node', 'a', 'a.fid = f.fid');
    $query->join('photos_image', 'p', 'p.fid = f.fid');
    $query->join('node', 'n', 'n.nid = p.pid');
    $query->join('users', 'u', 'u.uid = f.uid');
    $query->fields('f', array('uri', 'filemime', 'timestamp', 'filename', 'filesize'));
    $query->fields('p');
    $query->fields('n', array('title'));
    $query->fields('u', array('uid', 'name'));
    $query->condition('a.nid', $node->nid);
    $query->orderBy($term['order']['column'], $term['order']['sort']);
    $query->limit($term['limit']);
    $result = $query->execute();
    $output .= _photos_order_link('node/' . $node->nid . '/photos', $node->subalbum['count'], l(t('Album view'), 'photos/sub_album/' . $node->nid), 1);
    $output .= '<div class="messages">' . t('Move out: Move image out of the sub gallery, but it will not delete it.') . '</div>';
  }
  else {
    return;
  }
  foreach ($result as $data) {
    $images[] = photos_get_info(0, $data);
  }
  if ($images[0]->fid) {
    $images[0]->info = array(
      'title' => $node->title,
      'nid' => $node->nid,
      'uid' => $node->uid
    );
    $editlist_form = drupal_get_form('photos_editlist_form', $images, 'node');
    $output .= theme('pager');
    $output .= drupal_render($editlist_form);
    $output .= theme('pager');
  }

  return $output;
}

/**
 * Album image management page.
 */
function _photos_edit_page_album($node) {
  $output = '';
  if (isset($node->album['count'])) {
    $album_count = $node->album['count'];
  }
  else {
    $album_count = 0;
  }
  if (isset($node->album['count'])) {
    $column = isset($_GET['field']) ? $_GET['field'] : '';
    $sort = isset($_GET['sort']) ? $_GET['sort'] : '';
    $term = _photos_order_value($column, $sort, 10);
    $query = db_select('file_managed', 'f')
      ->extend('PagerDefault');
    $query->join('photos_image', 'p', 'p.fid = f.fid');
    $query->join('users', 'u', 'f.uid = u.uid');
    $query->fields('f', array('uri', 'filemime', 'timestamp', 'filename', 'filesize'));
    $query->fields('p');
    $query->fields('u', array('uid', 'name'));
    $query->condition('p.pid', $node->nid);
    $query->limit($term['limit']);
    $query->orderBy($term['order']['column'], $term['order']['sort']);
    $result = $query->execute();

    $output .= _photos_order_link('node/' . $node->nid . '/photos', $node->album['count'], l(t('Album view'), 'photos/album/' . $node->nid), 1);
  }
  else {
    return;
  }
  foreach ($result as $data) {
    $images[] = photos_get_info(0, $data);
  }
  if (isset($images[0]->fid)) {
    $images[0]->info = array(
      'cover' => $node->album['cover'],
      'pid' => $node->nid,
      'title' => $node->title,
      'uid' => $node->uid
    );
    $output .= theme('pager');
    $edit_form = drupal_get_form('photos_editlist_form', $images);
    $output .= drupal_render($edit_form);
    $output .= theme('pager');
  }

  return $output;
}

/**
 * Album image management page.
 */
function _photos_edit_page_single_image($fid) {
  $output = '';
  $images = array();
  $query = db_select('file_managed', 'f');
  $query->join('photos_image', 'p', 'p.fid = f.fid');
  $query->join('users', 'u', 'f.uid = u.uid');
  $query->fields('f', array('uri', 'filemime', 'timestamp', 'filename', 'filesize'));
  $query->fields('p');
  $query->fields('u', array('uid', 'name'));
  $query->condition('f.fid', $fid);
  $result = $query->execute();

  foreach ($result as $data) {
    $images[] = photos_get_info(0, $data);
  }

  if (isset($images[0]->fid)) {
    $node = node_load($images[0]->pid);
    $images[0]->info = array(
      'cover' => $node->album['cover'],
      'pid' => $node->nid,
      'title' => $node->title,
      'uid' => $node->uid
    );
    $edit_form = drupal_get_form('photos_editlist_form', $images);
    $output .= drupal_render($edit_form);
  }

  if (module_exists('colorbox')) {
    // Display in modal colorbox.
    print $output;
  }
  else {
    // Full page.
    return $output;
  }
}

/**
 * Edit list form.
 */
function photos_editlist_form($form, &$form_state, $images = array(), $type = 'album') {
  global $user;
  if ($type == 'album') {
    if ($user->uid <> $images[0]->info['uid']) {
      $album_update = array($images[0]->info['pid'], $images[0]->info['title']);
    }
    else {
      $album_update = '';
    }
    $album_pid = _photos_useralbum_option($user->uid, $album_update);
    $del_label = _photos_del_checkbox(0, t('Delete'));
    $form['cover_fid'] = array('#type' => 'hidden', '#default_value' => $images[0]->fid);
    $form['oldpid'] = array('#type' => 'hidden', '#default_value' => $images[0]->info['pid']);
    $submit = 'photos_editlist_submit';
  }
  else {
    $del_label = _photos_del_checkbox(0, t('Move out'));
    $submit = 'photos_editlist_submit_node';
    $form['nid'] = array('#type' => 'hidden', '#default_value' => $images[0]->info['nid']);
  }
  $form['photos']['#tree'] = TRUE;
  // $form['#action'] = url($_GET['q'], array('query' => drupal_get_destination(). (isset($_GET['page']) ? '&page=' . intval($_GET['page']): NULL)));
  $thumb = variable_get('photos_title_0', FALSE);
  foreach ($images as $image) {
    $form['photos'][$image->fid]['del'] = $del_label;
    $image->user = user_load($image->uid);
    $image->href = 'photos/image/' . $image->fid;
    $item = array();
    $title = $image->filename;
    $style_name = variable_get('photos_thumb_size', 'thumbnail');
    $item[] = l(theme('image_style', array('style_name' => $style_name, 'path' => $image->uri, 'alt' => $title, 'title' => $title)), $image->href, array('html' => TRUE, 'attributes' => array('title' => $title)));
    if ($type == 'album' && $images[0]->fid <> $image->fid) {
      $item[] = l(t('Set to Cover'), 'node/' . $image->pid . '/photos/cover/' . $image->fid);
    }
    if (isset($image->filesize)) {
      $item[] = t('Filesize: !size KB', array('!size' => round($image->filesize/1024)));
    }
    if (isset($image->count)) {
      $item[] = t('Visits: !count', array('!count' => $image->count));
    }
    if (isset($image->comcount)) {
      $item[] = t('Comments: !count', array('!count' => $image->comcount));
    }
    $form['photos'][$image->fid]['path']['#markup'] = theme('item_list', array('items' => $item));
    $form['photos'][$image->fid]['des'] = array(
      '#title' => t('Image description'),
      '#type' => 'textarea',
      '#default_value' => isset($image->des) ? preg_replace('/<.*?>/', '', $image->des) : '',
      '#cols' => 40,
      '#rows' => 4
    );
    $form['photos'][$image->fid]['filename'] = array(
      '#title' => t('Image title'),
      '#type' => 'textfield',
      '#default_value' => isset($image->filename) ? $image->filename : '',
      '#required' => TRUE
    );
    $form['photos'][$image->fid]['wid'] = array(
      '#title' => t('Weight'),
      '#type' => 'weight',
      '#default_value' => isset($image->wid) ? $image->wid : NULL,
      '#delta' => 100,
    );
    $form['photos'][$image->fid]['filepath'] = array('#type' => 'value', '#value' => $image->uri);
    if ($type == 'album') {
      $upload_info = t('Uploaded on !time by !name', array('!name' => theme('username', array('account' => $image->user)), '!time' => format_date($image->timestamp, 'small')));
      $form['photos'][$image->fid]['pid'] = array(
        '#title' => t('Move to the album'),
        '#type' => 'select',
        '#options' => $album_pid,
        '#default_value' => $image->pid,
        '#required' => TRUE
      );
    }
    else {
      $upload_info = t('!name in !time upload to !title', array('!name' => theme('username', array('account' => $image->user)), '!time' => format_date($image->timestamp, 'small'), '!title' => l($image->title, 'node/' . $image->pid)));
    }
    $form['photos'][$image->fid]['time']['#markup'] = $upload_info;
    $form['photos'][$image->fid]['uid'] = array('#type' => 'hidden', '#default_value' => $image->uid);
    $form['photos'][$image->fid]['oldfilename'] = array('#type' => 'hidden', '#default_value' => $image->filename);
  };
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Confirm changes'),
    '#submit' => array($submit),
  );

  return $form;
}

/**
 * Edit list form submit. (节点下提交处理)
 */
function photos_editlist_submit_node($form, &$form_state) {
  foreach ($form_state['values']['photos'] as $fid => $key) {
    if (!empty($key['del'])) {
      $msg[] = db_query('DELETE FROM {photos_node} WHERE fid = :fid',
        array(':fid' => $fid));
    }
    else {
      db_query("UPDATE {photos_image} SET des = :des, wid = :wid WHERE fid = :fid",
        array(':des' => $key['des'], ':wid' => $key['wid'], ':fid' => $fid));
      if ($key['filename'] != $key['oldfilename']) {
        db_query("UPDATE {file_managed} SET filename = :filename WHERE fid = :fid",
          array(':filename' => check_plain($key['filename']), ':fid' => $fid));
      }
    }
  }
  if ($msg) {
    photos_set_count('node_node', $form_state['values']['nid']);
    drupal_set_message(t('%count images are move out.', array('%count' => count($msg))));
  }
}

/**
 * Submit modifications to existing images. (相册下提交处理)
 */
function photos_editlist_submit($form, &$form_state) {
  foreach ($form_state['values']['photos'] as $fid => $key) {
    if (!empty($key['del'])) {
      if ($form_state['values']['cover_fid'] == $fid) {
        db_update('photos_album')
          ->fields(array(
            'fid' => 0
          ))
          ->condition('pid', $form_state['values']['oldpid'])
          ->execute();
      }
      $msg[] = photos_file_del($fid, $key['filepath']);
      $uids[] = $key['uid'];
    }
    else {
      db_update('photos_image')
        ->fields(array(
          'pid' => $key['pid'],
          'des' => $key['des'],
          'wid' => $key['wid']
        ))
        ->condition('fid', $fid)
        ->execute();

      if ($key['filename'] != $key['oldfilename']) {
        db_update('file_managed')
          ->fields(array(
            'filename' => check_plain($key['filename'])
          ))
          ->condition('fid', $fid)
          ->execute();
      }
      if ($key['pid'] != $form_state['values']['oldpid']) {
        $sub_select = db_select('photos_vote', 'v')
          ->fields('v', array('cid'))
          ->condition('v.fid', $fid)
          ->execute()->fetchCol();
        db_update('comment')
          ->fields(array(
            'nid' => $key['pid']
          ))
          ->condition('cid', $sub_select, 'IN')
          ->execute();
        $pid[] = $key['pid'];
        $uids[] = $key['uid'];
      }
    }
  }
  if (isset($msg)) {
    $pid[] = $form_state['values']['oldpid'];
    drupal_set_message(t('%count images are deleted.', array('%count' => count($msg))));
  }
  if (isset($pid) && count($pid)) {
    foreach ($pid as $nid) {
      _comment_update_node_statistics($nid);
      photos_set_count('node_album', $nid);
    }
    $uid = array_unique($uids);
    foreach ($uid as $id) {
      photos_set_count('user_image', $id);
    }
  }
}

/**
 * Handles image edit list delete checkbox.
 */
function _photos_del_checkbox($t = 0, $title = FALSE) {
  if ($t) {
    return (($t == 1) ? t('Delete') : $t) . '<input type="checkbox" id="del_check" />';
  }
  else {
    return array('#title' => $title . ' ', '#type' => 'checkbox', '#prefix' => '<div class="edit-del-all">', '#suffix' => '</div>');
  }
}

/**
 * Theme photos edit list.
 */
function theme_photos_editlist_form($variables) {
  $form = $variables['form'];
  $header = array(
    array('data' => _photos_del_checkbox(t('Select All'))),
    array('data' => t('Thumbnails')),
    array('data' => t('Content')),
  );
  foreach (element_children($form['photos']) as $key) {
    $rows[] = array(
      drupal_render($form['photos'][$key]['wid']) . drupal_render($form['photos'][$key]['del']),
      array(
        'data' => drupal_render($form['photos'][$key]['path']),
        'class' => 'photos_edit_thumb'
      ),
      '<div class="photos-edit-filename">' . drupal_render($form['photos'][$key]['filename']) . '</div><div class="photos-edit-des">' . drupal_render($form['photos'][$key]['des']) . '</div><div class="photos-edit-info">' . drupal_render($form['photos'][$key]['time']) . drupal_render($form['photos'][$key]['pid']) . '</div>'
    );
  }
  if (!$rows) {
    $rows[] = array(array('data' => t('No image'), 'colspan' => '3'));
  }
  $output = theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'photos_album_edit')));
  $output .= drupal_render_children($form);

  return $output;
}

/**
 * Update album cover. (设置封面)
 */
function photos_edit_cover($node, $fid) {
  $pid = db_query('SELECT pid FROM {photos_image} WHERE fid = :fid', array(':fid' => $fid))->fetchField();
  if ($pid == $node->nid) {
    db_query('UPDATE {photos_album} SET fid = :fid WHERE pid = :pid',
      array(':fid' => $fid, ':pid' => $node->nid));
    drupal_set_message(t('Cover successfully set.'));
    $goto = $_GET['destination'] ? $_GET['destination'] : 'node/' . $node->nid . '/photos';

    return drupal_goto($goto);
  }
  else {
    drupal_not_found();
  }
}

/**
 * Ajax edit image. (ajax更新图片信息)
 */
function photos_edit_update($fid = NULL) {
  drupal_add_http_header('Content-Type', 'text/plain;');
  $value = trim(check_plain($_POST['value']));
  $id = check_plain($_POST['id']);
  if (strstr($id, 'filename')) {
    $switch = 'filename';
    $fid = str_replace('photos-image-edit-filename-', '', $id);
  }
  elseif (strstr($id, 'des')) {
    $switch = 'des';
    $fid = str_replace('photos-image-edit-des-', '', $id);
  }
  // Validate token and check user image edit permissions.
  if (drupal_valid_token($_POST['token'], 'image_edit_token') && _photos_access('imageEdit', $fid)) {
    switch ($switch) {
      case 'filename':
        db_update('file_managed')
          ->fields(array(
            'filename' => $value
          ))
          ->condition('fid', $fid)
          ->execute();
        echo $value;
      break;
      case 'des':
        db_update('photos_image')
          ->fields(array(
            'des' => $value
          ))
          ->condition('fid', $fid)
          ->execute();
        echo $value;
      break;
      case 'del':
        // echo photos_file_del($fid, 0, 1);
    }
  }
}

/**
 * Delete photo.
 */
function photos_edit_delete($fid) {
  if (module_exists('colorbox')) {
    // Dispaly form in modal popup.
    $confirm_delete_form = drupal_get_form('photos_edit_confirm_delete', $fid);
    print drupal_render($confirm_delete_form);
  }
  else {
    // Render full page.
    return drupal_get_form('photos_edit_confirm_delete', $fid);
  }
}

/**
 * Confirm delete photo.
 */
function photos_edit_confirm_delete($form, &$form_state, $fid) {
  $type = isset($_GET['type']) ? check_plain($_GET['type']) : '';
  $form['fid'] = array(
    '#type' => 'value',
    '#value' => $fid,
  );
  $form['type'] = array(
    '#type' => 'value',
    '#value' => $type,
  );
  if ($type <> 'sub_album') {
    $description = t('This photo will be deleted from this gallery and all sub galleries along with all comments, title and description.');
    $remove = t('Delete');
  }
  else {
    $description = t('This photo will be moved out of this sub gallery only.');
    $remove = t('Move out');
  }
  return confirm_form($form, t('Confirm and delete this photo.'), 'photos/image/' . $fid, $description, $remove, t('Cancel'));
}

/**
 * Submit confirm delete photo.
 */
function photos_edit_confirm_delete_submit($form, &$form_state) {
  $fid = $form_state['values']['fid'];
  $type = isset($form_state['values']['type']) ? $form_state['values']['type'] : '';

  if ($type <> 'sub_album') {
    $v = photos_file_del($fid, 0, 1);
    if (isset($_GET['pid']) && intval($_GET['pid']) == $_GET['pid']) photos_set_count('node_album', $_GET['pid']);
    if (isset($_GET['uid']) && intval($_GET['uid']) == $_GET['uid']) photos_set_count('user_image', $_GET['uid']);
  }
  else {
    // Remove from sub album.
    $v = db_delete('photos_node')
      ->condition('fid', $fid)
      ->execute();
    if (isset($_GET['nid']) && intval($_GET['nid']) == $_GET['nid']) photos_set_count('node_node', $_GET['nid']);
  }
  if (isset($_GET['go']) && $_GET['go'] == 1) {
    drupal_add_http_header('Content-Type:', 'text/plain;');
    echo $v;
  }
  elseif ($v) {
    drupal_set_message(t('Delete success'));
    $form_state['redirect'] = $_GET['destination'];
    return;
  }
  else {
    drupal_set_message(t('Delete failure'));
    $form_state['redirect'] = $_GET['destination'];
    return;
  }
}

/**
 * Send image to sub album. (发送到子相册)
 */
function photos_edit_to_sub($fid) {
  global $user;
  $photos_to_sub_form = drupal_get_form('_photos_to_sub', $fid);
  $content = drupal_render($photos_to_sub_form);
  $content .= theme('pager');
  print theme('photos_print', array('content' => $content));
}

/**
 * Send to sub album form.
 */
function _photos_to_sub($form, &$form_state, $fid) {
  $select_type = _photos_select_sub_album();
  if ($select_type[0]) {
    $form['title']['#markup'] = '<h2>' . t('Please select sub-album') . ': ' . '</h2>';
    $query = db_select('photos_node', 'p')
      ->fields('p', array('nid'))
      ->condition('fid', $fid);
    $result = $query->execute();
    $select_sub = array();
    foreach ($result as $sub) {
      $select_sub[] = $sub->nid;
    }
    if (!isset($select_sub[0])) $select_sub[] = 0;
    $query = db_select('node', 'n')->extend('PagerDefault');
    $query->fields('n', array('nid', 'title'))
      ->condition('n.type', $select_type, 'IN')
      ->condition('n.nid', $select_sub, 'NOT IN')
      ->limit(50);
    $result = $query->execute();
    $form['sub']['#tree'] = TRUE;
    $true = FALSE;
    foreach ($result as $node) {
      $form['sub'][$node->nid] = array(
        '#type' => 'checkbox',
        '#title' => l($node->title, 'node/' . $node->nid, array('attributes' => array('target' => '_blank'))),
      );
      $true = TRUE;
    }
    if ($true) {
      $form['fid'] = array(
        '#type' => 'value',
        '#value' => $fid
      );
      $form['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Send confirmation'),
        '#submit' => array('_photos_to_sub_submit')
      );
    }
    else {
      $form['help']['#markup'] = t('There are no additional sub albums available.');
    }
  }
  else {
    $form['help']['#markup'] = t('Sub-album feature is not turned on.');
  }

  return $form;
}

/**
 * Submit sub albums and count.
 */
function _photos_to_sub_submit($form, &$form_state) {
  if (!$form_state['values']['fid']) return;
  $query = db_insert('photos_node');
  $nids = array();
  foreach ($form_state['values']['sub'] as $key => $sub) {
    if ($sub) {
      $query->fields(array(
        'nid' => $key,
        'fid' => $form_state['values']['fid']
      ));
      $nids[] = $key;
    }
  }
  if (!empty($nids)) {
    $query->execute();
    foreach ($nids as $nid) {
      photos_set_count('node_node', $nid);
    }
    drupal_set_message(t('Successfully sent to %count sub-album', array('%count' => count($nids))));
    $redirect = array('photos/image/' . $form_state['values']['fid']);
    $form_state['redirect'] = $redirect;
  }
}