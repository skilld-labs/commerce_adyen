<?php

namespace Drupal\commerce_adyen\Plugin\Commerce\PaymentGateway;

use Drupal\commerce_payment\CreditCard;
use Drupal\commerce_payment\Entity\PaymentInterface;
use Drupal\commerce_payment\Entity\PaymentMethodInterface;
use Drupal\commerce_payment\Exception\HardDeclineException;
use Drupal\commerce_payment\PaymentMethodTypeManager;
use Drupal\commerce_payment\PaymentTypeManager;
use Drupal\commerce_payment\Plugin\Commerce\PaymentGateway\OnsitePaymentGatewayBase;
use Drupal\commerce_price\Price;
use Drupal\Component\Datetime\TimeInterface;
use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\Form\FormStateInterface;

/**
 * Provides the Adyen payment gateway.
 *
 * @CommercePaymentGateway(
 *   id = "adyen_onsite",
 *   label = "Adyen (On-site)",
 *   display_label = "Adyen",
 *   forms = {
 *     "add-payment-method" = "Drupal\commerce_adyen\PluginForm\Adyen\AdyenPaymentMethodAddForm",
 *   },
 *   payment_method_types = {"credit_card"},
 *   credit_card_types = {
 *     "amex", "dinersclub", "discover", "jcb", "maestro", "mastercard", "visa",
 *   },
 * )
 */
class Adyen extends OnsitePaymentGatewayBase implements AdyenInterface {

  /**
   * {@inheritdoc}
   */
  public function __construct(array $configuration, $plugin_id, $plugin_definition, EntityTypeManagerInterface $entity_type_manager, PaymentTypeManager $payment_type_manager, PaymentMethodTypeManager $payment_method_type_manager, TimeInterface $time) {
    parent::__construct($configuration, $plugin_id, $plugin_definition, $entity_type_manager, $payment_type_manager, $payment_method_type_manager, $time);

    // You can create an instance of the SDK here and assign it to $this->api.
    // Or inject Guzzle when there's no suitable SDK.
  }

  /**
   * {@inheritdoc}
   */
  public function defaultConfiguration() {
    return [
      'merchant_account' => '',
      'client_user' => '',
      'client_password' => '',
      'skin_code' => '',
      'hmac' => '',
      'shopper_locale' => '',
      'recurring' => '',
      'state' => '',
      'default_payment_type' => '',
      'use_checkout_form' => '',
      'settings' => '',
    ] + parent::defaultConfiguration();
  }

