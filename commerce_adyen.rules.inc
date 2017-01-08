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

  // @see commerce_adyen_capture_request()
  $actions['commerce_adyen_capture_request'] = [
    'label' => t('Adyen capture request'),
    'group' => 'Commerce Order',
    'parameter' => [
      'order' => [
        'type' => 'commerce_order',
        'label' => t('Commerce order'),
      ],
    ],
  ];

  return $actions;
}

/**
 * Implements hook_rules_event_info().
 */
function commerce_adyen_rules_event_info() {
  $events = [];

  $events['commerce_adyen_order_paid_in_full'] = [
    'label' => t('Adyen transaction is authorised'),
    'group' => t('Commerce Adyen'),
    'variables' => entity_rules_events_variables('commerce_order', t('Order'), TRUE),
    'access callback' => 'commerce_order_rules_access',
  ];

  return $events;
}
