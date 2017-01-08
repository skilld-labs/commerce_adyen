<?php

/**
 * @file
 * Commerce integration.
 */

use Commerce\Adyen\Payment\Transaction;
use Commerce\Adyen\Payment\Notification;

/**
 * Implements hook_commerce_payment_method_info().
 */
function commerce_adyen_commerce_payment_method_info() {
  $info = [];

  $info[COMMERCE_ADYEN_PAYMENT_METHOD] = [
    'file' => 'includes/commerce_adyen.payment.inc',
    'title' => 'Adyen',
    'description' => t('Redirect users to submit payments through Adyen.'),
    'active' => TRUE,
    'terminal' => FALSE,
    'offsite' => TRUE,
    'offsite_autoredirect' => TRUE,
  ];

  return $info;
}

/**
 * Implements hook_commerce_payment_transaction_status_info().
 */
function commerce_adyen_commerce_payment_transaction_status_info() {
  $module_path = drupal_get_path('module', 'commerce_adyen');
  $info = [];

  foreach ([
    // Adyen have its own payment workflow: initially payment should get
    // the authorisation confirmation and only then capturing could be
    // done. This status must be set when Adyen responds to us.
    COMMERCE_ADYEN_PAYMENT_STATUS_AUTHORISED => [
      'title' => t('Authorised'),
      'total' => TRUE,
    ],
  ] as $status => $data) {
    $info[$status] = $data + [
      'status' => $status,
      'icon' => "$module_path/icons/$status.png",
    ];
  }

  return $info;
}

/**
 * Implements hook_commerce_payment_transaction_insert().
 *
 * @see commerce_payment_commerce_payment_transaction_insert()
 */
function commerce_adyen_commerce_payment_transaction_insert(\stdClass $transaction) {
  $adyen_transaction = new Transaction($transaction->order_id);

  // Check the transaction is authorised.
  if ($adyen_transaction->isAuthorised()) {
    $order = $adyen_transaction->getOrder()->value();

    // Then check to make sure the event hasn't been invoked for this order.
    if (empty($order->data['commerce_payment_order_paid_in_full_invoked'])) {
      // Check the order balance and invoke the event.
      $balance = commerce_payment_order_balance($order);

      if (!empty($balance) && $balance['amount'] <= 0) {
        // Invoke the event including a hook of the same name.
        rules_invoke_all('commerce_payment_order_paid_in_full', $order, $transaction);
        // Update the order's data array to indicate this just happened.
        $order->data['commerce_payment_order_paid_in_full_invoked'] = TRUE;
        // Save the updated order.
        commerce_order_save($order);
      }
    }
  }
}

/**
 * Implements hook_commerce_payment_transaction_update().
 */
function commerce_adyen_commerce_payment_transaction_update(\stdClass $transaction) {
  commerce_adyen_commerce_payment_transaction_insert($transaction);
}

/**
 * Implements hook_commerce_order_insert().
 *
 * @todo We need to deeply debug the "commerce_payment_order_paid_in_full" rule
 * and, possibly, get rid from this hook and "commerce_adyen_order_paid_in_full"
 * event.
 */
function commerce_adyen_commerce_order_insert(\stdClass $order) {
  $paid_but_not_with_adyen = !empty($order->data['commerce_payment_order_paid_in_full_invoked']) && empty($order->data['commerce_adyen_order_paid_in_full_invoked']);

  // Check that the order has been paid in full, but not mark as paid by this
  // module yet.
  if ($paid_but_not_with_adyen) {
    // Add our own paid in full flag. Note we're doing this before invoking the
    // rule, this is to allow other modules/rules to rely on this flag during
    // the rule invocation.
    $order->data['commerce_adyen_order_paid_in_full_invoked'] = TRUE;
    commerce_order_save($order);

    rules_invoke_event('commerce_adyen_order_paid_in_full', $order);
  }
}

/**
 * Implements hook_commerce_order_update().
 */
function commerce_adyen_commerce_order_update(\stdClass $order) {
  commerce_adyen_commerce_order_insert($order);
}

/**
 * Implements hook_commerce_adyen_notification().
 */
function commerce_adyen_commerce_adyen_notification($event_code, \stdClass $order, \stdClass $data) {
  switch ($event_code) {
    case Notification::AUTHORISATION:
    case Notification::PENDING:
      // Load/create transaction object.
      $transaction = new Transaction($order);

      if ($data->success) {
        if (Notification::AUTHORISATION === $event_code) {
          if (!$transaction->isAuthorised()) {
            $transaction->authorise($data->pspReference);

            // Send capture request automatically.
            commerce_adyen_capture_request($order);
            commerce_payment_redirect_pane_next_page($order);
          }
        }
        // This state is available in case of "Boleto"
        // payments. Capturing is not needed.
        elseif (Notification::PENDING === $event_code) {
          if (!$transaction->isPending()) {
            $transaction->pending($data->pspReference);

            commerce_order_status_update($order, 'pending');
            commerce_payment_redirect_pane_next_page($order);
          }
        }
      }
      else {
        // Allow to fail only new transaction because authorised or
        // pending transaction were processed.
        if ($transaction->isNew()) {
          // Something went wrong: order isn't paid.
          $transaction->fail($data->pspReference);
          commerce_payment_redirect_pane_previous_page($order);
        }
      }
      break;

    case Notification::CAPTURE:
      $transaction = new Transaction($order);

      if ($data->success && !$transaction->isFinalized()) {
        $transaction->finalize();
      }
      break;

    case Notification::CANCELLATION:
      $transaction = new Transaction($order);
      $transaction->fail($data->pspReference);

      commerce_order_status_update($order, 'canceled');
      break;
  }

  if (isset($transaction)) {
    $transaction->setPayload($data);
    $transaction->save();
  }
}
