<?php
/**
 * @file
 * Adyen notification response.
 */

namespace Commerce\Adyen\Response;

use Commerce\Adyen\Exception;
use Commerce\Adyen\ResponseInterface;

/**
 * Class Notification.
 *
 * @package Commerce\Adyen\Response
 */
class Notification implements ResponseInterface {

  /**
   * {@inheritdoc}
   */
  public static function type() {
    return 'notification';
  }

  /**
   * {@inheritdoc}
   */
  public function __toString() {
    // Return [accepted] to us. This is essential to let us
    // know that you received the notification. If we do NOT receive
    // [accepted] we try to send the notification again which
    // will put all other notification in a queue.
    return '[accepted]';
  }

  /**
   * {@inheritdoc}
   */
  public function __construct(array $data) {
    if (empty($data)) {
      throw new Exception(t('Adyen notification is empty.'));
    }

    $data['eventCode'] = strtolower($data['eventCode']);
    $order = commerce_order_load_by_number($data['merchantReference']);

    if (empty($order->order_id)) {
      throw new Exception(t('Incorrect merchant reference has been sent by Adyen in notification.'));
    }

    module_invoke_all('commerce_adyen_notification', $data['eventCode'], $order, $data);
  }

}
