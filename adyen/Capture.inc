<?php
/**
 * @file
 * Capture request.
 */

namespace Commerce\Adyen\Payment;

use Adyen\Client;
use Adyen\Service;
use Adyen\AdyenException;

/**
 * Class Capture
 *
 * @package Commerce\Adyen\Payment
 */
class Capture {

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
    $order = $this->transaction->getOrder()->value();

    $client = new Client();
    $client->setUsername($payment_method['settings']['client_user']);
    $client->setPassword($payment_method['settings']['client_password']);
    $client->setEnvironment($payment_method['settings']['mode']);

    // Make an API call to tell Adyen that we are waiting for notification
    // from it.
    $capture = (new Service\Modification($client))->capture([
      'reference' => $order->order_number,
      'merchantAccount' => $payment_method['settings']['merchant_account'],
      'originalReference' => $this->transaction->getRemoteId(),
      'modificationAmount' => [
        'currency' => $this->transaction->getCurrency(),
        'value' => $this->transaction->getAmount(),
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
