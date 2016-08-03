# Commerce Adyen

Module provides an API for sending payments to Adyen and receiving notifications (similar to PayPal IPN).

## API

Out of the box module provides two types of Adyen response listeners:

- [payment](adyen/Response/Payment.inc)
- [notification](adyen/Response/Notification.inc)

To extend by a new one you should:

- Implement a [hook_commerce_adyen_responses()](commerce_adyen.api.php#L15)
- Describe a logic in a class which implements the [\Commerce\Adyen\ResponseInterface](adyen/ResponseInterface.inc)

In action:

- MODULE.info

```ini
name = Module Name
description = Adyen response implementation.
package = Commerce
core = 7.x
php = 5.5

autoload[adyen][] = Commerce\Adyen

dependencies[] = commerce_adyen
```

- MODULE.module

```php
/**
 * Implements hook_commerce_adyen_responses().
 */
function MODULE_commerce_adyen_responses() {
  return [
    \Commerce\Adyen\Response\Test::class,
  ];
}
```

- adyen/Response/Test.inc

```php
/**
 * @file
 * Adyen test response.
 */

namespace Commerce\Adyen\Response;

use Commerce\Adyen\Exception;
use Commerce\Adyen\ResponseInterface;

/**
 * Class Test.
 *
 * @package Commerce\Adyen\Response
 */
class Test implements ResponseInterface {

  /**
   * {@inheritdoc}
   */
  public static function type() {
    return 'test';
  }

  /**
   * {@inheritdoc}
   */
  public function __toString() {
    return 'YOUR RESULT';
  }

  /**
   * {@inheritdoc}
   */
  public function __construct(array $data) {
    // YOU LOGIC.
  }

}
```

Also, before redirecting to Adyen vendor, you able to alter the payment request:

```php
/**
 * Implements hook_commerce_adyen_payment_request_alter().
 */
function MODULE_commerce_adyen_payment_request_alter(\Commerce\Adyen\Request\Payment $payment, \stdClass $order) {
  $payment->setSessionValidity(strtotime('+ 2 hour'));
  $payment->setShopperLocale(user_load($order->uid)->language);
}
```

And, when Adyen sends a notification to us, you can react on it:

```php
/**
 * Implements hook_commerce_adyen_notification().
 */
function MODULE_commerce_adyen_notification($event_code, \stdClass $order, array $data) {
  if ('cancellation' === $event_code) {
    $transactions = commerce_payment_transaction_load_multiple([], [
      'order_id' => $order->order_id,
      'instance_id' => COMMERCE_ADYEN_PAYMENT_METHOD_INSTANCE,
      'payment_method' => COMMERCE_ADYEN_PAYMENT_METHOD,
    ]);

    $transaction = reset($transactions);
    $transaction->status = COMMERCE_PAYMENT_STATUS_FAILURE;

    commerce_payment_transaction_save($transaction);
    commerce_order_status_update($order, 'canceled');
  }
}
```

## Contributors

- Volodymyr Haidamaka (https://www.drupal.org/u/gaydamaka)
- Alexandr Danilov (https://www.drupal.org/u/alexandrdnlv)
