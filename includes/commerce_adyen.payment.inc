<?php
/**
 * @file
 * Adyen payment.
 */

/**
 * Implements PAYMENT_METHOD_BASE_settings_form().
 *
 * {@inheritdoc}
 *
 * @see commerce_adyen_commerce_payment_method_info()
 * @see \RulesDataUIPaymentSettings::inputForm()
 *
 * @internal
 */
function commerce_adyen_settings_form(array $settings) {
  $inputs = [];
  $form = [];

  $link = function ($title, $link) use ($settings) {
    // @codingStandardsIgnoreStart
    return ' ' . t('You will find the right value in the <a href="@href" target="_blank">' . $title . '</a>.', [
      // @codingStandardsIgnoreEnd
      '@href' => sprintf("https://ca-%s.adyen.com/ca/ca/$link", isset($settings['mode']) ? $settings['mode'] : \Adyen\Environment::TEST),
    ]);
  };

  $form['url'] = [
    '#markup' => t('Please <a href="@notification_setup" target="_blank">configure your Adyen account</a> to send notifications to this URL: %notification_url.', [
      '@notification_setup' => 'https://docs.adyen.com/developers/api-manual#setupnotifications',
      '%notification_url' => url(COMMERCE_ADYEN_NOTIFICATION_URL, ['absolute' => TRUE]),
    ]),
  ];

  $inputs['mode'] = [
    '#type' => 'radios',
    '#title' => t('Mode'),
    '#default_value' => \Adyen\Environment::TEST,
    '#options' => [
      \Adyen\Environment::TEST => t('Test'),
      \Adyen\Environment::LIVE => t('Live'),
    ],
  ];

  $inputs['merchant_account'] = [
    '#title' => t('Merchant Account'),
    '#description' => t('Do not confuse this with your Adyen account name.') . $link('account list', 'accounts/show.shtml?accountTypeCode=MerchantAccount'),
  ];

  $inputs['client_user'] = [
    '#title' => t('Client user'),
    '#description' => t('Username for a web service.') . $link('user list', 'config/users.shtml?userType=SU&status=Active'),
  ];

  $inputs['client_password'] = [
    '#title' => t('Client password'),
    '#description' => t('Password for a web service user.'),
  ];

  $inputs['skin_code'] = [
    '#title' => t('Skin Code'),
    '#description' => t('One of HPP skin codes.') . $link('skin list', 'skin/skins.shtml'),
  ];

  $inputs['hmac'] = [
    '#title' => t('HMAC key'),
    '#description' => t('Make sure you have the same values here and in Adyen.'),
  ];

  $inputs['shopper_locale'] = [
    '#type' => 'select',
    '#title' => t('Shopper locale'),
    '#description' => t('A combination of language code and country code to specify the language used in the session.'),
    '#default_value' => 'en_GB',
    // @link https://docs.adyen.com/developers/hpp-manual#createaskin
    '#options' => array_map('t', [
      'zh' => 'Chinese – Traditional',
      'cz' => 'Czech',
      'da' => 'Danish',
      'nl' => 'Dutch',
      'en_GB' => 'English – British',
      'en_CA' => 'English – Canadian',
      'en_US' => 'English – US',
      'fi' => 'Finnish',
      'fr' => 'French',
      'fr_BE' => 'French – Belgian',
      'fr_CA' => 'French – Canadian',
      'fr_CH' => 'French – Swiss',
      'fy_NL' => 'Frisian',
      'de' => 'German',
      'el' => 'Greek',
      'hu' => 'Hungarian',
      'it' => 'Italian',
      'li' => 'Lithuanian',
      'no' => 'Norwegian',
      'pl' => 'Polish',
      'pt' => 'Portuguese',
      'ru' => 'Russian',
      'sk' => 'Slovak',
      'es' => 'Spanish',
      'sv' => 'Swedish',
      'th' => 'Thai',
      'tr' => 'Turkish',
      'uk' => 'Ukrainian',
    ]),
  ];

  foreach ($inputs as $name => $definition) {
    $inputs[$name] += [
      '#type' => 'textfield',
      '#required' => TRUE,
    ];

    if (isset($settings[$name])) {
      $inputs[$name]['#default_value'] = $settings[$name];
    }
  }

  return $form + $inputs;
}

