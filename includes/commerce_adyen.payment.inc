<?php

/**
 * @file
 * Adyen payment.
 */

use Adyen\Contract;
use Adyen\Environment;
use Commerce\Adyen\Payment\Controller\Checkout;
use Commerce\Adyen\Payment\Authorisation\Request;
use Commerce\Adyen\Payment\Authorisation\Response;

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
  $notification_url = url(COMMERCE_ADYEN_NOTIFICATION_URL, ['absolute' => TRUE]);
  $payment_types = commerce_adyen_payment_types();
  $backend_link = sprintf('https://ca-%s.adyen.com/ca/ca', isset($settings['mode']) ? $settings['mode'] : Environment::TEST);
  $types = [];
  $form = [];

  $link = function ($title, $link) use ($backend_link) {
    // @codingStandardsIgnoreStart
    return ' ' . t('You will find the right value in the <a href="@href" target="_blank">' . $title . '</a>.', [
      // @codingStandardsIgnoreEnd
      '@href' => "$backend_link/$link",
    ]);
  };

  $form['url'] = [
    '#markup' => t('Please <a href="@notification_setup" target="_blank">configure your Adyen account</a> to send notifications to this URL: %notification_url (@basic_auth_status).', [
      '@notification_setup' => 'https://docs.adyen.com/developers/api-manual#setupnotifications',
      '%notification_url' => $notification_url,
      '@basic_auth_status' => function_exists('basic_auth_config_exists') && basic_auth_config_exists(COMMERCE_ADYEN_NOTIFICATION_URL, TRUE) ? t('basic HTTP authorisation enabled') : t('basic HTTP authorisation disabled'),
    ]),
  ];

  $form['basic_auth'] = [
    '#type' => 'item',
    '#description' => t('You can restrict an access to the %notification_url URL using <a href="@basic_auth_url" target="_blank">basic HTTP authorisation</a>. We recommend to do this since it help to get rid from dummy messages in system log, when someone decide to visit that URL specially or by accident.', [
      '@basic_auth_url' => function_exists('basic_auth_config_exists') ? url('admin/config/system/basic-auth', ['absolute' => TRUE]) : url('https://www.drupal.org/project/basic_auth', ['external' => TRUE]),
      '%notification_url' => $notification_url,
    ]),
  ];

  $form['mode'] = [
    '#type' => 'radios',
    '#title' => t('Mode'),
    '#required' => TRUE,
    '#default_value' => Environment::TEST,
    '#options' => [
      Environment::TEST => t('Test'),
      Environment::LIVE => t('Live'),
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
    '#title' => t('Recurring contract'),
    '#empty_option' => t('Do not used'),
    '#options' => [
      Contract::ONECLICK => t('One click'),
      Contract::RECURRING => t('Recurring'),
      Contract::ONECLICK_RECURRING => t('One click, recurring'),
    ],
  ];

  $form['state'] = [
    '#type' => 'select',
    '#title' => t('Fields state'),
    '#default_value' => 0,
    '#description' => t('State of fields on Adyen HPP.'),
    '#options' => [
      t('Fields are visible and modifiable'),
      t('Fields are visible but unmodifiable'),
      t('Fields are not visible and unmodifiable'),
    ],
  ];

  $form['payment_types'] = [
    '#type' => 'vertical_tabs',
  ];

  foreach ($payment_types as $payment_type => $data) {
    $config_form = commerce_adyen_invoke_controller('payment', $payment_type, $settings, $payment_types)
      ->configForm();

    if (!empty($config_form)) {
      $config_form['#type'] = 'fieldset';
      $config_form['#title'] = $data['label'];

      $form['payment_types'][$payment_type] = $config_form;
    }

    // Form a list of payment types and their labels.
    $types[$payment_type] = $data['label'];
  }

  $form['default_payment_type'] = [
    '#type' => 'select',
    '#title' => t('Default payment type'),
    '#options' => $types,
    '#disabled' => empty($types),
    '#description' => t('Selected payment type will be set as default extender for the payment request. This value can be changed during checkout process.'),
    '#empty_option' => t('- None -'),
  ];

  $form['use_checkout_form'] = [
    '#type' => 'checkbox',
    '#title' => t('Use checkout forms'),
    '#disabled' => empty($payment_types),
    '#description' => t('Allow to use checkout forms for filing additional data for the payment type.'),
  ];

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
function commerce_adyen_submit_form(array $payment_method, array &$pane_values, array $checkout_pane, \stdClass $order) {
  $payment_types = commerce_adyen_payment_types();
  $form = [];

  if (!empty($payment_types)) {
    // This is a dummy type.
    $payment_type = '';

    // Add a "default" option to be able back to standard payment.
    $options = [
      $payment_type => t('Regular'),
    ];

    // Describe a standard payment type a little.
    $descriptions = [
      $payment_type => [
        '#description' => t('Extra data, required by other payment methods, will not be sent'),
      ],
    ];

    foreach ($payment_types as $name => $data) {
      /* @var \Commerce\Adyen\Payment\Controller\Payment $payment_controller */
      $payment_controller = $data['controllers']['payment'];

      $options[$name] = $data['label'];
      $descriptions[$name]['#description'] = implode(', ', $payment_controller::subTypes());
    }

    // Form have already been submitted. Use selected value.
    if (!empty($pane_values['payment_details']['payment_type'])) {
      $payment_type = $pane_values['payment_details']['payment_type'];
    }
    // In other case we can try to get the chosen payment type from an order.
    elseif (!empty($order->data['commerce_adyen_payment_type'])) {
      $payment_type = $order->data['commerce_adyen_payment_type'];
    }

    // Existing orders can have payment type selected. If this type will
    // be disabled, while an order be in active state, then fatal error
    // will be appeared on redirection to Adyen HPP. But we are not stupid,
    // we are double-check subtype for existence.
    if (empty($payment_types[$payment_type])) {
      $payment_type = '';
    }

    $form['payment_type'] = $descriptions + [
      '#type' => 'radios',
      '#title' => t('Type'),
      '#options' => $options,
      '#default_value' => $payment_type ?: $payment_method['settings']['default_payment_type'],
      '#ajax' => [
        // @see commerce_payment_pane_checkout_form()
        'callback' => 'commerce_payment_pane_checkout_form_details_refresh',
        'wrapper' => 'payment-details',
      ],
    ];

    $checkout_controller = commerce_adyen_invoke_controller('checkout', $payment_type, $payment_method['settings'], $payment_types);

    if (NULL !== $checkout_controller) {
      $checkout_controller->setOrder($order);
      $checkout_controller->setPaymentMethod($payment_method);

      if (!empty($payment_method['settings']['use_checkout_form'])) {
        $checkout_form = $checkout_controller->checkoutForm();

        if (!empty($checkout_form)) {
          $form[$payment_type] = $checkout_form;
          $form[$payment_type]['#tree'] = TRUE;
        }
      }

      $form['#checkout_controller'] = $checkout_controller;
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
  $validation_passed = TRUE;

  // An instance of checkout controller will be available
  // only if payment subtype has been selected.
  if (!empty($values['payment_type'])) {
    $payment_type = $values['payment_type'];
    // Simulate empty list of values from checkout form.
    $values += [$payment_type => []];

    // Save payment type into an order to be able to use it in a redirect
    // form. It will be needed to instantiate payment controller and
    // extending regular payment by a subtype.
    $order->data['commerce_adyen_payment_type'] = $payment_type;
    // Store values for the payment type from checkout form.
    $order->data[$payment_type] = $values[$payment_type];

    if (!empty($form['#checkout_controller']) && !empty($form[$payment_type]) && !empty($values[$payment_type])) {
      $checkout_controller = $form['#checkout_controller'];

      if ($checkout_controller instanceof Checkout) {
        $checkout_controller->checkoutFormValidate($form[$payment_type], $values[$payment_type]);
      }
    }
  }

  return $validation_passed;
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
    $payment = new Request($order, $payment_method);

    if (!empty($payment_method['settings']['recurring'])) {
      $payment->setRecurringContract($payment_method['settings']['recurring']);
    }

    if (!empty($order->data['commerce_adyen_payment_type'])) {
      $payment_controller = commerce_adyen_invoke_controller('payment', $order->data['commerce_adyen_payment_type'], $payment_method['settings']);
      $payment_controller->setCheckoutValues($order->data[$order->data['commerce_adyen_payment_type']]);
      $payment->extend($payment_controller);
    }

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
    $payment = new Response($order, $payment_method);
    $status = $payment->getAuthenticationResult();

    switch ($status) {
      case $payment::AUTHORISED:
      case $payment::PENDING:
        // Allow to authorise/pending the payment on local environments where
        // notifications are not available. Capturing must be done
        // manually from Adyen backend.
        if (variable_get('commerce_adyen_authorise_forcibly', FALSE)) {
          $transaction = $payment->getTransaction();
          $transaction->{$payment::PENDING === $status ? 'pending' : 'authorise'}($transaction->getRemoteId());
          $transaction->save();

          commerce_adyen_capture_request($order);
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
  // This needs to be done since Adyen will generate signature using submitted
  // data and will expect only those fields which described in API. Any other
  // data will lead to wrong signature of payment request.
  foreach (array_diff(element_children($form), array_keys(iterator_to_array($form['#payment']))) as $name) {
    unset($form[$name]);
  }

  return $form;
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
