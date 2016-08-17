<?php
/**
 * @file
 * Adyen payment.
 */

use Commerce\Adyen\Payment;

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
  $payment_types = commerce_adyen_payment_types();
  $payment_type = commerce_adyen_invoke_controller($settings, $payment_types);
  $types = [];
  $form = [];

  foreach ($payment_types as $name => $data) {
    $types[$name] = $data['label'];
  }

  $link = function ($title, $link) use ($settings) {
    // @codingStandardsIgnoreStart
    return ' ' . t('You will find the right value in the <a href="@href" target="_blank">' . $title . '</a>.', [
      // @codingStandardsIgnoreEnd
      '@href' => sprintf("https://ca-%s.adyen.com/ca/ca/$link", isset($settings['mode']) ? $settings['mode'] : \Adyen\Environment::TEST),
    ]);
  };

  // The wrapper is needed to reload a part of form using AJAX.
  $form['#prefix'] = '<div id="' . __FUNCTION__ . '">';
  $form['#suffix'] = '</div>';

  $form['url'] = [
    '#markup' => t('Please <a href="@notification_setup" target="_blank">configure your Adyen account</a> to send notifications to this URL: %notification_url.', [
      '@notification_setup' => 'https://docs.adyen.com/developers/api-manual#setupnotifications',
      '%notification_url' => url(COMMERCE_ADYEN_NOTIFICATION_URL, ['absolute' => TRUE]),
    ]),
  ];

  $form['mode'] = [
    '#type' => 'radios',
    '#title' => t('Mode'),
    '#required' => TRUE,
    '#default_value' => \Adyen\Environment::TEST,
    '#options' => [
      \Adyen\Environment::TEST => t('Test'),
      \Adyen\Environment::LIVE => t('Live'),
    ],
  ];

  $form['merchant_account'] = [
    '#type' => 'textfield',
    '#title' => t('Merchant Account'),
    '#required' => TRUE,
    '#description' => t('Do not confuse this with your Adyen account name.') . $link('account list', 'accounts/show.shtml?accountTypeCode=MerchantAccount'),
  ];

  $form['client_user'] = [
    '#type' => 'textfield',
    '#title' => t('Client user'),
    '#required' => TRUE,
    '#description' => t('Username for a web service.') . $link('user list', 'config/users.shtml?userType=SU&status=Active'),
  ];

  $form['client_password'] = [
    '#type' => 'textfield',
    '#title' => t('Client password'),
    '#required' => TRUE,
    '#description' => t('Password for a web service user.'),
  ];

  $form['skin_code'] = [
    '#type' => 'textfield',
    '#title' => t('Skin Code'),
    '#required' => TRUE,
    '#description' => t('A valid HPP skin code.') . $link('skin list', 'skin/skins.shtml'),
  ];

  $form['hmac'] = [
    '#type' => 'textfield',
    '#title' => t('HMAC key'),
    '#required' => TRUE,
    '#description' => t('Make sure this exactly matches the HMAC in Adyen skin configuration.'),
  ];

  $form['shopper_locale'] = [
    '#type' => 'select',
    '#title' => t('Shopper locale'),
    '#required' => TRUE,
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

  $form['recurring'] = [
    '#type' => 'select',
    '#title' => t('Recurring payments'),
    '#empty_option' => t('Do not used'),
    '#options' => [
      \Adyen\Contract::ONECLICK => t('One click'),
      \Adyen\Contract::RECURRING => t('Recurring'),
      \Adyen\Contract::ONECLICK_RECURRING => t('One click, recurring'),
    ],
  ];

  $form['type'] = [
    '#type' => 'select',
    '#title' => t('Payment type'),
    '#options' => $types,
    '#empty_option' => t('Default'),
    '#ajax' => [
      'callback' => 'commerce_adyen_settings_form_ajax',
      'wrapper' => __FUNCTION__,
    ],
  ];

  if (NULL !== $payment_type) {
    $payment_type_form = $payment_type->configForm();

    if (!empty($payment_type_form)) {
      $form[$settings['type']] = $payment_type_form + [
        '#tree' => TRUE,
        '#type' => 'fieldset',
        '#title' => $payment_types[$settings['type']]['label'],
      ];
    }
  }

  commerce_adyen_settings_form_process($form, $settings);

  return $form;
}