/**
 * Implements PAYMENT_METHOD_BASE_redirect_form().
 *
 * {@inheritdoc}
 *
 * @see commerce_adyen_commerce_payment_method_info()
 * @see commerce_payment_redirect_pane_checkout_form()
 *
 * @internal
 */
function commerce_adyen_redirect_form(array $form, array &$form_state, \stdClass $order, array $payment_method) {
  $redirect_available = TRUE;

  try {
    $payment = new \Commerce\Adyen\Payment\Authorization\Request($order, $payment_method);
    // @see hook_commerce_adyen_payment_authorization_request_alter()
    drupal_alter('commerce_adyen_payment_authorization_request', $payment, $order, $payment_method);
    // Sign the request to verify it later.
    $payment->signRequest();

    foreach ($payment as $name => $value) {
      $form[$name] = [
        '#type' => 'hidden',
        '#value' => $value,
      ];
    }

    $form['#action'] = $payment->getEndpoint();
    $form['#payment'] = $payment;
    $form['#pre_render'][] = 'commerce_adyen_redirect_form_pre_render';
  }
  catch (\Exception $e) {
    watchdog_exception(COMMERCE_ADYEN_PAYMENT_METHOD, $e);
    drupal_set_message($e->getMessage(), 'error');
    $redirect_available = FALSE;
  }

  $form['submit'] = [
    '#type' => 'submit',
    '#value' => t('Proceed to Adyen'),
    '#disabled' => !$redirect_available,
  ];

  return $form;
}

/**
 * Implements PAYMENT_METHOD_BASE_redirect_form_validate().
 *
 * {@inheritdoc}
 *
 * @see commerce_adyen_commerce_payment_method_info()
 * @see commerce_payment_redirect_pane_checkout_form()
 *
 * @internal
 */
function commerce_adyen_redirect_form_validate(\stdClass $order, array $payment_method) {
  try {
    // GET request should be here.
    $payment = new \Commerce\Adyen\Payment\Authorization\Response($order, $payment_method);
    $transaction = $payment->getTransaction();

    switch ($payment->getAuthenticationResult()) {
      case $payment::AUTHORIZED:
        $transaction->setStatus(COMMERCE_ADYEN_PAYMENT_STATUS_AUTHORISED);
        $transaction->setMessage('Payment has been successfully authorized.');
        break;

      case $payment::PENDING:
        $transaction->setStatus(COMMERCE_ADYEN_PAYMENT_STATUS_AUTHORISED);
        $transaction->setMessage('Payment is pending because final status information is unavailable or shopper needs to take further actions to complete it.');
        break;

      case $payment::ERROR:
      case $payment::REFUSED:
        $transaction->setStatus(COMMERCE_PAYMENT_STATUS_FAILURE);
        drupal_set_message(t('Payment authorisation was not successful. Please try again.'), 'error');
        commerce_payment_redirect_pane_previous_page($order);
        break;

      case $payment::CANCELLED:
        $transaction->setStatus(COMMERCE_PAYMENT_STATUS_FAILURE);
        drupal_set_message(t('Payment has been cancelled.'), 'status');
        commerce_payment_redirect_pane_previous_page($order);
        break;

      default:
        throw new \UnexpectedValueException(t('Received Adyen response with invalid "@property" property.', [
          '@property' => 'authResult',
        ]));
    }

    // @see hook_commerce_adyen_payment_authorization_response_alter()
    drupal_alter('commerce_adyen_payment_authorization_response', $payment, $order, $payment_method);

    $transaction->save();
  }
  catch (\Exception $e) {
    watchdog_exception(COMMERCE_ADYEN_PAYMENT_METHOD, $e);
    drupal_set_message($e->getMessage(), 'error');
    // We should return boolean indicator since this is unusual form validation.
    return FALSE;
  }

  return TRUE;
}

/**
 * Pre-render callback.
 *
 * {@inheritdoc}
 *
 * @see commerce_adyen_redirect_form()
 * @see drupal_render()
 *
 * @internal
 */
function commerce_adyen_redirect_form_pre_render(array $form) {
  // Remove Drupal fields from a form (such as "form_token", "form_id" etc).
  // This needs to be done since Adyen will generate a signature from submitted
  // data and will expect only those fields which described in API. Any other
  // data will lead to wrong signature of payment request.
  foreach (array_diff(element_children($form), array_keys(iterator_to_array($form['#payment']))) as $name) {
    unset($form[$name]);
  }

  return $form;
}
