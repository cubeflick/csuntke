version:7.x-2.x

INSTALL
-------

  *Depends on core image and file module(s).
  *Content type photos created automatically as primary album type.
  *Configure global settings: admin/config/media/photos
  *Update permissions.
  *For inline editing photo title and description:
    1. Install the Libraries API 2.x module.
    2. Save jquery.jeditable.js AND jquery.jeditable.mini.js from http://www.appelsiini.net/projects/jeditable.
    3. Add both files to sitea/all/libraries/jeditable.


UPGRADE
-------

  *Upgrade from D6 to D7.
  *Files will be transfered from old core files database table to new file_managed table.
  *Backup database! Backup files table before upgrade.
  *Database tables are renamed from x_* to photos_* (update any custom modules / code).
  *When upgrading from drupal 6 to drupal 7 your images will be rebuilt
    with the core image caching system. Warning: do not delete your old
    image directories, they could still be in use by your users external
    links and shared galleries, etc.
  *Update core image styles to meet your needs.
  *Update gallery settings to use new image sizes: admin/config/media/photos.


FEATURES
--------

  *Create photo galleries.
  *Upload and manage images.
  *Comment on images.
  *Integrates with core image styles.