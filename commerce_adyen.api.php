<?php
/**
 * @file
 * Commerce Adyen API.
 */

/**
 * Register Adyen response types.
 *
 * @see commerce_adyen_responses()
 *
 * @return \Commerce\Adyen\ResponseInterface[]
 *   A list of response types.
 */
function hook_commerce_adyen_responses() {
  return [
    \Commerce\Adyen\Response\Payment::class,
    \Commerce\Adyen\Response\Notification::class,
  ];
}

/**
 * Allow alter the data used to create redirect form to Adyen.
 *
 * @param \Commerce\Adyen\Request\Payment $payment
 *   The data used to create redirect form.
 * @param \stdClass $order
 *   The full order object the redirect form is being generated for.
 *
 * @see commerce_adyen_redirect_form()
 */
function hook_commerce_adyen_payment_request_alter(\Commerce\Adyen\Request\Payment $payment, \stdClass $order) {
  $payment->setSessionValidity(strtotime('+ 2 hour'));
  $payment->setShopperLocale(user_load($order->uid)->language);
}

/**
 * React on a notification from Adyen.
 *
 * @link https://docs.adyen.com/developers/api-manual#notificationfields
 * @see \Commerce\Adyen\Response\Notification::__construct()
 *
 * @param string $event_code
 *   One of event codes in a lowercase.
 * @param array $data
 *   Submitted data.
 */
function hook_commerce_adyen_notification($event_code, array $data) {
  if ('cancellation' === $event_code) {
    $order = commerce_order_load_by_number($data['merchantReference']);
    $transactions = commerce_payment_transaction_load_multiple([], [
      'order_id' => $order->order_id,
      'instance_id' => COMMERCE_ADYEN_PAYMENT_METHOD_INSTANCE,
      'payment_method' => COMMERCE_ADYEN_PAYMENT_METHOD,
    ]);

    $transaction = reset($transactions);
    $transaction->status = COMMERCE_PAYMENT_STATUS_FAILURE;
    commerce_payment_transaction_save($transaction);
  }
}
