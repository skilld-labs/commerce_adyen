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
 * Implements hook_rules_event_info().
 */
function commerce_adyen_rules_event_info() {
  // So that we can use the entity_rules_events_variables() helper function.
  module_load_include('inc', 'entity', 'entity.rules');

  $variables = array_merge(
    entity_rules_events_variables('commerce_order', t('Order', array(), array('context' => 'a drupal commerce order')), TRUE, TRUE),
    entity_rules_events_variables('commerce_payment_transaction', t('Last completed transaction'), TRUE)
  );

  $events['commerce_adyen_order_paid_in_full'] = array(
    'label' => t('Adyen transaction is authorised'),
    'group' => t('Commerce Adyen'),
    'variables' => $variables,
    'access callback' => 'commerce_order_rules_access',
  );

  return $events;
}

/**
 * Capture request.
 *
 * @see commerce_adyen_rules_action_info()
 *
 * @param \stdClass $order
 *   Commerce order.
 */
function commerce_adyen_capture_request(\stdClass $order) {
  try {
    $capture = new \Commerce\Adyen\Payment\Capture($order);
    $options = ['%order_number' => $order->order_number];

    if (empty($order->data['adyen_capture_sent'])) {
      if ($capture->isAvailable()) {
        if ($capture->request()) {
          // Mark that we don't need to resend the capture
          // request once we successfully did this.
          $order->data['adyen_capture_sent'] = TRUE;
          // Save the order.
          commerce_order_save($order);
          // Inform about capturing.
          watchdog(COMMERCE_ADYEN_PAYMENT_METHOD, 'Capture request for the %order_number order was received by Adyen.', $options);
        }
        else {
          throw new \RuntimeException(t('Capture request for the %order_number order was rejected by Adyen.', $options));
        }
      }
      else {
        throw new \RuntimeException(t('Capture request cannot be performed for the %order_number order because it have no authorised Adyen transactions.', $options));
      }
    }
  }
  catch (\Exception $e) {
    watchdog_exception(COMMERCE_ADYEN_PAYMENT_METHOD, $e);
  }
}