/**
 * Implements PAYMENT_METHOD_BASE_submit_form().
 *
 * {@inheritdoc}
 *
 * @see commerce_adyen_commerce_payment_method_info()
 * @see commerce_payment_pane_checkout_form()
 *
 * @internal
 */
function commerce_adyen_submit_form(array $payment_method, array $pane_values, array $checkout_pane, \stdClass $order) {
  $payment_type = commerce_adyen_invoke_controller($payment_method['settings'], NULL, 'checkout');
  $form = [];

  if (NULL !== $payment_type) {
    /* @var \EntityDrupalWrapper $wrapper */
    $wrapper = entity_metadata_wrapper('commerce_order', $order);
    $payment_type->setOrder($wrapper);

    // Create fields and attach them to "billing" customer profile.
    // @todo Maybe we need to move this logic into "hook_modules_enabled"?
    foreach ($payment_type->checkoutFields() as $field_name => $instance) {
      $instance['bundle'] = 'billing';
      $instance['field_name'] = COMMERCE_ADYEN_PAYMENT_METHOD . '_' . $field_name;
      $instance['entity_type'] = 'commerce_customer_profile';

      if (empty(field_info_field($instance['field_name']))) {
        field_create_field($instance);
      }

      if (empty(field_info_instance($instance['entity_type'], $instance['field_name'], $instance['bundle']))) {
        field_create_instance($instance);
      }
    }

    $checkout_form = $payment_type->checkoutForm($wrapper->commerce_customer_billing);

    if (!empty($checkout_form)) {
      $form[$payment_method['settings']['type']] = $checkout_form + ['#tree' => TRUE];
    }
  }

  return $form;
}

/**
 * Implements PAYMENT_METHOD_BASE_submit_form_submit().
 *
 * {@inheritdoc}
 *
 * @see commerce_adyen_commerce_payment_method_info()
 * @see commerce_payment_pane_checkout_form_submit()
 *
 * @internal
 */
