<?php

function migrate_swag_terms_install() {
  //migrate_swag_terms_vocabularies();
}

function migrate_swag_terms_vocabularies() {
  $description = st('Terms to categorize images.');
  $help = st('Please accurately select the cateogy your image falls into.  If multiple categories apply, you may hold ctrl to select multiple.');
  $vocabulary = (object) array(
    'name' => 'Image Galleries',
    'description' => $description,
    'machine_name' => 'image_galleries',
    'help' => $help,
  );
  taxonomy_vocabulary_save($vocabulary);

  $description = st('');
  $help = st('');
  $vocabulary = (object) array(
    'name' => 'Forums',
    'description' => $description,
    'machine_name' => 'forum',
    'help' => $help,
  );
  taxonomy_vocabulary_save($vocabulary);

  $description = st('');
  $help = st('');
  $vocabulary = (object) array(
    'name' => 'Blog Content',
    'description' => $description,
    'machine_name' => 'blog_content',
    'help' => $help,
  );
  taxonomy_vocabulary_save($vocabulary);
}
