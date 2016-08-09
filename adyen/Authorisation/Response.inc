<?php
/**
 * @file
 * Payment authorisation response.
 */

namespace Commerce\Adyen\Payment\Authorisation;

use Commerce\Adyen\Payment\Transaction;

/**
 * Class Response.
 *
 * @package Commerce\Adyen\Payment\Authorisation
 */
class Response extends Signature {

  // @see https://docs.adyen.com/developers/hpp-manual#hpppaymentresponse

  /**
   * An error occurred during the payment processing.
   */
  const ERROR = 'ERROR';
  /**
   * It is not possible to obtain the final status of the payment. This
   * can happen if the systems providing final status information for the
   * payment are unavailable, or if the shopper needs to take further action
   * to complete the payment.
   */
  const PENDING = 'PENDING';
  /**
   * Payment was refused. Payment authorisation was unsuccessful.
   */
  const REFUSED = 'REFUSED';
  /**
   * Payment was cancelled by the shopper before completion, or the shopper
   * returned to the merchant's site before completing the transaction.
   */
  const CANCELLED = 'CANCELLED';
  /**
   * Payment authorisation was successfully completed.
   */
  const AUTHORISED = 'AUTHORISED';

  /**
   * Payment transaction.
   *
   * @var Transaction
   */
  private $transaction;

  /**
   * Response constructor.
   *
   * @param \stdClass $order
   *   Commerce order.
   * @param array $payment_method
   *   Payment method information.
   */
  public function __construct(\stdClass $order, array $payment_method) {
    $this->data = drupal_get_query_parameters();

    if (empty($this->data)) {
      throw new \UnexpectedValueException(t('Empty response from Adyen has been received.'));
    }

    parent::__construct($order, $payment_method);

    // The "getSignature" method uses "$this->data" property for generating
    // signature. As this is response from Adyen we should not perform any
    // modifications of the values in this property until signature will not
    // be generated. In simple words: we should use raw data for calculation.
    if ($this->getSignature() !== $this->data['merchantSig']) {
      throw new \UnexpectedValueException(t('Received Adyen response with invalid signature.'));
    }

    $this->transaction = new Transaction($this->order);
    $this->transaction->setPayload($this->data);
    $this->transaction->setRemoteId($this->data['pspReference']);
    $this->transaction->setRemoteStatus($this->data['authResult']);
  }

  /**
   * Returns data received in response from Adyen.
   *
   * @return \stdClass
   *   Received data.
   */
  public function getReceivedData() {
    return (object) $this->data;
  }

  /**
   * Returns authentication result.
   *
   * @return string
   *   Value of one of constants of this class.
   */
  public function getAuthenticationResult() {
    return $this->data['authResult'];
  }

  /**
   * Returns payment transaction.
   *
   * @return Transaction
   *   Payment transaction.
   */
  public function getTransaction() {
    return $this->transaction;
  }

}
