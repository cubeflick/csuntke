<?php
/**
 * @file
 * Formats album layout.
 */
?>
<?php if(isset($album['album_url']) || isset($album['node_url'])): ?>
<div class="photos_album_menu">
  <?php if (module_exists('dfgallery')): ?>
    <div class="photos_album_float_right">
      <a href="<?php print $album['album_url'];?>/slide"><?php print t('Slideshow');?></a>
      (<a href="<?php print $album['album_url'];?>/slide" target="_blank"><?php print t('Open a new window');?></a>)
    </div>
  <?php endif; ?>
  <?php if ($album['access']['edit']): ?>
    <a href="<?php print $album['node_url']; ?>/edit"><?php print t('Album settings');?></a>
    <a href="<?php print $album['node_url']; ?>/photos"><?php print t('Images Management');?></a>
  <?php endif; ?>
</div>
<?php endif; ?>
<?php if (isset($node->type) && $node->type == 'photos'): ?>
  <div class="photos_album_header">
    <div class="photos_album_cover">
      <?php if (isset($node->album['cover']['url'])): ?>
      <?php
        $album_cover = theme('image_style', array(
          'style_name' => 'thumbnail',
          'path' => $node->album['cover']['url'],
          'alt' => $node->title,
          'title' => $node->title
        ));
        print $album_cover;?>
      <?php endif; ?>
    </div>
    <h2><?php print $node->title;?></h2>
    <?php if (isset($node->teaser)): ?>
      <?php print $node->teaser; ?>
    <?php endif; ?>
    <div class="photos_album_right">
      <?php print t('!cou images', array('!cou' => $node->album['count']));?>
      <div class="submitted">
        <?php print t('Submitted by !name on !date', array('!name' => $node->name, '!date' => format_date($node->created)));?>
      </div>
    </div>
  </div>
<?php endif; ?>

<?php print $album['links'];?>
<?php if (isset($album['pager'])): ?>
  <?php print theme_pager($album['pager']);?>
<?php endif; ?>
<?php
  foreach ($album['view'] as $view) {
    print $view;
  }
?>
<?php if (isset($album['pager'])): ?>
  <?php print theme_pager($album['pager']); ?>
<?php endif; ?>