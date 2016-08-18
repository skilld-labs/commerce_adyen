<?php
/**
 * @file
 * Commerce Adyen API.
 */

/**
 * Allow alter the data used to create payment authorisation.
 *
 * @param \Commerce\Adyen\Payment\Authorisation\Request $payment
 *   Payment authorisation request that will be sent to Adyen.
 * @param \stdClass $order
 *   Commerce order.
 * @param array $payment_method
 *   Commerce payment method.
 *
 * @see commerce_adyen_redirect_form()
 */
function hook_commerce_adyen_payment_authorisation_request_alter(\Commerce\Adyen\Payment\Authorisation\Request $payment, \stdClass $order, array $payment_method) {
  $payment->setSessionValidity(strtotime('+ 2 hour'));
  $payment->setShopperLocale(user_load($order->uid)->language);
}

/**
 * Allow alter the data used to process payment authorisation.
 *
 * @param \Commerce\Adyen\Payment\Authorisation\Response $payment
 *   Payment authorisation response that has been received from Adyen.
 * @param \stdClass $order
 *   Commerce order.
 * @param array $payment_method
 *   Commerce payment method.
 *
 * @see commerce_adyen_redirect_form_validate()
 */
function hook_commerce_adyen_payment_authorisation_response_alter(\Commerce\Adyen\Payment\Authorisation\Response $payment, \stdClass $order, array $payment_method) {
  switch ($payment->getAuthenticationResult()) {
    case $payment::ERROR:
      $transaction = $payment->getTransaction();
      $transaction->setStatus(COMMERCE_PAYMENT_STATUS_FAILURE);
      $transaction->setMessage('Payment cannot be completed. Adyen response: <pre>@response</pre>.', [
        '@response' => var_export($payment->getReceivedData(), TRUE),
      ]);
      break;
  }
}

/**
 * React on receiving capture request by Adyen.
 *
 * WARNING! You must not set transaction status to success here because
 * "receiving" just means that Adyen server receive your query. It could
 * be wrongly created - anyway Adyen will respond that capture received.
 *
 * WRONG USE CASE: you have multiple merchant accounts in your account in
 * Adyen. You've made a payment with one of them and capture request - with
 * another one. In this case you will think that payment has been captured,
 * but it's not true.
 *
 * Correct use case: use notifications to finalize the payment transaction.
 *
 * @param \Commerce\Adyen\Payment\Transaction $transaction
 *   Payment transaction.
 * @param \stdClass $order
 *   Commerce order.
 *
 * @see \Commerce\Adyen\Payment\Capture::request()
 */
function hook_commerce_adyen_capture_received(\Commerce\Adyen\Payment\Transaction $transaction, \stdClass $order) {
  /* @var \EntityDrupalWrapper $message */
  $message = entity_metadata_wrapper('message', message_create('commerce_adyen', [
    'arguments' => [
      '@message' => t('Capture request for %order_number order has been received.', [
        '%order_number' => $order->order_number,
      ]),
    ],
  ]));

  $message->message_commerce_order = $order->order_id;
  $message->save();
}

/**
 * React on rejecting capture request by Adyen.
 *
 * @param \Commerce\Adyen\Payment\Transaction $transaction
 *   Payment transaction.
 * @param \stdClass $order
 *   Commerce order.
 *
 * @see \Commerce\Adyen\Payment\Capture::request()
 */
function hook_commerce_adyen_capture_rejected(\Commerce\Adyen\Payment\Transaction $transaction, \stdClass $order) {

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

/**
 * Provide additional data for Adyen by implementing payment types.
 *
 * @return array[]
 *   An associative array where key - is an unique name of payment type
 *   and value - is an associative array with two mandatory keys: "label"
 *   and "class".
 *
 * @see commerce_adyen_payment_types()
 */
function hook_commerce_adyen_payment_types() {
  $types = [];

  $types['openinvoice'] = [
    'label' => 'OpenInvoice',
    'controllers' => [
      'payment' => \Commerce\Adyen\Payment\OpenInvoice\PaymentController::class,
      'checkout' => \Commerce\Adyen\Payment\OpenInvoice\CheckoutController::class,
    ],
  ];

  return $types;
}

/**
 * Allow to alter existing definitions of payment types.
 *
 * @param array[] $payment_types
 *   An associative array with defined payment types.
 *
 * @see hook_commerce_adyen_payment_types()
 */
function hook_commerce_adyen_payment_types_alter(array &$payment_types) {
  unset($payment_types['openinvoice']);
}

/**
 * React on initialization of checkout fields.
 *
 * @param array[] $instances
 *   Drupal field definitions.
 * @param \EntityDrupalWrapper $order
 *   Wrapper for the "commerce_order" entity.
 * @param \Commerce\Adyen\Payment\Controller\Checkout $controller
 *   An instance of checkout controller.
 * @param string $type
 *   Machine name of payment type.
 *
 * @see commerce_adyen_submit_form()
 */
function hook_commerce_adyen_checkout_fields_init(array $instances, \EntityDrupalWrapper $order, \Commerce\Adyen\Payment\Controller\Checkout $controller, $type) {

}

/**
 * React on saving the values of checkout fields.
 *
 * This hook will be triggered only if validation of the controller
 * will be succeed.
 *
 * @param array $values
 *   Values of checkout form.
 * @param \EntityDrupalWrapper $order
 *   Wrapper for the "commerce_order" entity.
 * @param \Commerce\Adyen\Payment\Controller\Checkout $controller
 *   An instance of checkout controller.
 * @param string $type
 *   Machine name of payment type.
 *
 * @see commerce_adyen_submit_form_submit()
 */
function hook_commerce_adyen_checkout_fields_save(array $values, \EntityDrupalWrapper $order, \Commerce\Adyen\Payment\Controller\Checkout $controller, $type) {

}