function commerce_adyen_submit_form_submit(array $payment_method, array $form, array $values, \stdClass $order, array $balance) {
  $payment_type = commerce_adyen_invoke_controller($payment_method['settings'], NULL, 'checkout');

  // We have no checkout controller for this payment type.
  if (NULL === $payment_type) {
    return TRUE;
  }

  /* @var \EntityDrupalWrapper $wrapper */
  $wrapper = entity_metadata_wrapper('commerce_order', $order);
  // Get values of payment type form.
  $values = $values[$payment_method['settings']['type']];
  // Populate order to checkout controller.
  $payment_type->setOrder($wrapper);
  // Validate payment type checkout information.
  $status = $payment_type->checkoutFormValidate($form[$payment_method['settings']['type']], $values, $wrapper->commerce_customer_billing);

  // Save values into billing profile if validation successfully completed.
  if ($status) {
    try {
      foreach ($payment_type->checkoutFields() as $field_name => $instance) {
        $wrapper->commerce_customer_billing->{COMMERCE_ADYEN_PAYMENT_METHOD . '_' . $field_name} = isset($values[$field_name]) ? $values[$field_name] : NULL;
      }

      $wrapper->commerce_customer_billing->save();
    }
    catch (\Exception $e) {
      watchdog_exception(COMMERCE_ADYEN_PAYMENT_METHOD, $e);
      // Payment request cannot be submitted without these values.
      return FALSE;
    }
  }

  return $status;
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
    $payment = new Payment\Authorisation\Request($order, $payment_method);
    $payment_type = commerce_adyen_invoke_controller($payment_method['settings']);

    if (!empty($payment_method['settings']['recurring'])) {
      // @todo Shouldn't we use "$payment->getRecurringContractDetails()"
      // here, choose the reference and set it as "$payment_reference"
      // argument for "$payment->setRecurringContract()"? Which logic should
      // be implemented?
      $payment->setRecurringContract($payment_method['settings']['recurring']);
    }

    if (NULL !== $payment_type) {
      $payment->extend($payment_type);
    }

    // @see hook_commerce_adyen_payment_authorisation_request_alter()
    drupal_alter('commerce_adyen_payment_authorisation_request', $payment, $order, $payment_method);
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
    $payment = new Payment\Authorisation\Response($order, $payment_method);

    switch ($payment->getAuthenticationResult()) {
      case $payment::AUTHORISED:
        // Allow to authorise the payment on local environments where
        // notifications are not available. Capturing must be done
        // manually from Adyen backend.
        if (variable_get('commerce_adyen_authorise_forcibly', FALSE)) {
          $transaction = new Payment\Transaction($order);
          $transaction->authorise($_GET['pspReference']);
          $transaction->setPayload($_GET);
          $transaction->save();
        }
        break;

      case $payment::ERROR:
      case $payment::REFUSED:
        throw new \RuntimeException(t('Payment authorisation was not successful. Please try again.'));

      case $payment::CANCELLED:
        throw new \Exception(t('Payment has been cancelled.'));
    }

    // @see hook_commerce_adyen_payment_authorisation_response_alter()
    drupal_alter('commerce_adyen_payment_authorisation_response', $payment, $order, $payment_method);
  }
  catch (\Exception $e) {
    watchdog_exception(COMMERCE_ADYEN_PAYMENT_METHOD, $e);
    // For exceptions of type "\Exception" will be used "warning"
    // status of a message. In all other cases - "error".
    drupal_set_message($e->getMessage(), is_subclass_of($e, \Exception::class) ? 'error' : 'warning');
    // If we exit we're failed, then customer cannot proceed to the next page.
    commerce_payment_redirect_pane_previous_page($order);
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

/**
 * AJAX callback.
 *
 * {@inheritdoc}
 *
 * @see commerce_adyen_settings_form()
 * @see ajax_get_form()
 *
 * @internal
 */
function commerce_adyen_settings_form_ajax(array $form, array &$form_state) {
  $form_state['rebuild'] = TRUE;

  return $form['parameter']['payment_method']['settings'];
}

/**
 * Populate values from settings to form items recursively.
 *
 * {@inheritdoc}
 *
 * @see commerce_adyen_settings_form()
 *
 * @internal
 */
function commerce_adyen_settings_form_process(array &$form, &$settings) {
  foreach (element_children($form) as $child) {
    if (isset($settings[$child])) {
      $form[$child]['#default_value'] = $settings[$child];
    }
    elseif (isset($form[$child]['#default_value'])) {
      $settings[$child] = $form[$child]['#default_value'];
    }

    if (is_array($form[$child])) {
      // @codingStandardsIgnoreStart
      call_user_func_array(__FUNCTION__, [&$form[$child], &$settings[$child]]);
      // @codingStandardsIgnoreEnd
    }
  }
}

/**
 * Instantiate payment type class.
 *
 * @param array $settings
 *   Payment method settings.
 * @param array[] $payment_types
 *   List of payment types. Must be result of "commerce_adyen_payment_types()".
 * @param string $controller
 *   One of controller types: "payment" or "checkout".
 *
 * @return Payment\Controller\Payment|Payment\Controller\Checkout|NULL
 *   NULL is payment type class cannot be instantiated.
 *
 * @internal
 */
function commerce_adyen_invoke_controller(array $settings, array $payment_types = NULL, $controller = 'payment') {
  if (empty($settings['type'])) {
    return NULL;
  }

  if (NULL === $payment_types) {
    $payment_types = commerce_adyen_payment_types();
  }

  return new $payment_types[$settings['type']]['controllers'][$controller](isset($settings[$settings['type']]) ? $settings[$settings['type']] : []);
}
