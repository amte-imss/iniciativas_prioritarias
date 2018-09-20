<?php

/**
 * @file
 * H5pScormFile class.
 */

/**
 * Class H5pScormFile.
 */
class H5pScormFile {

  private $result, $field, $filesDirectory, $interface;

  public $type, $name, $path, $mime, $size;

  /**
   * H5pScormFile constructor.
   *
   * @param H5PDrupal $interface
   *   H5PDrupal object.
   * @param string $filesDirectory
   *   Path to files.
   */
  function __construct($interface, $filesDirectory) {
    $field = filter_input(INPUT_POST, 'field', FILTER_SANITIZE_STRING, FILTER_FLAG_NO_ENCODE_QUOTES);

    // Check for file upload.
    if ($field === NULL || empty($_FILES) || !isset($_FILES['file'])) {
      return;
    }

    $this->interface = $interface;
    // Create a new result object.
    $this->result = new stdClass();

    // Set directory.
    $this->filesDirectory = $filesDirectory;

    // Create the temporary directory if it doesn't exist.
    $dirs = array('', '/files');
    foreach ($dirs as $dir) {
      if (!$this->dirReady($this->filesDirectory . $dir)) {
        $this->result->error = $this->interface->t('Unable to create directory.');
        return;
      }
    }

    // Get the field.
    $this->field = json_decode($field);

    // Check if uploaded base64 encoded file.
    if (isset($_POST) && isset($_POST['dataURI']) && $_POST['dataURI'] !== '') {
      $data = $_POST['dataURI'];

      // Extract data from string.
      list($type, $data) = explode(';', $data);
      list(, $data) = explode(',', $data);
      $this->data = base64_decode($data);

      // Extract file type and extension.
      list(, $type) = explode(':', $type);
      list(, $extension) = explode('/', $type);
      $this->type = $type;
      $this->extension = $extension;
      $this->size = strlen($this->data);
    }
    else {

      // Handle temporarily uploaded form file.
      if (function_exists('finfo_file')) {
        $finfo = finfo_open(FILEINFO_MIME_TYPE);
        $this->type = finfo_file($finfo, $_FILES['file']['tmp_name']);
        finfo_close($finfo);
      }
      elseif (function_exists('mime_content_type')) {
        // Deprecated, only when finfo isn't available.
        $this->type = mime_content_type($_FILES['file']['tmp_name']);
      }
      else {
        $this->type = $_FILES['file']['type'];
      }

      $this->extension = pathinfo($_FILES['file']['name'], PATHINFO_EXTENSION);
      $this->size = $_FILES['file']['size'];
    }
  }

  /**
   * Check if file loaded.
   *
   * @return boolean
   *   Returns TRUE if loaded whether returns FALSE.
   */
  public function isLoaded() {
    return is_object($this->result);
  }

  /**
   * Check current file up agains mime types and extensions in the given list.
   *
   * @param array $mimes
   *   List to check against.
   *
   * @return boolean
   *    Returns if checked or not.
   */
  public function check($mimes) {
    $ext = strtolower($this->extension);
    foreach ($mimes as $mime => $extension) {
      if (is_array($extension)) {
        // Multiple extensions.
        if (in_array($ext, $extension)) {
          $this->type = $mime;
          return TRUE;
        }
      }
      elseif (
        $ext === $extension
      ) {
        // TODO: Either remove everything that has to do with mime types.
        // Currently we're experiencing trouble with mime types on different
        // servers.
        $this->type = $mime;
        return TRUE;
      }
    }
    return FALSE;
  }

  /**
   * Validate the file.
   *
   * @return boolean
   *   Returns TRUE if file is valid.
   */
  public function validate() {
    if (isset($this->result->error)) {
      return FALSE;
    }

    // Check for field type.
    if (!isset($this->field->type)) {
      $this->result->error = $this->interface->t('Unable to get field type.');
      return FALSE;
    }

    // Check if mime type is allowed.
    if ((isset($this->field->mimes) && !in_array($this->type, $this->field->mimes)) || substr($this->extension, 0, 3) === 'php') {
      $this->result->error = $this->interface->t(
        'The specified file could not be uploaded. Only files with the following extensions are allowed: !extensions.',
        array(
          '!extensions' => 'zip',
        )
      );

      return FALSE;
    }

    // Type specific validations.
    switch ($this->field->type) {
      default:
        $this->result->error = $this->interface->t('Invalid field type.');
        return FALSE;

      case 'file':
        // TODO: Try to get file extension for type and check that it matches the current extension.
        $this->result->mime = $this->type;
    }

    return TRUE;
  }

