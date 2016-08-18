<?php
/**
 * @file
 * Commerce Adyen Payment Transaction.
 */

namespace Commerce\Adyen\Payment;

use Commerce\Utils;

/**
 * Class Transaction.
 *
 * @package Commerce\Adyen\Payment
 */
class Transaction extends Utils\Transaction {

  /**
   * {@inheritdoc}
   */
  public function getPaymentMethodName() {
    return COMMERCE_ADYEN_PAYMENT_METHOD;
  }

  /**
   * {@inheritdoc}
   */
  public function getPaymentInstanceId() {
    return COMMERCE_ADYEN_PAYMENT_METHOD_INSTANCE;
  }

  /**
   * Authorise a transaction.
   *
   * @param string|int $remote_id
   *   Remote identifier of a payment transaction.
   */
  public function authorise($remote_id) {
    $this->setRemoteId($remote_id);
    $this->setStatus(COMMERCE_ADYEN_PAYMENT_STATUS_AUTHORISED);
    $this->setRemoteStatus(COMMERCE_ADYEN_PAYMENT_REMOTE_STATUS_AUTHORISED);
    $this->setMessage('Payment has been successfully authorised.');
  }

  /**
   * Check if payment was only authorised and waiting for capturing.
   *
   * @return bool
   *   A state of check.
   */
  public function isAuthorised() {
    return
      $this->getStatus() === COMMERCE_ADYEN_PAYMENT_STATUS_AUTHORISED &&
      $this->getRemoteStatus() === COMMERCE_ADYEN_PAYMENT_REMOTE_STATUS_AUTHORISED;
  }

  /**
   * Pending a transaction.
   *
   * @param string|int $remote_id
   *   Remote identifier of a payment transaction.
   */
  public function pending($remote_id) {
    $this->setRemoteId($remote_id);
    $this->setStatus(COMMERCE_PAYMENT_STATUS_PENDING);
    $this->setRemoteStatus(COMMERCE_ADYEN_PAYMENT_REMOTE_STATUS_PENDING);
    $this->setMessage('Payment has been successfully set to pending');
  }

  /**
   * Check if payment was only pending and waiting for authorised.
   *
   * @return bool
   *   A state of check.
   */
  public function isPending() {
    return
      $this->getStatus() === COMMERCE_PAYMENT_STATUS_PENDING &&
      $this->getRemoteStatus() === COMMERCE_ADYEN_PAYMENT_REMOTE_STATUS_PENDING;
  }

  /**
   * Fail a transaction.
   *
   * @param string|int $remote_id
   *   Remote identifier of a payment transaction.
   */
  public function fail($remote_id) {
    $this->setRemoteId($remote_id);
    $this->setStatus(COMMERCE_PAYMENT_STATUS_FAILURE);
    $this->setRemoteStatus(COMMERCE_ADYEN_PAYMENT_REMOTE_STATUS_FAILURE);
    $this->setMessage('Payment failed.');
  }

  /**
   * Check if payment failed.
   *
   * @return bool
   *   A state of check.
   */
  public function isFailed() {
    return
      $this->getStatus() === COMMERCE_PAYMENT_STATUS_FAILURE &&
      $this->getRemoteStatus() === COMMERCE_ADYEN_PAYMENT_REMOTE_STATUS_FAILURE;
  }

  /**
   * Finalize a transaction. Must be used only after successful capturing.
   */
  public function finalize() {
    $this->setStatus(COMMERCE_PAYMENT_STATUS_SUCCESS);
    $this->setRemoteStatus(COMMERCE_ADYEN_PAYMENT_REMOTE_STATUS_CAPTURED);
    $this->setMessage('Payment has been captured and completed.');
  }

  /**
   * Check if payment has been finalized.
   *
   * @return bool
   *   A state of check.
   */
  public function isFinalized() {
    return
      $this->getStatus() === COMMERCE_PAYMENT_STATUS_SUCCESS &&
      $this->getRemoteStatus() === COMMERCE_ADYEN_PAYMENT_REMOTE_STATUS_CAPTURED;
  }

}
