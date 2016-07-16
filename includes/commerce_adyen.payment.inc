<?php
/**
 * @file
 * Adyen payment.
 */

use Commerce\Adyen;

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
      'test' => t('Test'),
      'live' => t('Live'),
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

  $form['algorithm'] = [
    '#type' => 'select',
    '#title' => t('Encoding algorithm'),
    '#default_value' => 'sha256',
    '#options' => [
      'sha1' => 'SHA1',
      'sha256' => 'SHA256',
    ],
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

    // Payment fields.
    // @link https://docs.adyen.com/developers/hpp-manual#hpppaymentfields
    $payment = new Adyen\Request\Payment();
    $payment->setPaymentAmount($order_wrapper->commerce_order_total->amount->value());
    $payment->setCurrencyCode($order_wrapper->commerce_order_total->currency_code->value());
    $payment->setMerchantAccount($payment_method['settings']['merchant_account']);
    $payment->setMerchantReference($order->order_numnber);
    $payment->setSkinCode($payment_method['settings']['skin_code']);
    $payment->setSessionValidity(strtotime('+1 hour'));
    $payment->setShopperEmail($order->mail);
    $payment->setShopperLocale($payment_method['settings']['shopper_locale']);
    $payment->setMerchantReturnData($order->data['payment_redirect_key']);
    $payment->setResURL(commerce_adyen_payment_response_url());

    // Allow modules to alter parameters of the request.
    drupal_alter('commerce_adyen_payment_request', $payment, $order);

    // @todo Why we sign it here and unset a value on "#pre_render"?
    $payment->signRequest($payment_method['settings']['algorithm'], $payment_method['settings']['hamc']);

    foreach ($payment as $name => $value) {
      $form[$name] = [
        '#type' => 'hidden',
        '#value' => $value,
      ];
    }

    $form['#action'] = commerce_adyen_url($payment_method['settings']['mode'], 'create');
    $form['#payment'] = $payment;
    $form['#pre_render'][] = 'commerce_adyen_pre_render_redirect_form';

    $form['submit'] = [
      '#type' => 'submit',
      '#value' => t('Proceed to Adyen'),
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
  foreach (array_diff(element_children($form), array_keys($form['#payment'])) as $name) {
    unset($form[$name]);
  }

  unset($form['merchantSig']);

  return $form;
}

/**
 * Returns an URL Adyen will respond to.
 *
 * @internal
 *
 * @return string
 *   Response URL.
 */
function commerce_adyen_payment_response_url() {
  return url(COMMERCE_ADYEN_BASE_URL . '/' . Adyen\Response\Payment::type(), ['absolute' => TRUE]);
}
