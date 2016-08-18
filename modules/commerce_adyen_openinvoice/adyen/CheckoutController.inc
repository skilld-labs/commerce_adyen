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
        // By OpenInvoice specification you must be older than 18 year.
        'year_range' => '-100:-18',
        'granularity' => drupal_map_assoc(['year', 'month', 'day']),
      ],
      'widget' => [
        'input_format' => 'd/m/Y',
      ],
    ];

    $fields['social_number'] = [
      'type' => 'text',
      'label' => t('Social security number'),
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
      '#options' => $fields['gender']['settings']['allowed_values'],
    ];

    $form['phone_number'] = [
      '#type' => 'textfield',
    ];

    $form['birth_date'] = [
      '#type' => 'date_select',
      '#date_format' => $fields['birth_date']['widget']['input_format'],
      '#date_year_range' => $fields['birth_date']['settings']['year_range'],
    ];

    $form['social_number'] = [
      '#type' => 'textfield',
    ];

    foreach ($fields as $field => $specification) {
      $field_name = COMMERCE_ADYEN_PAYMENT_METHOD . '_' . $field;

      $form[$field] += [
        '#title' => $fields[$field]['label'],
        '#required' => !empty($fields[$field]['required']),
        '#default_value' => isset($billing->{$field_name}) ? $billing->{$field_name}->value() : NULL,
      ];
    }

    // The "date_default_date()" function will not handle UNIX timestamps
    // correctly because third argument will be passed to constructor of
    // the "\DateObject" object in any case.
    // @see date_default_date()
    // @see \DateObject::__construct()
    // @todo This line should be removed and patch to "date" module created.
    $form['birth_date']['#default_value'] = date('Y-m-d', $form['birth_date']['#default_value']);

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function checkoutFormValidate(array $form, array &$values, \EntityDrupalWrapper $billing) {
    $values['birth_date'] = strtotime($values['birth_date']);

    return TRUE;
  }

}
