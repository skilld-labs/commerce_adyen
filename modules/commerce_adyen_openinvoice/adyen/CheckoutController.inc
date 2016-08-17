<?php
/**
 * @file
 * OpenInvoice checkout controller.
 */

namespace Commerce\Adyen\Payment\OpenInvoice;

use Commerce\Adyen\Payment\Controller\Checkout;

/**
 * Class CheckoutController.
 *
 * @package Commerce\Adyen\Payment\OpenInvoice
 */
class CheckoutController extends Checkout {

  /**
   * {@inheritdoc}
   */
  public function checkoutFields() {
    $fields = [];

    $fields['gender'] = [
      'label' => t('Gender'),
    ];

    $fields['phone_number'] = [
      'label' => t('Phone number'),
    ];

    $fields['birth_date'] = [
      'label' => t('Date of birth'),
    ];

    foreach ($fields as $name => $info) {
      $fields[$name] += [
        'type' => 'text',
        'cardinality' => 1,
        'translatable' => FALSE,
      ];
    }

    return $fields;
  }

  /**
   * {@inheritdoc}
   */
  public function checkoutForm(\EntityDrupalWrapper $billing) {
    $form = [];

    $form['gender'] = [
      '#type' => 'radios',
      '#title' => t('Gender'),
      '#required' => TRUE,
      '#default_value' => $billing->commerce_adyen_gender->value(),
      '#options' => [
        'MALE' => t('Male'),
        'FEMALE' => t('Female'),
      ],
    ];

    $form['phone_number'] = [
      '#type' => 'textfield',
      '#title' => t('Phone'),
      '#required' => TRUE,
      '#default_value' => $billing->commerce_adyen_phone_number->value(),
    ];

    $form['birth_date'] = [
      '#type' => 'date_popup',
      '#title' => t('Date of birth'),
      '#required' => TRUE,
      '#date_format' => $this->config['date_format'],
      '#default_value' => $billing->commerce_adyen_birth_date->value(),
    ];

    return $form;
  }

}
