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
  $watchdog = function (\Exception $e = NULL) {
    $message = "Notification has been received. <pre>@data</pre>";
    $arguments = [
      '@data' => print_r($_POST, TRUE),
    ];

    if (NULL !== $e) {
      $message = 'Notification has' . ($e instanceof \RuntimeException ? ' ' : ' not ') . 'been received and exception been thrown: <pre>@exception</pre><pre>@data</pre>';
      $arguments['@exception'] = print_r($e, TRUE);
    }

    watchdog(COMMERCE_ADYEN_PAYMENT_METHOD, $message, $arguments);
  };

  try {
    // POST request should be here.
    if (empty($_POST)) {
      throw new \RuntimeException('Adyen notification is empty.');
    }

    $data = (object) $_POST;
    $order = commerce_order_load_by_number($data->merchantReference);

    if (FALSE === $order) {
      throw new \RuntimeException('Authorisation request made with incorrect "merchantReference" property which must be a valid order number.');
    }

    $watchdog();
    module_invoke_all(__FUNCTION__, $data->eventCode, $order, $data);
  }
  // All runtime exceptions does not mean that we didn't receive notification.
  catch (\RuntimeException $e) {
    $watchdog($e);
  }
  // Something went wrong - notification should be put in a queue.
  catch (\Exception $e) {
    $watchdog($e);
    exit();
  }

  // Return "[accepted]" to Adyen. This is essential to let it know that
  // notification has been received. If Adyen do NOT receive "[accepted]"
  // then it'll try to send it again which will put all other notification
  // in a queue.
  exit('[accepted]');
}
