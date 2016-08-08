<?php
/**
 * @file
 * Commerce Adyen API.
 */

/**
 * Allow alter the data used to create payment authorization.
 *
 * @param \Commerce\Adyen\Payment\Authorization\Request $payment
 *   Payment authorization request that will be sent to Adyen.
 * @param \stdClass $order
 *   Commerce order.
 * @param array $payment_method
 *   Commerce payment method.
 *
 * @see commerce_adyen_redirect_form()
 */
function hook_commerce_adyen_payment_authorization_request_alter(\Commerce\Adyen\Payment\Authorization\Request $payment, \stdClass $order, array $payment_method) {
  $payment->setSessionValidity(strtotime('+ 2 hour'));
  $payment->setShopperLocale(user_load($order->uid)->language);
}

/**
 * Allow alter the data used to process payment authorization.
 *
 * @param \Commerce\Adyen\Payment\Authorization\Response $payment
 *   Payment authorization response that has been received from Adyen.
 * @param \stdClass $order
 *   Commerce order.
 * @param array $payment_method
 *   Commerce payment method.
 *
 * @see commerce_adyen_redirect_form_validate()
 */
function hook_commerce_adyen_payment_authorization_response_alter(\Commerce\Adyen\Payment\Authorization\Response $payment, \stdClass $order, array $payment_method) {
  switch ($payment->getAuthenticationResult()) {
    case $payment::ERROR:
      $transaction = $payment->getTransaction();
      $transaction->setStatus(COMMERCE_PAYMENT_STATUS_FAILURE);
      $transaction->setMessage('Payment cannot be completed. Adyen response: <pre>@response</pre>.', [
        '@response' => print_r($payment->getReceivedData(), TRUE),
      ]);
      break;
  }
}

/**
 * React on a notification from Adyen.
 *
 * @link https://docs.adyen.com/developers/api-manual#notificationfields
 *
 * @param string $event_code
 *   One of event codes in a lowercase.
 * @param \stdClass $order
 *   Commerce order.
 * @param \stdClass $data
 *   Received data (from $_REQEUST superglobal).
 *
 * @see commerce_adyen_notification()
 */
function hook_commerce_adyen_notification($event_code, \stdClass $order, \stdClass $data) {
  switch ($event_code) {
    case \Commerce\Adyen\Payment\Notification::CANCELLATION:
      $transaction = new \Commerce\Adyen\Payment\Transaction($order);
      $transaction->setStatus(COMMERCE_PAYMENT_STATUS_FAILURE);
      $transaction->save();

      commerce_order_status_update($order, 'canceled');
      break;
  }
}
