<?php
/**
 * @file
 * Rules hooks.
 */

/**
 * Implements hook_rules_action_info().
 */
function commerce_adyen_pro_rules_action_info() {
  $actions = [];

  $actions['adyen_capture_request'] = [
    'label' => t('Capture request'),
    'group' => 'Commerce Order',
    'parameter' => [
      'order' => [
        'type' => 'commerce_order',
        'label' => t('Commerce order'),
      ],
    ],
    'callbacks' => [
      'execute' => '_commerce_adyen_pro_capture_request',
    ],
  ];

  return $actions;
}

/**
 * Rules action: Capture request.
 *
 * @see commerce_adyen_pro_rules_action_info()
 *
 * @param \stdClass $order
 *   Commerce order.
 */
function _commerce_adyen_pro_capture_request(\stdClass $order) {
  if (!empty($order)) {
    $payment_transaction = commerce_payment_transaction_load_multiple([], ['order_id' =>  $order->order_id]);

    if (!empty($payment_transaction)) {
      $payment_transaction = array_shift($payment_transaction);

      if (
        'commerce_adyen_pro' === $payment_transaction->payment_method &&
        'authorised' === $payment_transaction->remote_status
      ) {
        $payment_method = commerce_payment_method_instance_load($payment_transaction->instance_id);
          // Required parameter.
          //
          // @link https://docs.adyen.com/developers/api-manual#capture
          $request = [
            // In this case, it's the capture payment: Payment.capture.
            'action' => 'Payment.capture',
            // The merchant account the payment was processed with.
            'modificationRequest.merchantAccount' => $payment_method['settings']['merchant_account'],
            // The amount to capture.
            'modificationRequest.modificationAmount.currency' => $payment_transaction->currency_code,
            'modificationRequest.modificationAmount.value' => $payment_transaction->amount,
            // This is the pspReference that was assigned to the authorisation.
            'modificationRequest.originalReference' => $payment_transaction->remote_id,
          ];

          $ch = curl_init();

          curl_setopt($ch, CURLOPT_URL, _commerce_adyen_pro_server_url($payment_method['settings']['is_test'], 'capture'));
          curl_setopt($ch, CURLOPT_HEADER, FALSE);
          curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
          curl_setopt($ch, CURLOPT_USERPWD, $payment_method['settings']['client_user'] . ':' . $payment_method['settings']['client_password']);
          curl_setopt($ch, CURLOPT_POST, count($request));
          curl_setopt($ch, CURLOPT_POSTFIELDS, http_build_query($request));
          curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);

          $result = curl_exec($ch);

          if (empty($result)) {
            $message = curl_error($ch);

            watchdog(COMMERCE_ADYEN_PRO_PAYMENT_METHOD_NAME, $message, [], WATCHDOG_ERROR);

            $payment_transaction->status = COMMERCE_PAYMENT_STATUS_FAILURE;
            $payment_transaction->message = $message;

            commerce_payment_transaction_save($payment_transaction);

            commerce_order_status_update($order, LUSH_ORDER_STUCK_STATUS);
          }
          else {
            parse_str($result, $result);

            if ('[capture-received]' === $result['modificationResult_response']) {
              watchdog(COMMERCE_ADYEN_PRO_PAYMENT_METHOD_NAME, t('Payment transaction is capture'), [], WATCHDOG_INFO);
            }
          }

          curl_close($ch);
      }
    }
  }
}
