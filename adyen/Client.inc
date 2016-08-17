<?php
/**
 * @file
 * Adyen client.
 */

namespace Commerce\Adyen\Payment;

/**
 * Trait Client.
 *
 * @package Commerce\Adyen\Payment
 */
trait Client {

  /**
   * Get client.
   *
   * @param array $payment_method
   *   Payment method instance.
   *
   * @return \Adyen\Client
   *   Client object.
   */
  public function getClient(array $payment_method) {
    $client = new \Adyen\Client();
    $client->setUsername($payment_method['settings']['client_user']);
    $client->setPassword($payment_method['settings']['client_password']);
    $client->setEnvironment($payment_method['settings']['mode']);

    return $client;
  }

}
