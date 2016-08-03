<?php
/**
 * @file
 * An interface which should be implemented by class which requires signature.
 */

namespace Commerce\Adyen;

/**
 * Class SignatureInterface.
 *
 * @package Commerce\Adyen
 */
interface SignatureInterface {

  /**
   * Returns a list of fields for signature creation.
   *
   * @return string[]
   *   List of signature fields.
   */
  public static function signatureFields();

}
