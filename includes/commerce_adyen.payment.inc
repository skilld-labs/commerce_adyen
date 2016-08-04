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
  $form = [];

  $form['url'] = [
    '#markup' => t('Use this URL for response from Adyen: @url', [
      '@url' => commerce_adyen_payment_response_url(),
    ]),
  ];

  $form['mode'] = [
    '#type' => 'radios',
    '#title' => t('Mode'),
    '#default_value' => 'test',
    '#options' => [
      \Adyen\Environment::TEST => t('Test'),
      \Adyen\Environment::LIVE => t('Live'),
    ],
  ];

  $form['merchant_account'] = [
    '#type' => 'textfield',
    '#title' => t('Merchant Account'),
    '#description' => t('Do not confuse this with your Adyen account name. You find the right value in the backend in the accounts list.'),
    '#default_value' => '',
  ];

  $form['client_user'] = [
    '#type' => 'textfield',
    '#title' => t('Client user'),
    '#description' => t('User name for web service. You find the right value in "Settings -> Users" in Adyen UI.'),
  ];

  $form['client_password'] = [
    '#type' => 'textfield',
    '#title' => t('Client password'),
  ];

  $form['skin_code'] = [
    '#type' => 'textfield',
    '#title' => t('Skin Code'),
  ];

  $form['hmac'] = [
    '#type' => 'textfield',
    '#title' => t('HMAC'),
    '#description' => t('Please make sure you have the same HMAC value set here and in the Adyen backend.'),
  ];

  $form['shopper_locale'] = [
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

  foreach ($form as $name => $definition) {
    $form[$name] += ['#required' => TRUE];

    if (isset($settings[$name])) {
      $form[$name]['#default_value'] = $settings[$name];
    }
  }

  return $form;
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
  if (empty($payment_method['settings'])) {
    drupal_set_message(t('You are not configure Adyen gateway!'), 'error');
  }
  else {
    /* @var \EntityDrupalWrapper $order_wrapper */
    $order_wrapper = entity_metadata_wrapper('commerce_order', $order);
    $redirect_available = TRUE;

    // Payment fields.
    // @link https://docs.adyen.com/developers/hpp-manual#hpppaymentfields
    $payment = new \Commerce\Adyen\Request\Payment();
    $payment->setPaymentAmount($order_wrapper->commerce_order_total->amount->value());
    $payment->setCurrencyCode($order_wrapper->commerce_order_total->currency_code->value());
    $payment->setMerchantAccount($payment_method['settings']['merchant_account']);
    $payment->setMerchantReference($order->order_number);
    $payment->setSkinCode($payment_method['settings']['skin_code']);
    $payment->setSessionValidity(strtotime('+1 hour'));
    $payment->setShopperEmail($order->mail);
    $payment->setShopperLocale($payment_method['settings']['shopper_locale']);
    $payment->setMerchantReturnData($order->data['payment_redirect_key']);
    $payment->setResUrl(commerce_adyen_payment_response_url());

    // Allow modules to alter parameters of the request.
    // @see hook_commerce_adyen_payment_request_alter()
    drupal_alter('commerce_adyen_payment_request', $payment, $order);

    try {
      $payment->signRequest($payment_method['settings']['hmac']);
    }
    catch (\Exception $e) {
      watchdog_exception(COMMERCE_ADYEN_PAYMENT_METHOD, $e);
      drupal_set_message($e->getMessage(), 'error');
      $redirect_available = FALSE;
    }

    foreach ($payment as $name => $value) {
      $form[$name] = [
        '#type' => 'hidden',
        '#value' => $value,
      ];
    }

    // @link https://docs.adyen.com/developers/hpp-manual#hppendpoints
    $form['#action'] = sprintf('https://%s.adyen.com/hpp/pay.shtml', $payment_method['settings']['mode']);
    $form['#payment'] = $payment;
    $form['#pre_render'][] = 'commerce_adyen_pre_render_redirect_form';

    $form['submit'] = [
      '#type' => 'submit',
      '#value' => t('Proceed to Adyen'),
      '#disabled' => !$redirect_available,
    ];
  }

  return $form;
}

/**
 * Pre-render callback.
 *
 * {@inheritdoc}
 *
 * @see drupal_render()
 *
 * @internal
 */
function commerce_adyen_pre_render_redirect_form(array $form) {
  // Remove Drupal fields from a form (such as "form_token", "form_id" etc).
  // This needs to be done since Adyen will generate a signature from submitted
  // data and will expect only those fields which described in API. Any other
  // data will lead to wrong signature of payment request.
  foreach (array_diff(element_children($form), array_keys(iterator_to_array($form['#payment']))) as $name) {
    unset($form[$name]);
  }

  return $form;
}
