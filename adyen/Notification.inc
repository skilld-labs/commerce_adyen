<?php
/**
 * @file
 * Notification Interface.
 */

namespace Commerce\Adyen\Payment;

/**
 * Class Notification.
 *
 * @package Commerce\Adyen\Payment
 */
abstract class Notification {

  // @see https://github.com/Adyen/php/blob/master/3.Notifications/httppost/notification-server.php

  const AUTHORISATION = 'AUTHORISATION';
  const CANCELLATION = 'CANCELLATION';
  const REFUND = 'REFUND';
  const CANCEL_OR_REFUND = 'CANCEL_OR_REFUND';
  const CAPTURE = 'CAPTURE';
  const REFUNDED_REVERSED = 'REFUNDED_REVERSED';
  const CAPTURE_FAILED = 'CAPTURE_FAILED';
  const REQUEST_FOR_INFORMATION = 'REQUEST_FOR_INFORMATION';
  const NOTIFICATION_OF_CHARGE_BACK = 'NOTIFICATION_OF_CHARGEBACK';
  const CHARGE_BACK = 'CHARGEBACK';
  const CHARGE_BACK_REVERSED = 'CHARGEBACK_REVERSED';
  const REPORT_AVAILABLE = 'REPORT_AVAILABLE';

}
