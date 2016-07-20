<?php
/**
 * @file
 * Adyen response interface.
 */

namespace Commerce\Adyen;

/**
 * Interface ResponseInterface.
 *
 * @package Commerce\Adyen
 */
interface ResponseInterface {

  /**
   * Returns type of response.
   *
   * Returned value will be concatenated with a "commerce/adyen"
   * path and, when URL visited, object will be initialized.
   *
   * @see commerce_adyen_responses()
   *
   * @code
   * public static function type() {
   *   return 'test';
   * }
   * @endcode
   *
   * @example
   * Result: "commerce/adyen/test".
   *
   * @return string
   *   Response type.
   */
  public static function type();

  /**
   * Resulting response for this URL.
   *
   * @return string
   *   Response string.
   */
  public function __toString();

  /**
   * ResponseInterface constructor.
   *
   * @param array $data
   *   The "$_REQUEST" superglobal variable will be passed.
   *
   * @see commerce_adyen_controller()
   */
  public function __construct(array $data);

}
