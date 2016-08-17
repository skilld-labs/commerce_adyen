<?php
/**
 * @file
 * Commerce integration.
 */

use Commerce\Adyen\Payment;

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
  if ((new Payment\Transaction($transaction->order_id))->isAuthorised()) {
    // An authorised payment with Adyen needs to be treated as part of the order
    // total. So we need to check the balance and fire the paid in full event.
    // Invoke the commerce_payment hook, with a spoofed success status,
    // instead of duplicating code.
    $transaction->status = COMMERCE_PAYMENT_STATUS_SUCCESS;

    module_invoke('commerce_payment', 'commerce_payment_transaction_insert', $transaction);

    // Restore our custom transaction status.
    $transaction->status = COMMERCE_ADYEN_PAYMENT_STATUS_AUTHORISED;
  }
}

/**
 * Implements hook_commerce_payment_transaction_update().
 */
function commerce_adyen_commerce_payment_transaction_update(\stdClass $transaction) {
  commerce_adyen_commerce_payment_transaction_insert($transaction);
}

/**
 * Implements hook_commerce_order_presave().
 */
function commerce_adyen_commerce_order_presave(\stdClass $order) {
  if (
    // Check that "commerce_payment_order_paid_in_full" event has been already
    // invoked.
    !empty($order->data['commerce_payment_order_paid_in_full_invoked']) &&
    empty($order->data['commerce_adyen_order_paid_in_full_invoked'])
  ) {
    rules_invoke_event('commerce_adyen_order_paid_in_full', $order);
    $order->data['commerce_adyen_order_paid_in_full_invoked'] = TRUE;
  }
}

/**
 * Implements hook_commerce_adyen_notification().
 */
function commerce_adyen_commerce_adyen_notification($event_code, \stdClass $order, \stdClass $data) {
  switch ($event_code) {
    case Payment\Notification::AUTHORISATION:
      $transaction = new Payment\Transaction($order);

      if ($data->success) {
        if (!$transaction->isAuthorised()) {
          $transaction->authorise($data->pspReference);
          commerce_payment_redirect_pane_next_page($order);
        }
      }
      else {
        $transaction->fail($data->pspReference);
        commerce_payment_redirect_pane_previous_page($order);
      }
      break;

    case Payment\Notification::CAPTURE:
      $transaction = new Payment\Transaction($order);

      if ($data->success && !$transaction->isFinalized()) {
        $transaction->finalize();
      }
      break;

    case Payment\Notification::CAPTURE_FAILED:
      // @todo What should be implemented here?
      break;

    case Payment\Notification::CANCELLATION:
      $transaction = new Payment\Transaction($order);
      $transaction->fail($data->pspReference);

      commerce_order_status_update($order, 'canceled');
      break;
  }

  if (isset($transaction)) {
    $transaction->setPayload($data);
    $transaction->save();
  }
}