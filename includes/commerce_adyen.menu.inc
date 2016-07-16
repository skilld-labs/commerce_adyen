<?php
/**
 * @file
 * Adyen menu.
 */

/**
 * Menu route controller.
 *
 * @see commerce_adyen_menu()
 *
 * @param string $type
 *   Type of response from Adyen.
 *
 * @return string
 *   Processing result.
 */
function commerce_adyen_controller($type) {
  $response = '';

  try {
    $responses = commerce_adyen_responses();

    if (isset($responses[$type])) {
      $response = new $responses[$type]($_REQUEST);
    }
  }
  catch (\Commerce\Adyen\Exception $e) {
    $response = $e->getMessage();
    watchdog_exception(COMMERCE_ADYEN_PAYMENT_METHOD, $e);
  }

  return $response;
}

/**
 * Menu route delivery.
 *
 * @see drupal_deliver_html_page()
 *
 * @param string $response
 *   Response from callback.
 */
function commerce_adyen_deliver($response) {
  // Do not trigger complex logic of default delivery callback.
  print $response;
}
