<?php
/**
 * @file
 * Routes controllers for Adyen.
 */

/**
 * {@inheritdoc}
 *
 * @see commerce_adyen_menu()
 *
 * @internal
 */
function commerce_adyen_notification() {
  try {
    // POST request should be here.
    if (empty($_REQUEST)) {
      throw new \UnexpectedValueException('Adyen notification is empty.');
    }

    $data = (object) $_REQUEST;
    $data->eventCode = strtolower($data->eventCode);
    $order = commerce_order_load_by_number($data->merchantReference);

    if (empty($order->order_id)) {
      throw new \UnexpectedValueException('Authorization request was made with incorrect "merchantReference" property which must be a valid order number.');
    }

    module_invoke_all(__FUNCTION__, $data->eventCode, $order, $data);

    // Return "[accepted]" to Adyen. This is essential to let it know that
    // notification has been received . If Adyen do NOT receive "[accepted]"
    // then it'll try to send it again which will put all other notification
    // in a queue.
    return '[accepted]';
  }
  catch (\Exception $e) {
    watchdog_exception(COMMERCE_ADYEN_PAYMENT_METHOD, $e);

    return $e->getMessage();
  }
}
