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
      'type' => 'list_text',
      'label' => t('Gender'),
      'widget' => [
        'type' => 'options_buttons',
      ],
      'settings' => [
        'allowed_values' => [
          'MALE' => t('Male'),
          'FEMALE' => t('Female'),
        ],
      ],
    ];

    $fields['phone_number'] = [
      'type' => 'text',
      'label' => t('Phone number'),
    ];

    $fields['birth_date'] = [
      'type' => 'datestamp',
      'label' => t('Date of birth'),
      'settings' => [
        'year_range' => '-100:-5',
        'granularity' => drupal_map_assoc(['year', 'month', 'day']),
      ],
      'widget' => [
        'input_format' => 'd/m/Y',
      ],
    ];

    foreach ($fields as $name => $info) {
      $fields[$name] += [
        'required' => TRUE,
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
    $fields = $this->checkoutFields();
    $form = [];

    $form['gender'] = [
      '#type' => 'radios',
      '#title' => t('Gender'),
      '#options' => $fields['gender']['settings']['allowed_values'],
    ];

    $form['phone_number'] = [
      '#type' => 'textfield',
      '#title' => t('Phone'),
    ];

    $form['birth_date'] = [
      '#type' => 'date_select',
      '#title' => t('Date of birth'),
      '#date_format' => $fields['birth_date']['widget']['input_format'],
      '#date_year_range' => $fields['birth_date']['settings']['year_range'],
    ];

    foreach ($form as $item => $definition) {
      $field_name = COMMERCE_ADYEN_PAYMENT_METHOD . '_' . $item;

      $form[$item] += [
        '#required' => !empty($fields[$item]['required']),
        '#default_value' => isset($billing->{$field_name}) ? $billing->{$field_name}->value() : NULL,
      ];
    }

    return $form;
  }

}
