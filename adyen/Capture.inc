<?php
/**
 * @file
 * Capture request.
 */

namespace Commerce\Adyen\Payment;

use Adyen\Service;
use Adyen\AdyenException;

/**
 * Class Capture
 *
 * @package Commerce\Adyen\Payment
 */
class Capture {

  use Client;

  /**
   * Payment transaction.
   *
   * @var Transaction
   */
  private $transaction;

  /**
   * Capture constructor.
   *
   * @param \stdClass|int|string $order
   *   Commerce order object or order ID.
   */
  public function __construct($order) {
    $this->transaction = new Transaction($order, COMMERCE_ADYEN_PAYMENT_REMOTE_STATUS_AUTHORISED);
  }

  /**
   * Checks that order has authorised Adyen transaction.
   *
   * @return bool
   *   A state of check.
   */
  public function isAvailable() {
    return $this->transaction->isAuthorised() && !empty($this->transaction->getRemoteId());
  }

  /**
   * Send a capture request.
   *
   * @throws AdyenException
   *   When capture request is not valid.
   */
  public function request() {
    $payment_method = $this->transaction->getPaymentMethod();
    $currency_code = $this->transaction->getCurrency();
    $order = $this->transaction->getOrder()->value();

    // Make an API call to tell Adyen that we are waiting for notification
    // from it.
    $capture = (new Service\Modification($this->getClient($payment_method)))->capture([
      'reference' => $order->order_number,
      'merchantAccount' => $payment_method['settings']['merchant_account'],
      'originalReference' => $this->transaction->getRemoteId(),
      'modificationAmount' => [
        'currency' => $currency_code,
        'value' => commerce_adyen_amount($this->transaction->getAmount(), $currency_code),
      ],
    ]);

    $status = '[capture-received]' === $capture['response'];
    $hook = $status ? 'received' : 'rejected';

    watchdog(COMMERCE_ADYEN_PAYMENT_METHOD, "Payment capturing request for the %order_number order has been $hook by Adyen.", [
      '%order_number' => $order->order_number,
    ]);

    module_invoke_all("commerce_adyen_capture_$hook", $this->transaction, $order);

    return $status;
  }

}
