<?php

/**
 * @file
 * Notifications receiver.
 */

use Commerce\Adyen\Payment\Exception\NotificationException;

/**
 * {@inheritdoc}
 *
 * @see commerce_adyen_menu()
 *
 * @internal
 */
function commerce_adyen_notification() {
  $dump = function ($variable) {
    ob_start();
    var_dump($variable);

    return ob_get_clean();
  };

  $watchdog = function ($data, \Exception $e = NULL) use ($dump) {
    $message = "Notification has been received. <pre>@data</pre>";
    $arguments = [
      '@data' => $dump($data),
    ];

    if (NULL !== $e) {
      $message = 'Notification has been handled' . ($e instanceof NotificationException ? ' wrongly' : '') . ' and exception been thrown: <pre>@exception</pre><pre>@data</pre>';
      $arguments['@exception'] = $dump($e);
    }

    watchdog(COMMERCE_ADYEN_PAYMENT_METHOD, $message, $arguments);
  };

  try {
    $data = (object) $_POST;

    // POST request should be here. No sense to let Drupal continue working.
    if (empty($_POST) || empty($data->merchantReference)) {
      throw new \RuntimeException('Adyen notification is empty.');
    }

    $order = commerce_order_load_by_number($data->merchantReference);

    // Order cannot be located/loaded. No sense to let Drupal continue working.
    if (FALSE === $order) {
      throw new \RuntimeException('Authorisation request made with incorrect "merchantReference" property which must be a valid order number.');
    }

    // Yeah, Adyen, you are nice guy! Let's send us something similar
    // in that spirit. We will process everything.
    // The "success" or "live" properties will be strings with "true"
    // or "false" values.
    foreach ($data as $property => $value) {
      if (is_string($value)) {
        // Sometimes Adyen can send visually empty strings as values
        // of properties like "success", but programmatically it's not
        // true (space characters inside).
        $data->{$property} = trim($value);

        // Convert string representations of booleans.
        switch (drupal_strtolower($data->{$property})) {
          case 'true':
            $data->{$property} = TRUE;
            break;

          case 'false':
            $data->{$property} = FALSE;
            break;
        }
      }
    }

    // Treat any kind of emptiness as explicit "FALSE".
    foreach (['success', 'live'] as $property) {
      if (empty($data->{$property})) {
        $data->{$property} = FALSE;
      }
    }

    // Log the notification (data from POST request).
    $watchdog($data);

    // Notify subscribed modules about notification.
    module_invoke_all(__FUNCTION__, $data->eventCode, $order, $data);
  }
  // Something went terribly wrong - notification should be put in a queue.
  catch (NotificationException $e) {
    $watchdog($data, $e);
    exit();
  }
  // All other exceptions does not mean that we didn't handle notification.
  catch (\Exception $e) {
    $watchdog($data, $e);
  }

  // Return "[accepted]" to Adyen. This is essential to let it know that
  // notification has been received. If Adyen do NOT receive "[accepted]"
  // then it'll try to send it again which will put all other notification
  // in a queue.
  exit('[accepted]');
}