  /**
   * {@inheritdoc}
   */
  public function buildConfigurationForm(array $form, FormStateInterface $form_state) {
    $form = parent::buildConfigurationForm($form, $form_state);

    $form['merchant_account'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Merchant Account'),
      '#default_value' => $this->configuration['merchant_account'],
      '#required' => TRUE,
    ];

    $form['client_user'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Client User'),
      '#default_value' => $this->configuration['client_user'],
      '#required' => TRUE,
    ];

    $form['client_password'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Client Password'),
      '#default_value' => $this->configuration['client_password'],
      '#required' => TRUE,
    ];

    $form['skin_code'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Skin Code'),
      '#default_value' => $this->configuration['skin_code'],
      '#required' => TRUE,
    ];

    $form['hmac'] = [
      '#type' => 'textfield',
      '#title' => $this->t('HMAC Key'),
      '#default_value' => $this->configuration['hmac'],
      '#required' => TRUE,
    ];

    $form['shopper_locale'] = [
      '#type' => 'select',
      '#title' => $this->t('Shopper locale'),
      '#default_value' => $this->configuration['shopper_locale'],
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
      '#required' => TRUE,
    ];
    
    $form['recurring'] = [
      '#type' => 'select',
      '#title' => t('Recurring contract'),
      '#empty_option' => t('Do not used'),
      '#default_value' => $this->configuration['recurring'],
      '#options' => [
        'ONECLICK' => t('One click'),
        'RECURRING' => t('Recurring'),
        'ONECLICK,RECURRING' => t('One click, recurring'),
      ],
    ];

    $form['state'] = [
      '#type' => 'select',
      '#title' => t('Fields state'),
      '#default_value' => $this->configuration['state'],
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

    $types = [];
    //TODO Upgrade this code.
    foreach ($payment_types as $payment_type => $data) {
      $config_form = commerce_adyen_invoke_controller('payment', $payment_type, $settings, $payment_types)->configForm();

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
      '#default_value' => $this->configuration['default_payment_type'],
      '#disabled' => empty($types),
      '#description' => t('Selected payment type will be set as default extender for the payment request. This value can be changed during checkout process.'),
      '#empty_option' => t('- None -'),
    ];

    $form['use_checkout_form'] = [
      '#type' => 'checkbox',
      '#default_value' => $this->configuration['use_checkout_form'],
      '#title' => t('Use checkout forms'),
      '#disabled' => empty($payment_types),
      '#description' => t('Allow to use checkout forms for filing additional data for the payment type.'),
    ];

    return $form;
  }


  /**
   * {@inheritdoc}
   */
  public function submitConfigurationForm(array &$form, FormStateInterface $form_state) {
    parent::submitConfigurationForm($form, $form_state);

    if (!$form_state->getErrors()) {
      $values = $form_state->getValue($form['#parents']);
      $this->configuration['merchant_account'] = $values['merchant_account'];
      $this->configuration['client_user'] = $values['client_user'];
      $this->configuration['client_password'] = $values['client_password'];
      $this->configuration['skin_code'] = $values['skin_code'];
      $this->configuration['hmac'] = $values['hmac'];
      $this->configuration['shopper_locale'] = $values['shopper_locale'];
      $this->configuration['recurring'] = $values['recurring'];
      $this->configuration['states'] = $values['states'];
      $this->configuration['default_payment_type'] = $values['default_payment_type'];
      $this->configuration['use_checkout_form'] = $values['use_checkout_form'];
    }
  }

  /**
   * {@inheritdoc}
   */
  public function createPayment(PaymentInterface $payment, $capture = TRUE) {
    $this->assertPaymentState($payment, ['new']);
    $payment_method = $payment->getPaymentMethod();
    $this->assertPaymentMethod($payment_method);

    // Add a built in test for testing decline exceptions.
    /** @var \Drupal\address\Plugin\Field\FieldType\AddressItem $billing_address */
    if ($billing_address = $payment_method->getBillingProfile()) {
      $billing_address = $payment_method->getBillingProfile()->get('address')->first();
      if ($billing_address->getPostalCode() == '53140') {
        throw new HardDeclineException('The payment was declined');
      }
    }

    // Perform the create payment request here, throw an exception if it fails.
    // See \Drupal\commerce_payment\Exception for the available exceptions.
    // Remember to take into account $capture when performing the request.
    $amount = $payment->getAmount();
    $payment_method_token = $payment_method->getRemoteId();
    // The remote ID returned by the request.
    $remote_id = '123456';
    $next_state = $capture ? 'completed' : 'authorization';

    $payment->setState($next_state);
    $payment->setRemoteId($remote_id);
    $payment->save();
  }

  /**
   * {@inheritdoc}
   */
  public function capturePayment(PaymentInterface $payment, Price $amount = NULL) {
    $this->assertPaymentState($payment, ['authorization']);
    // If not specified, capture the entire amount.
    $amount = $amount ?: $payment->getAmount();

    // Perform the capture request here, throw an exception if it fails.
    // See \Drupal\commerce_payment\Exception for the available exceptions.
    $remote_id = $payment->getRemoteId();
    $number = $amount->getNumber();

    $payment->setState('completed');
    $payment->setAmount($amount);
    $payment->save();
  }

  /**
   * {@inheritdoc}
   */
  public function voidPayment(PaymentInterface $payment) {
    $this->assertPaymentState($payment, ['authorization']);
    // Perform the void request here, throw an exception if it fails.
    // See \Drupal\commerce_payment\Exception for the available exceptions.
    $remote_id = $payment->getRemoteId();

    $payment->setState('authorization_voided');
    $payment->save();
  }

  /**
   * {@inheritdoc}
   */
  public function refundPayment(PaymentInterface $payment, Price $amount = NULL) {
    $this->assertPaymentState($payment, ['completed', 'partially_refunded']);
    // If not specified, refund the entire amount.
    $amount = $amount ?: $payment->getAmount();
    $this->assertRefundAmount($payment, $amount);

    // Perform the refund request here, throw an exception if it fails.
    // See \Drupal\commerce_payment\Exception for the available exceptions.
    $remote_id = $payment->getRemoteId();
    $number = $amount->getNumber();

    $old_refunded_amount = $payment->getRefundedAmount();
    $new_refunded_amount = $old_refunded_amount->add($amount);
    if ($new_refunded_amount->lessThan($payment->getAmount())) {
      $payment->setState('partially_refunded');
    }
    else {
      $payment->setState('refunded');
    }

    $payment->setRefundedAmount($new_refunded_amount);
    $payment->save();
  }

  /**
   * {@inheritdoc}
   */
  public function createPaymentMethod(PaymentMethodInterface $payment_method, array $payment_details) {
    $required_keys = [
      // The expected keys are payment gateway specific and usually match
      // the PaymentMethodAddForm form elements. They are expected to be valid.
      'type', 'number', 'expiration',
    ];
    foreach ($required_keys as $required_key) {
      if (empty($payment_details[$required_key])) {
        throw new \InvalidArgumentException(sprintf('$payment_details must contain the %s key.', $required_key));
      }
    }

    // If the remote API needs a remote customer to be created.
    $owner = $payment_method->getOwner();
    if ($owner && $owner->isAuthenticated()) {
      $customer_id = $this->getRemoteCustomerId($owner);
      // If $customer_id is empty, create the customer remotely and then do
      // $this->setRemoteCustomerId($owner, $customer_id);
      // $owner->save();
    }

    // Perform the create request here, throw an exception if it fails.
    // See \Drupal\commerce_payment\Exception for the available exceptions.
    // You might need to do different API requests based on whether the
    // payment method is reusable: $payment_method->isReusable().
    // Non-reusable payment methods usually have an expiration timestamp.
    $payment_method->card_type = $payment_details['type'];
    // Only the last 4 numbers are safe to store.
    $payment_method->card_number = substr($payment_details['number'], -4);
    $payment_method->card_exp_month = $payment_details['expiration']['month'];
    $payment_method->card_exp_year = $payment_details['expiration']['year'];
    $expires = CreditCard::calculateExpirationTimestamp($payment_details['expiration']['month'], $payment_details['expiration']['year']);
    // The remote ID returned by the request.
    $remote_id = '789';

    $payment_method->setRemoteId($remote_id);
    $payment_method->setExpiresTime($expires);
    $payment_method->save();
  }

  /**
   * {@inheritdoc}
   */
  public function deletePaymentMethod(PaymentMethodInterface $payment_method) {
    // Delete the remote record here, throw an exception if it fails.
    // See \Drupal\commerce_payment\Exception for the available exceptions.
    // Delete the local entity.
    $payment_method->delete();
  }

}