  /**
   * Save the file.
   *
   * @return bool
   *   Returns TRUE if file was saved.
   */
  public function copy() {
    $matches = array();
    preg_match('/([a-z0-9]{1,})$/i', $_FILES['file']['name'], $matches);

    $this->name = uniqid($this->field->name . '-');
    $extract_to = $this->filesDirectory . '/' . $this->field->type . 's/' . $this->name;
    $extract_to_path = drupal_realpath($this->filesDirectory) . '/' . $this->field->type . 's/' . $this->name;

    // Add extension to name.
    if (isset($this->data)) {
      $this->name .= '.' . $this->extension;
    }
    else {
      if (isset($matches[0])) {
        $this->name .= '.' . $matches[0];
      }
    }

    $this->name = $this->field->type . 's/' . $this->name;
    $this->path = $this->filesDirectory . '/' . $this->name;

    // Save file to path.
    if (isset($this->data)) {
      // Store base64 file to new path.
      if (file_put_contents($this->path, $this->data) == FALSE) {
        $this->result->error = $this->interface->t('Could not save file.');
      }
    }
    else {
      // Copy tmp file to new path.
      if (!copy($_FILES['file']['tmp_name'], $this->path)) {
        $this->result->error = $this->interface->t('Could not upload file.');
        return FALSE;
      }
    }

    if (!$this->extractScormFile(drupal_realpath($this->path), $extract_to_path)) {
      return FALSE;
    }

    $this->result->path = $this->name;
    $this->result->filename = $_FILES['file']['name'];
    $this->result->extracted = $extract_to;
    $this->result->url = $this->getScormPageInit($extract_to);
    $this->result->pages = $this->getScormPages($extract_to);
    $this->result->params = array();

    if (file_exists($extract_to . DIRECTORY_SEPARATOR . 'tincan.xml')) {
      $this->result->type = H5P_SCORM_TYPE_TINCAN;
    }
    else {
      $this->result->type = H5P_SCORM_TYPE_SCORM;
    }

    return TRUE;
  }

  /**
   * Extract package file.
   *
   * @param string $file_path
   *   File path.
   * @param string $extract_to
   *   Directory path.
   *
   * @return bool
   *   Returns TRUE if file was unzipped.
   */
  protected function extractScormFile($file_path, $extract_to) {
    $instance = new ZipArchive();
    $res = $instance->open($file_path);

    if ($res === TRUE) {
      $instance->extractTo($extract_to);
      $instance->close();

      // Validation for package.
      $manifest_path = $extract_to . DIRECTORY_SEPARATOR . 'imsmanifest.xml';
      $manifest_exists = file_exists($manifest_path);
      $tincan_path = $extract_to . DIRECTORY_SEPARATOR . 'tincan.xml';

      if (!$manifest_exists && !file_exists($tincan_path)) {
        $this->result->error = $this->interface->t('The specified file could not be uploaded. Package does not include all required files.');

        return FALSE;
      }
      elseif ($manifest_exists) {
        // Replace launch href in manifest file.
        $dom = new DomDocument();
        $dom->Load($manifest_path);

        // Get resources href.
        $href_resources = $dom->getElementsByTagName('resource')
          ->item(0)
          ->getAttribute('href');

        // Get file href.
        $href_file = $dom->getElementsByTagName('file')
          ->item(0)
          ->getAttribute('href');

        if ($href_resources == 'launcher.html' && $href_file == 'launcher.html' && file_exists($extract_to . DIRECTORY_SEPARATOR . 'index_lms.html')) {

          // Update attr href.
          $dom->getElementsByTagName('resource')
            ->item(0)->setAttribute('href', 'index_lms.html');

          $dom->Save($manifest_path);
        }
      }

      return TRUE;
    }

    $this->result->error = $this->interface->t('Could not read file.');

    return FALSE;
  }

  /**
   * Get init page.
   *
   * @param string $url
   *   Path to extracted package directory.
   *
   * @return string
   *   Path including the init file.
   */
  protected function getScormPageInit($url) {
    return _h5p_scorm_get_page_init_url($url);
  }

  /**
   * Get list of SCORM pages.
   *
   * @param string $url
   *   Path to extracted package directory.
   *
   * @return array
   *   List of pages.
   */
  protected function getScormPages($url) {
    return _h5p_scorm_get_pages($url);
  }

  /**
   * Get result data.
   *
   * @return string
   *   Result data.
   */
  public function getResult() {
    return json_encode($this->result);
  }

  /**
   * Checks if directory ready.
   *
   * @param string $path
   *   Directory path.
   *
   * @return bool
   *   Flag which states if directory writeable.
   */
  public function dirReady($path) {
    if (!file_exists($path)) {
      $parent = preg_replace("/\/[^\/]+\/?$/", '', $path);
      if (!$this->dirReady($parent)) {
        return FALSE;
      }

      mkdir($path, 0777, TRUE);
    }

    if (!is_dir($path)) {
      trigger_error('Path is not a directory ' . $path, E_USER_WARNING);
      return FALSE;
    }

    if (!is_writable($path)) {
      trigger_error('Unable to write to ' . $path . ' – check directory permissions –', E_USER_WARNING);
      return FALSE;
    }

    return TRUE;
  }

}
