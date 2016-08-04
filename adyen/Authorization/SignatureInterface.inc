<?php
/**
 * @file
 * An interface which should be implemented by class which requires signature.
 */

namespace Commerce\Adyen\Payment\Authorization;

/**
 * Interface SignatureInterface.
 *
 * @package Commerce\Adyen\Payment\Authorization
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
