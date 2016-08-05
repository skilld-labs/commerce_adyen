<?php
/**
 * @file
 * Authorisation Signature.
 */

namespace Commerce\Adyen\Payment\Authorization;

use Adyen\Util\Util;

/**
 * Class Signature.
 *
 * @package Commerce\Adyen\Payment\Authorization
 */
abstract class Signature implements \Iterator {

  /**
   * @var array
   */
  protected $data = [];
  /**
   * @var \stdClass
   */
  protected $order;
  /**
   * @var array
   */
  protected $paymentMethod = [];

  /**
   * Signature constructor.
   *
   * @param \stdClass $order
   *   Commerce order.
   * @param array $payment_method
   *   Payment method information.
   */
  public function __construct(\stdClass $order, array $payment_method) {
    $this->order = $order;
    $this->paymentMethod = $payment_method;
  }

  /**
   * Returns calculated SHA256 signature.
   *
   * @return string
   *   Calculated SHA256 signature.
   */
  protected function getSignature() {
    $values = [];

    foreach ($this->signatureFields() as $field) {
      if (!empty($this->data[$field])) {
        $values[$field] = $this->data[$field];
      }
    }

    return Util::calculateSha256Signature($this->paymentMethod['settings']['hmac'], $values);
  }

  /**
   * {@inheritdoc}
   */
  public function current() {
    return current($this->data);
  }

  /**
   * {@inheritdoc}
   */
  public function next() {
    next($this->data);
  }

  /**
   * {@inheritdoc}
   */
  public function key() {
    return key($this->data);
  }

  /**
   * {@inheritdoc}
   */
  public function valid() {
    return $this->key() !== NULL;
  }

  /**
   * {@inheritdoc}
   */
  public function rewind() {
    reset($this->data);
  }

  /**
   * Returns a list of fields for signature creation.
   *
   * Values must be in "$this->data" property.
   *
   * @return string[]
   *   List of signature fields.
   */
  abstract protected function signatureFields();

}
