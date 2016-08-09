<?php
/**
 * @file
 * Rules integration.
 */

/**
 * Implements hook_rules_action_info().
 */
function commerce_adyen_rules_action_info() {
  $actions = [];

  $actions['commerce_adyen_capture_request'] = [
    'label' => t('Adyen capture request'),
    'group' => 'Commerce Order',
    'parameter' => [
      'order' => [
        'type' => 'commerce_order',
        'label' => t('Commerce order'),
      ],
    ],
    'callbacks' => [
      'execute' => 'commerce_adyen_capture_request',
    ],
  ];

  return $actions;
}

/**
 * Capture request. Used as callback for action of a "rules".
 *
 * @see commerce_adyen_rules_action_info()
 *
 * @param \stdClass $order
 *   Commerce order.
 */
function commerce_adyen_capture_request(\stdClass $order) {
  try {
    $capture = new \Commerce\Adyen\Payment\Capture($order);

    if ($capture->isAvailable()) {
      $capture->request();
    }
    else {
      throw new \RuntimeException(t('Capture request cannot be performed for the %order_number order because it have no authorised Adyen transactions.', [
        '%order_number' => $order->order_number,
      ]));
    }
  }
  catch (\Exception $exception) {
    watchdog_exception(COMMERCE_ADYEN_PAYMENT_METHOD, $exception);
  }
}
